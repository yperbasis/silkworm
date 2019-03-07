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

#include "rlp.hpp"

namespace silkworm::mptrie {

// https://github.com/ethereum/wiki/wiki/Patricia-Tree
Hash branch_node_hash(std::bitset<16> empty, const std::array<Hash, 16>& hash) {
  thread_local rlp::List rlp(16, "");

  for (Nibble i = 0; i < 16; ++i) {
    auto& str = boost::get<std::string>(rlp[i]);
    if (empty[i]) {
      str.clear();
    } else {
      str = byte_view(hash[i]);
    }
  }

  thread_local std::string out;
  out.clear();
  rlp::encode(rlp, out);

  return keccak(out);
}

}  // namespace silkworm::mptrie
