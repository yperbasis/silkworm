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

#include "miner.hpp"

namespace silkworm {

bool Miner::new_block() {
  if (state_.synced_block() < 0) {
    return false;
  }

  new_block_ = state_.synced_block() + 1;
  return true;
}

void Miner::create_account(const Address& address, const Account& account) {
  state_.put(keccak(byte_view(address)), to_rlp(account));
}

bool Miner::seal_block() {
  if (new_block_ == 0) {
    return false;
  }

  state_.init_from_db(new_block_);

  new_block_ = 0;
  return true;
}

}  // namespace silkworm
