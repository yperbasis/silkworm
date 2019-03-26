/*
   Copyright 2019 Ethereum Foundation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "state.hpp"

#include <algorithm>
#include <cassert>

#include "db_util.hpp"
#include "keccak.hpp"
#include "mptrie.hpp"
#include "rlp.hpp"

namespace silkworm {

// TODO randomize phase 1 & 2 cursors
State::State(DbBucket& db, uint8_t depth, uint8_t phase1_depth)
    : db_(db),
      tree_(depth),
      phase1_cursor_(phase1_depth),
      phase2_leaf_cursor_(depth) {
  if (depth < 2) {
    throw std::length_error("too shallow");
  }
  if (depth > 15) {
    throw std::length_error("too deep");
  }

  if (phase1_depth > depth) {
    throw std::invalid_argument("phase1_depth > depth");
  }

  for (uint8_t i = 0; i < depth; ++i) {
    tree_[i].resize(1ull << (i * 4));
  }
}

void State::init_from_db(const uint32_t data_valid_for_block) {
  auto prefix = Prefix(depth());

  // bottom nodes
  for (uint64_t i = 0; i < tree_.back().size(); ++i) {
    auto& nodes = tree_.back();
    nodes[i].block = data_valid_for_block;

    if (nodes[i].synced.all()) {
      prefix += 16;
      continue;
    }

    for (Nibble j = 0; j < 16; ++j, ++prefix) {
      if (nodes[i].synced[j]) {
        continue;
      }

      const auto hasher = db_util::hasher(db_, prefix);

      nodes[i].empty[j] = hasher.empty();

      if (!hasher.empty()) {
        nodes[i].hash[j] = hasher.hash();
      }

      nodes[i].synced[j] = true;
    }
  }

  // the rest of the tree
  for (int lvl = static_cast<int>(depth()) - 2; lvl >= 0; --lvl) {
    auto& nodes = tree_[lvl];

    for (uint64_t i = 0; i < nodes.size(); ++i) {
      nodes[i].block = data_valid_for_block;

      if (nodes[i].synced.all()) {
        continue;
      }

      for (Nibble j = 0; j < 16; ++j) {
        if (nodes[i].synced[j]) {
          continue;
        }

        const auto& child = tree_[lvl + 1][i * 16 + j];
        const bool empty = child.empty.all();
        nodes[i].empty[j] = empty;
        if (!empty) {
          nodes[i].hash[j] = mptrie::branch_node_hash(child.empty, child.hash);
        }
        nodes[i].synced[j] = true;
      }
    }
  }
}

void State::put(Hash key, std::string val) {
  root().block = -1;  // prevent sync while block is not sealed yet

  const Prefix prefix(depth(), key);
  for (uint8_t level = 0; level < depth(); ++level) {
    auto& nd = node(level, prefix);
    const Nibble nbl = prefix[level];
    nd.synced[nbl] = false;
  }

  db_.put(byte_view(key), val);
}

void State::update_blocks_down_path(Prefix prefix) {
  for (uint8_t level = 1; level < prefix.size(); ++level) {
    if (!update_block_at(prefix, level)) {
      break;
    }
  }
}

bool State::update_block_at(const Prefix prefix, const uint8_t level) {
  const auto& parent = node(level - 1, prefix);
  auto& child = node(level, prefix);

  if (parent.block == -1 || child.block == -1) {
    return false;
  }

  if (parent.block == child.block) {
    return true;
  }

  const bool child_empty = child.empty.all();
  const Hash child_hash = mptrie::branch_node_hash(child.empty, child.hash);

  const auto nibble = prefix[level - 1];
  if (nibble_obsolete(parent, nibble, child_empty, child_hash)) {
    return false;
  }

  child.block = parent.block;
  return true;
}

uint8_t State::consistent_path_depth(Prefix prefix) const {
  int32_t block = root().block;
  for (uint8_t level = 0; level < prefix.size(); ++level) {
    const auto& nd = node(level, prefix);
    if (nd.block == -1 || nd.block != block) {
      return level;
    }
  }
  return prefix.size();
}

sync::LeavesReply State::get_leaves(
    const sync::GetLeavesRequest& request) const {
  const auto prefix = request.prefix;
  if (prefix.size() == 0) {
    throw std::runtime_error("TODO prefix.size = 0 not implemented yet");
  }
  if (prefix.size() > depth()) {
    throw std::runtime_error("TODO prefix.size > depth not implemented yet");
  }

  sync::LeavesReply reply;

  if (consistent_path_depth(prefix) != prefix.size()) {
    reply.status = sync::LeavesReply::kDontHaveData;
    return reply;
  }

  const Node& nd = node(prefix.size() - 1, prefix);
  const auto nibble = prefix.last();

  if (!nd.synced[nibble]) {
    reply.status = sync::LeavesReply::kDontHaveData;
    return reply;
  }

  auto rb =
      request.block_number ? static_cast<int32_t>(*request.block_number) : -1;
  if (rb > nd.block) {
    reply.status = sync::LeavesReply::kDontHaveData;
    return reply;
  }

  reply.block_number = nd.block;

  const bool full_proof = rb < nd.block;
  const uint8_t proof_start = full_proof ? 0 : request.from_level;

  for (auto i = proof_start; i < prefix.size(); ++i) {
    const auto& y = node(i, prefix);
    reply.proof.push_back(sync::Proof{y.empty, y.hash});
  }

  reply.leaves = std::vector<sync::Leaf>{};
  if (!nd.empty[nibble]) {
    db_util::iterate(
        db_, prefix, [&reply](std::string_view key, std::string_view val) {
          reply.leaves->push_back(sync::Leaf{string_to_hash(key), val});
        });
  }

  return reply;
}

std::variant<std::monostate, sync::GetLeavesRequest, sync::GetNodeRequest>
State::next_sync_request() {
  if (!phase1_sync_done_) {
    const auto r = next_leaves_request(phase1_cursor_, true);
    if (!r)
      phase1_sync_done_ = true;
    else
      return *r;
  }

  if (synced_block() == -1) {
    const auto nr = next_node_request();
    if (!nr.prefixes.empty()) {
      return nr;
    }

    const auto lr = next_leaves_request(phase2_leaf_cursor_, false);
    if (lr) {
      return *lr;
    }
  }

  return {};
}

sync::GetNodeRequest State::next_node_request() {
  sync::GetNodeRequest request;
  request.block_number = root().block;

  auto& prefix = phase2_node_cursor_;
  const auto level = prefix.size();

  if (level >= depth()) {
    return request;
  }

  if (level == 0) {
    request.prefixes.emplace_back('\0');
    prefix = Prefix(1);
    return request;
  }

  while (true) {
    update_block_at(prefix, level);

    const auto& nd = node(level, prefix);
    if (nd.block < root().block) {
      request.prefixes.push_back(prefix);
    }

    ++prefix;

    if (prefix.val() == 0) {
      prefix = Prefix(level + 1);
      return request;
    }

    if (request.prefixes.size() >= kMaxNodesPerRequest) {
      return request;
    }
  }
}

std::optional<sync::GetLeavesRequest> State::next_leaves_request(Prefix& cursor,
                                                                 bool phase1) {
  do {
    const auto prefix = cursor;
    ++cursor;

    const auto& nd = node(prefix.size() - 1, prefix);
    const Nibble x = prefix.last();

    update_blocks_down_path(prefix);
    const auto cpd = consistent_path_depth(prefix);

    if (nd.synced[x] && (cpd == prefix.size() || phase1)) {
      continue;
    } else {
      sync::GetLeavesRequest request{prefix};

      if (root().block != -1) {
        request.block_number = root().block;
      }

      request.from_level = cpd;
      return request;
    }
  } while (cursor.val() != 0);

  return {};
}

bool State::nibble_obsolete(const Node& node, const Nibble nibble,
                            const bool new_empty, const Hash& new_hash) {
  if (!node.empty[nibble] && !new_empty)
    return node.hash[nibble] != new_hash;
  else
    return node.empty[nibble] != new_empty;
}

void State::process_leaves_reply(const Prefix prefix,
                                 const sync::LeavesReply& reply) {
  if (prefix.size() == 0) {
    throw std::runtime_error("TODO prefix.size = 0 not implemented yet");
  }
  if (prefix.size() > depth()) {
    throw std::runtime_error("TODO prefix.size > depth not implemented yet");
  }

  auto& main_node = node(prefix.size() - 1, prefix);

  int32_t rb = reply.block_number;
  if (root().block > rb) {
    return;  // old reply
  } else if (root().block < rb) {
    phase2_node_cursor_ = Prefix(1);
  }

  // TODO verify the reply (proof hashes, etc)
  // if !reply.leaves, check that's legit
  // otherwise check leaves match the prefix
  // and their hash matches proof
  // and they are strictly ordered

  const auto tail = depth() - prefix.size();

  if (tail == 0) {  // prefix.size() == depth()
    const auto& new_empty =
        reply.proof.empty() ? main_node.empty : reply.proof.back().empty;
    const auto& new_hash =
        reply.proof.empty() ? main_node.hash : reply.proof.back().hash;

    const auto nibble = prefix.last();

    for (Nibble j = 0; j < 16; ++j) {
      Prefix nibble_prefix = prefix;
      nibble_prefix.set(prefix.size() - 1, j);

      if (j == nibble) {
        if (reply.leaves) {
          if (main_node.synced[j] && !main_node.empty[j]) {
            db_util::del(db_, nibble_prefix);
          }

          auto it = reply.leaves->begin();
          const auto end = reply.leaves->end();

          db_.put([&it, end]() -> std::optional<DbBucket::KeyVal> {
            if (it == end) {
              return {};
            }
            DbBucket::KeyVal x(byte_view(it->first), it->second);
            ++it;
            return x;
          });
        }
        main_node.empty[j] = new_empty[j];
        main_node.hash[j] = new_hash[j];
        main_node.synced[j] = true;
      } else if (nibble_obsolete(main_node, j, new_empty[j], new_hash[j])) {
        if (main_node.synced[j] && !main_node.empty[j]) {
          db_util::del(db_, nibble_prefix);
        }
        main_node.synced[j] = false;
      }
    }
  } else if (reply.leaves) {  // prefix.size() < depth()
    auto it = reply.leaves->begin();

    db_util::del(db_, prefix);

    // process bottom nodes
    auto btm_prfx = Prefix{depth(), prefix.val()};

    for (uint64_t i = 0; i < (1ull << (4 * tail)); ++i, ++btm_prfx) {
      const auto nibble = btm_prfx.last();
      auto& bottom_node = node(depth() - 1, btm_prfx);

      LeafHasher hasher;

      auto it2 = it;
      for (; it != reply.leaves->end() && btm_prfx.matches(it->first); ++it) {
        hasher.append(byte_view(it->first), it->second);
      }

      db_.put([&it2, it]() -> std::optional<DbBucket::KeyVal> {
        if (it2 == it) {
          return {};
        }
        DbBucket::KeyVal x(byte_view(it2->first), it2->second);
        ++it2;
        return x;
      });

      bottom_node.empty[nibble] = hasher.empty();

      if (!hasher.empty()) {
        bottom_node.hash[nibble] = hasher.hash();
      }

      bottom_node.synced[nibble] = true;
    }

    // propagate up the subtree of main_node
    for (uint8_t level = depth() - 1; level >= prefix.size(); --level) {
      auto sub_prfx = Prefix{level, prefix.val()};
      const auto shift = 4 * (level - prefix.size());

      for (uint64_t i = 0; i < (1ull << shift); ++i, ++sub_prfx) {
        const auto nibble = sub_prfx.last();
        auto& parent = node(level - 1, sub_prfx);
        auto& child = node(level, sub_prfx);

        child.block = rb;

        parent.empty[nibble] = child.empty.all();
        if (!parent.empty[nibble]) {
          parent.hash[nibble] =
              mptrie::branch_node_hash(child.empty, child.hash);
        }
        parent.synced[nibble] = true;
      }
    }
  }

  // update the nodes up the tree path
  const auto start_from =
      static_cast<uint8_t>(prefix.size() - reply.proof.size());
  for (auto level = start_from; level < prefix.size(); ++level) {
    update_node(node(level, prefix), reply.proof[level - start_from], rb);
  }

  propagate_synced_up(prefix, prefix.size() - 1);
}

void State::propagate_synced_up(const Prefix prefix, const uint8_t from_level) {
  for (auto level = from_level; level > 0; --level) {
    const auto nibble = prefix[level - 1];
    auto& parent = node(level - 1, prefix);
    const auto& child = node(level, prefix);
    parent.synced[nibble] = child.synced.all();
  }
}

void State::update_node(Node& nd, const sync::Proof& proof, int32_t new_block) {
  if (new_block <= nd.block) {
    return;
  }

  for (Nibble j = 0; j < 16; ++j) {
    if (nibble_obsolete(nd, j, proof.empty[j], proof.hash[j])) {
      nd.synced[j] = false;
    }
  }

  nd.empty = proof.empty;
  nd.hash = proof.hash;
  nd.block = new_block;
}

std::optional<sync::NodeReply> State::get_nodes(
    const sync::GetNodeRequest& request) const {
  if (request.block_number &&
      static_cast<int32_t>(*request.block_number) > root().block) {
    return {};
  }

  sync::NodeReply reply;
  reply.block_number = root().block;
  reply.nodes.resize(request.prefixes.size());

  for (size_t i = 0; i < reply.nodes.size(); ++i) {
    const auto prefix = request.prefixes[i];

    if (prefix.size() >= depth()) {
      // TODO implement
      continue;
    }

    if (consistent_path_depth(prefix) == prefix.size()) {
      const auto& nd = node(prefix.size(), prefix);
      reply.nodes[i] = sync::Proof{nd.empty, nd.hash};
    }
  }

  return reply;
}

void State::process_node_reply(const sync::GetNodeRequest& request,
                               const sync::NodeReply& reply) {
  if (reply.nodes.size() != request.prefixes.size()) {
    throw std::runtime_error("reply.nodes.size != request.prefixes.size");
  }

  int32_t block_num = reply.block_number;
  if (block_num < root().block) {
    return;  // old reply
  }

  for (size_t i = 0; i < reply.nodes.size(); ++i) {
    const auto nd = reply.nodes[i];
    if (!nd) {
      continue;
    }

    const auto prefix = request.prefixes[i];

    if (prefix.size() >= depth() ||
        consistent_path_depth(prefix) != prefix.size()) {
      continue;
    }

    if (prefix.size() != 0 && block_num > root().block) {
      continue;
    }

    update_node(node(prefix.size(), prefix), *nd, block_num);
    propagate_synced_up(prefix, prefix.size());
  }

  if (block_num > root().block) {
    // root block is stale
    phase2_node_cursor_ = Prefix(0);
  }
}

}  // namespace silkworm
