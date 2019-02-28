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

#ifndef SILKWORM_CORE_ACCOUNT_HPP_
#define SILKWORM_CORE_ACCOUNT_HPP_

#include "keccak.hpp"

namespace silkworm {

struct Account {
  // should be UInt256 instead according to Yellow Paper's Eq (12)
  uint64_t nonce = 0;
  UInt256 balance = 0;
  Hash storage = kEmptyStringHash;
  Hash code = kEmptyStringHash;
};

std::string to_rlp(const Account&);

}  // namespace silkworm

#endif  // SILKWORM_CORE_ACCOUNT_HPP_
