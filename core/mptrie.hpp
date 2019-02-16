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

#ifndef SILKWORM_CORE_MPTRIE_HPP_
#define SILKWORM_CORE_MPTRIE_HPP_

#include <array>
#include <bitset>

#include "common.hpp"
#include "db_bucket.hpp"

// Things related to the Modified Merkle Patricia Trie
// https://github.com/ethereum/wiki/wiki/Patricia-Tree

namespace silkworm::mptrie {

Hash hash_of_leaves(uint8_t level, DbBucket::Range);

Hash branch_node_hash(std::bitset<16> empty, const std::array<Hash, 16>& hash);

}  // namespace silkworm::mptrie

#endif  // SILKWORM_CORE_MPTRIE_HPP_
