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

#include "keccak.hpp"
#include "mptrie.hpp"
#include "rlp.hpp"

namespace silkworm {

State::State(DbBucket& db, unsigned depth, unsigned phase1_depth)
    : db_(db), tree_(depth), phase1_cursor_(phase1_depth) {
  if (depth < 2) {
    throw std::length_error("too shallow");
  }
  if (depth > 15) {
    throw std::length_error("too deep");
  }

  if (phase1_depth > depth) {
    throw std::invalid_argument("phase1_depth > depth");
  }

  for (unsigned i = 0; i < depth; ++i) {
    tree_[i].resize(1ull << (i * 4));
  }
}

void State::init_from_db(uint32_t data_valid_for_block) {
  auto prefix = Prefix(depth());

  // bottom nodes
  for (uint64_t i = 0; i < tree_.back().size(); ++i) {
    auto& nodes = tree_.back();

    for (unsigned j = 0; j < 16; ++j) {
      const auto leaves = db_.leaves(prefix);

      const bool empty = leaves.first == leaves.second;
      nodes[i].empty[j] = empty;

      if (!empty) {
        nodes[i].hash[j] = mptrie::hash_of_leaves(leaves.first, leaves.second);
      }

      nodes[i].synced[j] = true;

      ++prefix;
    }

    nodes[i].block = data_valid_for_block;
  }

  // the rest of the tree
  for (int lvl = static_cast<int>(depth()) - 2; lvl >= 0; --lvl) {
    auto& nodes = tree_[lvl];

    for (uint64_t i = 0; i < nodes.size(); ++i) {
      for (unsigned j = 0; j < 16; ++j) {
        const auto& child = tree_[lvl + 1][i * 16 + j];
        const bool empty = child.empty.all();
        nodes[i].empty[j] = empty;
        if (!empty) {
          nodes[i].hash[j] = mptrie::branch_node_hash(child.empty, child.hash);
        }
        nodes[i].synced[j] = true;
      }
      nodes[i].block = data_valid_for_block;
    }
  }
}

unsigned State::consistent_path_depth(Prefix prefix) const {
  int32_t block = root().block;
  for (unsigned level = 0; level < prefix.size(); ++level) {
    const auto& node = this->node(level, prefix);
    if (node.block == -1 || node.block != block) {
      return level;
    }
  }
  return prefix.size();
}

std::variant<sync::Reply, sync::Error> State::get_leaves(
    const sync::Request& request) const {
  const auto prefix = request.prefix;
  if (prefix.size() == 0) {
    throw std::runtime_error("TODO prefix.size = 0 not implemented yet");
  }
  if (prefix.size() > depth()) {
    throw std::runtime_error("TODO prefix.size > depth not implemented yet");
  }

  if (consistent_path_depth(prefix) != prefix.size()) {
    return sync::kDontHaveData;
  }

  const Node& nd = node(prefix.size() - 1, prefix);
  const Nibble last_nibble = prefix[prefix.size() - 1];

  // TODO propogate synced up with consistency checks
  // or some kind of dynamic checks?
  if (!nd.empty[last_nibble] && !nd.synced[last_nibble]) {
    return sync::kDontHaveData;
  }

  auto rb = request.block ? static_cast<int32_t>(*request.block) : -1;
  if (rb > nd.block) {
    return sync::kDontHaveData;
  }

  sync::Reply reply(prefix, nd.block);

  const bool full_proof = rb < nd.block;
  const unsigned proof_start = full_proof ? 0 : request.start_proof_from;

  for (unsigned i = proof_start; i < prefix.size(); ++i) {
    const auto& y = node(i, prefix);
    reply.proof.push_back(sync::Proof{y.empty, y.hash});
  }

  const bool leaves_required = !request.hash_of_leaves ||
                               *request.hash_of_leaves != nd.hash[last_nibble];

  if (leaves_required) {
    reply.leaves = std::vector<sync::Leaf>{};
    if (!nd.empty[last_nibble]) {
      const auto db_leaves = db_.leaves(prefix);
      for (auto it = db_leaves.first; it != db_leaves.second; ++it) {
        reply.leaves->push_back(sync::Leaf{it->first, it->second});
      }
    }
  }

  return reply;
}

std::optional<sync::Request> State::next_sync_request() {
  const auto prefix = phase1_cursor_;
  const auto& nd = node(prefix.size() - 1, prefix);
  const Nibble x = prefix[prefix.size() - 1];

  if (phase1_cursor_.val() == 0 && nd.synced[x]) {
    // TODO check if we still have data gaps
    return {};
  }

  sync::Request request{prefix};

  if (root().block != -1) {
    request.block = root().block;
  }

  request.start_proof_from = consistent_path_depth(prefix);

  if (!nd.empty[x] && nd.synced[x]) {
    request.hash_of_leaves = nd.hash[x];
  }

  // TODO some kind of randomisation instead
  ++phase1_cursor_;

  return request;
}

void State::process_sync_data(const sync::Reply& reply) {
  const auto prefix = reply.prefix;
  if (prefix.size() == 0) {
    throw std::runtime_error("TODO prefix.size = 0 not implemented yet");
  }
  if (prefix.size() > depth()) {
    throw std::runtime_error("TODO prefix.size > depth not implemented yet");
  }

  auto& main_node = node(prefix.size() - 1, prefix);

  int32_t rb = reply.block;
  if (main_node.block > rb) {
    return;  // old reply
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

    const auto last_nibble = prefix[prefix.size() - 1];

    for (unsigned j = 0; j < 16; ++j) {
      Prefix nibble_prefix = prefix;
      nibble_prefix.set(prefix.size() - 1, j);

      if (j == last_nibble) {
        if (reply.leaves) {
          if (main_node.synced[j] && !main_node.empty[j]) {
            db_.erase(nibble_prefix);
          }
          for (const auto& x : *reply.leaves) {
            db_.put(x.first, x.second);
          }
        }
        main_node.synced[j] = true;
      } else if (main_node.empty[j] != new_empty[j] ||
                 (!new_empty[j] && main_node.hash[j] != new_hash[j])) {
        // sibling nibble out of sync

        if (main_node.synced[j] && !main_node.empty[j]) {
          db_.erase(nibble_prefix);
        }
        main_node.synced[j] = false;
      }
    }
  } else if (reply.leaves) {  // prefix.size() < depth()
    auto it = reply.leaves->begin();

    db_.erase(prefix);

    // process bottom nodes
    auto btm_prfx = Prefix{depth(), prefix.val()};

    for (uint64_t i = 0; i < (1ull << (4 * tail)); ++i, ++btm_prfx) {
      const auto last_nibble = btm_prfx[depth() - 1];
      auto& bottom_node = node(depth() - 1, btm_prfx);

      const auto range_begin = it;
      auto range_end = reply.leaves->end();

      while (it != reply.leaves->end()) {
        if (btm_prfx.matches(it->first)) {
          db_.put(it->first, it->second);
          ++it;
        } else {
          range_end = it;
          break;
        }
      }

      const bool empty = range_begin == range_end;
      bottom_node.empty[last_nibble] = empty;

      if (!empty) {
        bottom_node.hash[last_nibble] =
            mptrie::hash_of_leaves(range_begin, range_end);
      }

      bottom_node.synced[last_nibble] = true;
    }

    // propagate up the subtree of main_node
    for (unsigned level = depth() - 1; level >= prefix.size(); --level) {
      auto sub_prfx = Prefix{level, prefix.val()};
      const auto shift = 4 * (level - prefix.size());

      for (uint64_t i = 0; i < (1ull << shift); ++i, ++sub_prfx) {
        const auto last_nibble = sub_prfx[level - 1];
        auto& parent = node(level - 1, sub_prfx);
        auto& child = node(level, sub_prfx);

        child.block = rb;

        parent.empty[last_nibble] = child.empty.all();
        if (!parent.empty[last_nibble]) {
          parent.hash[last_nibble] =
              mptrie::branch_node_hash(child.empty, child.hash);
        }
        parent.synced[last_nibble] = child.synced.all();
      }
    }
  }

  // update the nodes up the tree path
  const unsigned start_from = prefix.size() - reply.proof.size();
  for (unsigned level = start_from; level < prefix.size(); ++level) {
    Node& path_node = node(level, prefix);
    if (path_node.block < rb) {
      const auto& proof = reply.proof[level - start_from];
      path_node.empty = proof.empty;
      path_node.hash = proof.hash;
      path_node.block = rb;
    }
  }

  // TODO reset synced if hash changed
}

}  // namespace silkworm
