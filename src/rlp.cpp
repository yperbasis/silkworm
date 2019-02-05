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

#include "rlp.hpp"

namespace {
using namespace silkworm::rlp;

std::string encode_length(uint64_t len, char offset) {
  if (len < 56) {
    return {static_cast<char>(len + offset)};
  } else {
    auto bl = to_binary(len);
    return static_cast<char>(bl.length() + offset + 55) + bl;
  }
}
}  // namespace

namespace silkworm::rlp {

std::string encode(const Item& var) {
  return std::visit(
      [](auto&& input) {
        using T = std::decay_t<decltype(input)>;
        if constexpr (std::is_same_v<T, std::string>) {
          if (input.length() == 1 && input[0] < 0x80)
            return input;
          else
            return encode_length(input.length(), 0x80) + input;
        } else {
          // TODO: implement
          return std::string{};
        }
      },
      var);
}

Item decode(const std::string&) {
  // TODO: implement
  return "";
}

std::string to_binary(uint64_t x) {
  if (x == 0)
    return "";
  else
    return to_binary(x / 256) + static_cast<char>(x % 256);
}
}  // namespace silkworm::rlp
