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
  int32_t block = -1;
  Prefix prefix;
  uint8_t start_proof_from = 0;  // start_proof_from <= prefix.size
};

enum Error {
  kDontHaveData = 1,
  kTooManyLeaves = 2,
};

struct Leaf {
  std::string hash_key;
  std::string value;
};

struct Reply {
  // must be >= request.block
  int32_t block = 0;

  // If block = request.block
  // proof.size = prefix.size - start_proof_from
  // else if block > request.block
  // proof.size = prefix.size
  std::vector<std::array<Hash, 16>> proof;

  // must be ordered by hash_key
  std::vector<Leaf> leaves;
};

}  // namespace silkworm::sync

#endif  // SILKWORM_CORE_SYNC_HPP_
