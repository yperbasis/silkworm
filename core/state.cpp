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

State::State(DbBucket& db, unsigned level)
    : db_(db), tree_(level + 1), sync_request_cursor_(Prefix{level + 1}) {
  if (level > 15) {
    throw std::length_error("level is too deep");
  }

  for (unsigned i = 0; i <= level; ++i) {
    tree_[i].resize(1ull << (i * 4));
  }
}

void State::init_from_db(uint32_t data_valid_for_block) {
  auto prefix = Prefix(level() + 1);

  for (uint64_t i = 0; i < tree_[level()].size(); ++i) {
    auto& nodes = tree_[level()];

    for (unsigned j = 0; j < 16; ++j) {
      const auto leaves = db_.leaves(prefix);

      const bool empty = leaves.first == leaves.second;
      nodes[i].empty[j] = empty;

      if (!empty) {
        nodes[i].hash[j] = mptrie::hash_of_leaves(level() + 1, leaves);
        nodes[i].leaves_in_db[j] = true;
      }

      if (prefix.next()) {
        prefix = *prefix.next();
      } else {
        assert(i == nodes.size() - 1);
        assert(j == 15);
      }

      nodes[i].block = data_valid_for_block;
    }
  }

  for (int lvl = static_cast<int>(level()) - 1; lvl >= 0; --lvl) {
    auto& nodes = tree_[lvl];

    for (uint64_t i = 0; i < nodes.size(); ++i) {
      for (unsigned j = 0; j < 16; ++j) {
        const auto& child = tree_[lvl + 1][i * 16 + j];
        const bool empty = child.empty.all();
        nodes[i].empty[j] = empty;
        if (!empty) {
          nodes[i].hash[j] = mptrie::branch_node_hash(child.empty, child.hash);
          nodes[i].leaves_in_db[j] = true;
        }
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
  if (prefix.size() != level() + 1) {
    throw std::runtime_error(
        "TODO prefix.size() != level() + 1 not implemented yet");
  }

  if (consistent_path_depth(prefix) != prefix.size()) {
    return sync::kDontHaveData;
  }

  const Node& x = node(level(), prefix);

  const Nibble last_nibble = prefix[level()];
  if (!x.empty[last_nibble] && !x.leaves_in_db[last_nibble]) {
    return sync::kDontHaveData;
  }

  auto rb = request.block ? static_cast<int32_t>(*request.block) : -1;
  if (rb > x.block) {
    return sync::kDontHaveData;
  }

  sync::Reply reply(prefix, x.block);

  const bool full_proof = rb < x.block;
  const unsigned proof_start = full_proof ? 0 : request.start_proof_from;

  for (unsigned i = proof_start; i <= level(); ++i) {
    const auto& y = node(i, prefix);
    reply.proof.push_back(sync::Proof{y.empty, y.hash});
  }

  const bool leaves_required =
      !request.hash_of_leaves || *request.hash_of_leaves != x.hash[last_nibble];

  if (leaves_required) {
    reply.leaves = std::vector<sync::Leaf>{};
    if (!x.empty[last_nibble]) {
      const auto db_leaves = db_.leaves(prefix);
      for (auto it = db_leaves.first; it != db_leaves.second; ++it) {
        reply.leaves->push_back(sync::Leaf{it->first, it->second});
      }
    }
  }

  return reply;
}

std::optional<sync::Request> State::next_sync_request() {
  if (!sync_request_cursor_) {
    // TODO check if we still have data gaps
    return {};
  }

  const auto prefix = *sync_request_cursor_;
  sync::Request request{prefix};

  if (root().block != -1) {
    request.block = root().block;
  }

  request.start_proof_from = consistent_path_depth(prefix);

  const auto& nd = node(level(), prefix);
  const Nibble x = prefix[level()];
  if (!nd.empty[x] && nd.leaves_in_db[x]) {
    request.hash_of_leaves = nd.hash[x];
  }

  // TODO some kind of randomisation instead
  sync_request_cursor_ = sync_request_cursor_->next();

  return request;
}

void State::process_sync_data(const sync::Reply& reply) {
  const auto prefix = reply.prefix;
  if (prefix.size() != level() + 1) {
    throw std::runtime_error(
        "TODO prefix.size() != level() + 1 not implemented yet");
  }

  int32_t rb = reply.block;

  auto& last_node = node(level(), prefix);

  if (last_node.block > rb) {
    return;  // old reply
  }

  // TODO check proof
  // TODO if !reply.leaves, check that's legit
  // TODO otherwise check leaves match the prefix
  // and their hash matches proof

  const unsigned start_from = prefix.size() - reply.proof.size();
  for (unsigned level = start_from; level < prefix.size(); ++level) {
    Node& x = node(level, prefix);
    if (x.block < rb) {
      x.empty = reply.proof[level - start_from].empty;
      x.hash = reply.proof[level - start_from].hash;
      x.block = rb;
    }
  }

  // TODO clean the database for stale nibbles and reset leaves_in_db

  if (!reply.leaves) {
    return;
  }

  for (const auto& x : *reply.leaves) {
    db_.put(x.hash_key, x.value);
  }

  const auto last_nibble = prefix[level()];
  last_node.leaves_in_db[last_nibble] = !last_node.empty[last_nibble];
}

}  // namespace silkworm
