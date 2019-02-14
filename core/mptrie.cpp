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

namespace silkworm {

Hash hash_of_leaves(uint8_t, DbBucket::Range range) {
  if (range.first == range.second) {
    return kEmptyStringHash;
  }
  // TODO implement properly
  std::string joint_leaves;
  joint_leaves.reserve(kHashBytes * 1000);
  for (auto it = range.first; it != range.second; ++it) {
    joint_leaves += byte_view(keccak(it->second));
  }
  return keccak(joint_leaves);
}

}  // namespace silkworm
