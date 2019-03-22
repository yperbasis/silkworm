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
#include "keccak.hpp"
#include "memdb_bucket.hpp"

// Things related to the Modified Merkle Patricia Trie
// https://github.com/ethereum/wiki/wiki/Patricia-Tree

namespace silkworm::mptrie {

// TODO implement properly (extension nodes, etc)
// https://github.com/AlexeyAkhunov/go-ethereum/blob/a9dd04dbc1908aa43e0033d0fe00c8445a47a280/trie/resolver.go#L369
template <class ConstIterator>
Hash hash_of_leaves(ConstIterator begin, ConstIterator end) {
  if (begin == end) {
    return kEmptyStringHash;
  }

  std::string joint_leaves;
  for (auto it = begin; it != end; ++it) {
    const auto& leaf_val = it->second;
    joint_leaves += byte_view(keccak(leaf_val));
  }
  return keccak(joint_leaves);
}

Hash branch_node_hash(std::bitset<16> empty, const std::array<Hash, 16>& hash);

}  // namespace silkworm::mptrie

#endif  // SILKWORM_CORE_MPTRIE_HPP_
