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
  * single-machine in-memory PoC with ~1m dust accounts
  * describe the algo
[TODO]
  * remove subtrieHash from the doc
  * theoretical convergence
  * LMDB(?) database
  * experimental convergence with ~100m dust accounts
  * storage tries. pack multiple accounts into 1 request/reply for small tries
  * extension/leaf nodes (prereq: Issue #7, better test coverage)
  * handle chain reorgs
  * remove blockAtLeast from the protocol
  * use the real protocol in the modelling code
  * network layer, p2p
  * multiple leechers, BitTorrent-like swarm
  * leaf proof checking
  * compare against geth's fast sync and parity's warp sync
  * real historical Ethereum data
*/

namespace silkworm::sync {

// TODO move to protocol
struct GetLeavesRequest {
  // {} account means state rather than storage trie
  std::optional<Address> account = {};

  // request all leaves with this prefix
  Prefix prefix;

  // may not respond with older data
  std::optional<uint32_t> block_number;

  // If from_level is greater than zero, the given number of trie nodes closest
  // to the root must be omitted from the proof.
  // Only applies if respond.block_number = request.block_number,
  // otherwise full proof is required.
  uint8_t from_level = 0;  // <= prefix.size

  explicit GetLeavesRequest(Prefix prefix) : prefix{prefix} {}

  size_t byte_size() const {
    return sizeof(*this) + (account ? kAddressBytes : 0);
  }
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
  enum Status {
    kOK = 0,
    kDontHaveData,
    kTooManyLeaves,  // TODO how many is too many?
  };

  // still send proof if status = kTooManyLeaves
  Status status = kOK;

  // must be >= request.block_number
  uint32_t block_number = 0;

  // If block_number = request.block_number
  // proof.size = prefix.size - from_level // TODO extension/leaf nodes
  // else if block_number > request.block_number || request.block_number not set
  // proof.size = prefix.size
  std::vector<Proof> proof;

  // don't send leaves if hash(leaves) = request.hash, but send proof anyway
  std::optional<std::vector<Leaf>>
      leaves;  // must be strictly ordered by hash_key

  size_t byte_size() const {
    auto sz = sizeof(*this) + proof.size() * sizeof(Proof);
    if (leaves) {
      sz += leaves->size() * kLeafSize;
    }
    return sz;
  }
};

// uses prefixes unlike PV63
struct GetNodeRequest {
  // {} account means state rather than storage trie
  std::optional<Address> account = {};

  std::vector<Prefix> prefixes;

  // may not respond with older data
  std::optional<uint32_t> block_number;

  size_t byte_size() const {
    return sizeof(*this) + (account ? kAddressBytes : 0) +
           prefixes.size() * sizeof(Prefix);
  }
};

struct NodeReply {
  // must be >= request.block_number
  uint32_t block_number = 0;

  std::vector<std::optional<Proof>> nodes;

  size_t byte_size() const {
    return sizeof(*this) + nodes.size() * sizeof(Proof);
  }
};

// TODO: GetStorageSize

struct Stats {
  uint64_t num_requests = 0;
  uint64_t request_total_bytes = 0;
  uint64_t num_replies = 0;
  uint64_t reply_total_bytes = 0;
  uint64_t reply_total_leaves = 0;
  uint64_t reply_total_nodes = 0;
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

  uint8_t depth_to_fit_in_memory() const;

  // not taking depth_to_fit_in_memory into account
  uint8_t optimal_phase2_depth() const;
  uint8_t optimal_phase1_depth() const;
  double inf_bandwidth_reply_overhead() const;  // compared to warp sync

  static uint64_t num_tree_nodes(uint8_t depth) {
    return ((1ull << (4 * depth)) - 1) / 15;
  }

  uint64_t tree_size_in_bytes(uint8_t depth) const {
    const auto tree_overhead = depth * 8;
    return num_tree_nodes(depth) * node_size + tree_overhead;
  }
};

}  // namespace silkworm::sync

#endif  // SILKWORM_CORE_SYNC_HPP_
