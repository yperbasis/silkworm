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

#include "mptrie.hpp"

#include "keccak.hpp"
#include "rlp.hpp"

namespace silkworm::mptrie {

Hash hash_of_leaves(uint8_t, DbBucket::Range range) {
  if (range.first == range.second) {
    return kEmptyStringHash;
  }
  // TODO implement properly (extension nodes, etc)
  // https://github.com/AlexeyAkhunov/go-ethereum/blob/a9dd04dbc1908aa43e0033d0fe00c8445a47a280/trie/resolver.go#L369
  std::string joint_leaves;
  joint_leaves.reserve(kHashBytes * 1000);
  for (auto it = range.first; it != range.second; ++it) {
    const auto& leaf_val = it->second;
    joint_leaves += byte_view(keccak(leaf_val));
  }
  return keccak(joint_leaves);
}

// https://github.com/ethereum/wiki/wiki/Patricia-Tree
Hash branch_node_hash(std::bitset<16> empty, const std::array<Hash, 16>& hash) {
  rlp::List rlp(16);
  for (unsigned i = 0; i < 16; ++i) {
    rlp[i] = !empty[i] ? std::string{byte_view(hash[i])} : "";
  }
  return keccak(rlp::encode(rlp));
}

}  // namespace silkworm::mptrie
