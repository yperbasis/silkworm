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

// TODO sync research:
// 0. single-machine in-memory PoC with ~1m dust accounts
// 1. database with ~100m dust accounts
// 2. network layer, p2p
// 3. real protocol
// 4. proof checking
// 5. smart contract storage
// 6. compare against geth's fast sync
// 7. real historical Ethereum data

namespace silkworm::sync {

// GetLeaves
struct Request {
  // request all leaves with this prefix
  Prefix prefix;

  // only applies if respond.block = request.block
  // otherwise full proof is required
  uint8_t start_proof_from = 0;  // <= prefix.size

  // may not respond with older data
  std::optional<uint32_t> block;

  // known hash(leaves), allows to avoid re-sending the same leaves
  std::optional<Hash> hash_of_leaves;

  explicit Request(Prefix prefix) : prefix{prefix} {}

  size_t byte_size() const {
    return sizeof(this) + (hash_of_leaves ? kHashBytes : 0);
  }
};

enum Error {
  kDontHaveData = 1,
  kTooManyLeaves = 2,  // TODO how many is too many?
};

struct Leaf {
  Hash hash_key;
  std::string value;
};

// a reasonable approximation for dust accounts
static constexpr size_t kLeafSize = sizeof(Leaf) + 80;

inline bool operator==(const Leaf& a, const Leaf& b) {
  return a.hash_key == b.hash_key && a.value == b.value;
}

struct Proof {
  std::bitset<16> empty = std::bitset<16>{}.flip();
  std::array<Hash, 16> hash;
};

struct Reply {
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
  std::optional<std::vector<Leaf>> leaves;  // must be ordered by hash_key

  Reply(Prefix prefix, uint32_t block) : prefix{prefix}, block{block} {}

  size_t byte_size() const {
    auto sz = sizeof(this) + proof.size() * sizeof(Proof);
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

}  // namespace silkworm::sync

#endif  // SILKWORM_CORE_SYNC_HPP_
