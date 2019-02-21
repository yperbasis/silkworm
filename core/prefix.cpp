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

#include "rlp.hpp"

namespace silkworm {

Prefix::Prefix(size_t size, uint64_t val) : size_(size), val_(val) {
  if (size > 16) {
    throw std::length_error(
        "only prefixes up to 16 nibbles are currently supported");
  }
  const auto shift = 64u - size * 4;
  if (val << shift) {
    throw std::invalid_argument("non-zero padding");
  }
}

bool Prefix::matches(const Hash& hash) const {
  for (unsigned i = 0; i < size_ / 2; ++i) {
    const uint8_t byte = (val_ << (8 * i)) >> (8 * 7);
    if (byte != hash[i]) {
      return false;
    }
  }
  if (size_ % 2 == 0) {
    return true;
  }
  Nibble last_nibble = (*this)[size_ - 1];
  return (hash[size_ / 2] >> 4) == last_nibble;
}

Hash Prefix::padded() const {
  Hash array;
  for (unsigned i = 0; i < 8; ++i) {
    array[i] = (val_ << (8 * i)) >> (8 * 7);
  }
  for (unsigned i = 8; i < kHashBytes; ++i) {
    array[i] = 0;
  }
  return array;
}

Prefix operator"" _prefix(const char* in, std::size_t n) {
  std::string str(8 * 2, '0');
  std::copy_n(in, n, str.begin());
  const std::string bytes = hex_string_to_bytes(str);
  uint64_t val = rlp::to_integer(bytes);
  return Prefix{n, val};
}

}  // namespace silkworm
