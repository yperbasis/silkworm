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

#ifndef SILKWORM_CORE_SYNC_HPP_
#define SILKWORM_CORE_SYNC_HPP_

#include <bitset>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "common.hpp"
#include "prefix.hpp"

/* Sync Research
[DONE]
  0. single-machine in-memory PoC with ~1m dust accounts
  1. optimal phase 2 depth
[TODO]
  1. describe the algo. perhaps we need to use geth's fast sync for some bits
  2. theoretical convergence (if possible)
  3. LMDB(?) database
  4. experimental convergence with ~100m dust accounts
  5. smart contract storage
  6. extension/leaf nodes
  7. real protocol + doc
  8. network layer, p2p
  9. multiple leechers, BitTorrent-like swarm
  10. proof checking
  11. compare against geth's fast sync and parity's warp sync
  12. real historical Ethereum data
*/

namespace silkworm::sync {

// TODO move to protocol
struct GetLeavesRequest {
  // request all leaves with this prefix
  Prefix prefix;

  // only applies if respond.block = request.block
  // otherwise full proof is required
  uint8_t start_proof_from = 0;  // <= prefix.size

  // may not respond with older data
  std::optional<uint32_t> block;

  // known hash(leaves), allows to avoid re-sending the same leaves
  std::optional<Hash> hash_of_leaves;

  explicit GetLeavesRequest(Prefix prefix) : prefix{prefix} {}

  size_t byte_size() const {
    return sizeof(*this) + (hash_of_leaves ? kHashBytes : 0);
  }
};

enum Error {
  kDontHaveData = 1,
  kTooManyLeaves = 2,  // TODO how many is too many?
};

using Leaf = std::pair<Hash, std::string>;

// a reasonable approximation for dust accounts
static constexpr size_t kLeafSize = sizeof(Leaf) + 80;

// TODO: extension/leaf nodes
struct Proof {
  std::bitset<16> empty = std::bitset<16>{}.flip();
  std::array<Hash, 16> hash;
};

struct LeavesReply {
  // must = request.prefix
  Prefix prefix;

  // must be >= request.block
  uint32_t block = 0;

  // If block = request.block
  // proof.size = prefix.size - start_proof_from
  // else if block > request.block || request.block not set
  // proof.size = prefix.size
  std::vector<Proof> proof;

  // don't send leaves if hash(leaves) = request.hash, but send proof anyway
  std::optional<std::vector<Leaf>>
      leaves;  // must be strictly ordered by hash_key

  LeavesReply(Prefix prefix, uint32_t block) : prefix{prefix}, block{block} {}

  size_t byte_size() const {
    auto sz = sizeof(*this) + proof.size() * sizeof(Proof);
    if (leaves) {
      sz += leaves->size() * kLeafSize;
    }
    return sz;
  }
};

// TODO (potentially) delta request & reply

struct Stats {
  uint64_t num_requests = 0;
  uint64_t request_total_bytes = 0;
  uint64_t num_replies = 0;
  uint64_t reply_total_bytes = 0;
  uint64_t reply_total_leaves = 0;
};

// all sizes are in bytes
struct Hints {
  uint64_t max_memory = 8 * 1024 * 1024 * 1024ll;
  uint64_t num_leaves = 50'000'000;

  unsigned approx_max_reply_size = 32 * 1024;

  unsigned proof_size = sizeof(Proof);
  unsigned node_size = proof_size + 8;
  unsigned leaf_size = kLeafSize;
  unsigned reply_overhead = sizeof(LeavesReply);

  unsigned depth_to_fit_in_memory() const;

  // not taking depth_to_fit_in_memory into account
  unsigned optimal_phase2_depth() const;
  unsigned optimal_phase1_depth() const;
  double inf_bandwidth_reply_overhead() const;  // compared to warp sync

  static uint64_t num_tree_nodes(unsigned depth) {
    return ((1ull << (4 * depth)) - 1) / 15;
  }

  uint64_t tree_size_in_bytes(unsigned depth) const {
    const auto tree_overhead = depth * 8;
    return num_tree_nodes(depth) * node_size + tree_overhead;
  }
};

}  // namespace silkworm::sync

#endif  // SILKWORM_CORE_SYNC_HPP_
