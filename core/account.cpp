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
  thread_local rlp::List list = {"", "", "", ""};
  boost::get<std::string>(list[0]) = rlp::to_big_endian(in.nonce);
  boost::get<std::string>(list[1]) = rlp::to_big_endian(in.balance);
  boost::get<std::string>(list[2]) = byte_view(in.storage);
  boost::get<std::string>(list[3]) = byte_view(in.code);

  return rlp::encode(list);
}
}  // namespace silkworm
