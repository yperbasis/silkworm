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

#include "account.hpp"

#include "rlp.hpp"

namespace silkworm {

std::string to_rlp(const Account& in) {
  const rlp::List list = {
      rlp::to_binary(in.nonce),
      rlp::to_binary(in.balance),
      std::string(byte_view(in.storage)),
      std::string(byte_view(in.code)),
  };
  return rlp::encode(list);
}
}  // namespace silkworm
