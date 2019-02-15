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

namespace {
bool is_bit_set(uint16_t num, unsigned bit) { return (num >> bit) & 1; }
void set_bit(uint16_t& num, unsigned bit) { num |= (1u << bit); }
// void clear_bit(uint16_t& num, unsigned bit) { num &= ~(1u << bit); }
}  // namespace

namespace silkworm {

State::State(DbBucket& db, unsigned level) : db_(db), tree_(level + 1) {
  if (level > 15) {
    throw std::length_error("level is too deep");
  }

  for (unsigned i = 0; i <= level; ++i) {
    tree_[i].resize(1ull << (i * 4));
  }
}

void State::init_from_db(uint32_t data_valid_for_block) {
  auto prefix = Prefix(level() + 1);
  auto& nodes = tree_[level()];

  for (unsigned i = 0; i < nodes.size(); ++i) {
    for (unsigned j = 0; j < 16; ++j) {
      const auto leaves = db_.leaves(prefix);
      Hash hash = hash_of_leaves(level() + 1, leaves);
      nodes[i].hash[j] = hash;
      nodes[i].block = data_valid_for_block;
      nodes[i].have_data = -1;  // set all bits to true
      if (prefix.next()) {
        prefix = *prefix.next();
      } else {
        assert(i == nodes.size() - 1);
        assert(j == 15);
        return;
      }
    }
  }
}

std::variant<sync::Reply, sync::Error> State::get_leaves(
    const sync::Request& request) const {
  const auto prefix = request.prefix;
  if (prefix.size() != level() + 1) {
    throw std::runtime_error(
        "TODO prefix.size() != level() + 1 not implemented yet");
  }

  auto& nodes = tree_[level()];
  const uint64_t node_num = prefix.val() >> (64 - level() * 4);

  Nibble last_nibble = prefix[level()];
  if (!is_bit_set(nodes[node_num].have_data, last_nibble)) {
    return sync::kDontHaveData;
  }

  if (request.block && *request.block > nodes[node_num].block) {
    return sync::kDontHaveData;
  }

  sync::Reply reply{prefix, nodes[node_num].block};

  // TODO proof

  const bool leaves_required =
      !request.hash_of_leaves ||
      *request.hash_of_leaves != nodes[node_num].hash[last_nibble];

  if (leaves_required) {
    reply.leaves = std::vector<sync::Leaf>{};
    const auto db_leaves = db_.leaves(prefix);
    for (auto it = db_leaves.first; it != db_leaves.second; ++it) {
      reply.leaves->push_back(sync::Leaf{it->first, it->second});
    }
  }

  return reply;
}

std::optional<sync::Request> State::next_sync_request() {
  const unsigned shift = (level() + 1) * 4;

  if (sync_request_cursor_ >= (1ull << shift)) {
    // TODO check if we still have data gaps
    return {};
  }

  auto prefix = Prefix(level() + 1, sync_request_cursor_ << (64u - shift));
  sync::Request request{prefix};

  // TODO block, start_proof_from, hash_of_leaves

  // TODO some kind of randomisation instead
  ++sync_request_cursor_;

  return request;
}

void State::process_sync_data(const sync::Reply& reply) {
  const auto prefix = reply.prefix;
  if (prefix.size() != level() + 1) {
    throw std::runtime_error(
        "TODO prefix.size() != level() + 1 not implemented yet");
  }

  auto& nodes = tree_[level()];
  const uint64_t node_num = prefix.val() >> (64 - level() * 4);

  if (nodes[node_num].have_data && nodes[node_num].block > reply.block) {
    return;  // old reply
  }

  // TODO check proof
  // TODO if !reply.leaves, check that's legit
  // TODO otherwise check leaves match the prefix
  // and their hash matches proof

  // TODO clear have_data for stale nibbles && clean the database
  // TODO update nodes.hash

  nodes[node_num].block = reply.block;

  if (!reply.leaves) {
    return;
  }

  for (const auto& x : *reply.leaves) {
    db_.put(x.hash_key, x.value);
  }

  Nibble last_nibble = prefix[level()];
  set_bit(nodes[node_num].have_data, last_nibble);
}

}  // namespace silkworm
