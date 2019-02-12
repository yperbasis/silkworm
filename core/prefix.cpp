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

#include "prefix.hpp"

#include <iterator>

namespace silkworm {

std::optional<Prefix> Prefix::next() const {
  BigInt next_int;
  import_bits(next_int, bytes_.begin(), bytes_.end(), 8);
  next_int += odd_ ? 16 : 1;

  std::string next_str;
  next_str.reserve(bytes_.size());
  export_bits(next_int, std::back_inserter(next_str), 8);

  if (next_str.size() > bytes_.size())
    return {};
  else
    return Prefix{next_str, odd_};
}

Prefix operator"" _prefix(const char* in, std::size_t n) {
  const bool odd = n % 2;
  std::string str(in, n);
  if (odd) {
    str += '0';
  }
  std::string bytes = hex_string_to_bytes(str);
  return {bytes, odd};
}

}  // namespace silkworm
