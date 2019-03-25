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
#include <string>
#include <string_view>

#include "common.hpp"
#include "keccak.hpp"

// Things related to the Modified Merkle Patricia Trie
// https://github.com/ethereum/wiki/wiki/Patricia-Tree

namespace silkworm {

class LeafHasher {
 public:
  void append(std::string_view key, std::string_view val);

  bool empty() const { return empty_; }

  // TODO implement properly (extension nodes, etc)
  // https://github.com/AlexeyAkhunov/go-ethereum/blob/a9dd04dbc1908aa43e0033d0fe00c8445a47a280/trie/resolver.go#L369
  Hash hash() const { return keccak(joint_leaves_); }

 private:
  std::string joint_leaves_;
  bool empty_ = true;
};

namespace mptrie {

Hash branch_node_hash(std::bitset<16> empty, const std::array<Hash, 16>& hash);

}  // namespace mptrie

}  // namespace silkworm

#endif  // SILKWORM_CORE_MPTRIE_HPP_
