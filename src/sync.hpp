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

#ifndef SILKWORM_SYNC_HPP_
#define SILKWORM_SYNC_HPP_

#include <stdint.h>
#include <string>
#include <variant>
#include <vector>

#include "keccak.hpp"

// TODO sync researh:
// 0. in-memory single-machine PoC with ~1m dust accounts
// 1. database with ~100m dust accounts
// 2. network layer
// 3. real protocol
// 4. proof checking
// 5. smart contract storage
// 6. compare against geth's fast sync
// 7. real historical Ethereum data

namespace silkworm::sync {

using Nibble = char;

struct Request {
  int32_t block = -1;
  std::vector<Nibble> prefix;
};

enum Error {
  kDontHaveData = 1,
  kTooManyLeaves = 2,
};

struct Leaf {
  std::vector<Nibble> midfix;
  std::string value;
};

struct Proof {
  uint16_t mask;
  std::vector<UInt256> hashes;  // hashes.size() == #bits(mask)
};

struct Reply {
  int32_t block = 0;
  std::vector<Proof> proofs;  // proofs.size() == prefix.size()
  std::vector<Leaf> leaves;
};

}  // namespace silkworm::sync

#endif  // SILKWORM_SYNC_HPP_
