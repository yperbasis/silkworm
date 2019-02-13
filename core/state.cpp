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

#include "state.hpp"
#include "keccak.hpp"
#include "rlp.hpp"

namespace silkworm {

void State::init_from_db(uint32_t block_height) {
  // TODO build the chunks from the database
  // (calculate hashes w/o extension nodes to start with)
  db_.put(kEmptyStringHash,
          rlp::to_binary(block_height));  // TODO remove this shite
}

}  // namespace silkworm
