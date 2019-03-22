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

Prefix::Prefix(uint8_t size, uint64_t val) : size_(size), val_(val) {
  if (size > 16) {
    throw std::length_error(
        "only prefixes up to 16 nibbles are currently supported");
  }
  const auto shift = 64 - size * 4;
  if ((size_ == 0 && val_ != 0) || (val << shift) != 0) {
    throw std::invalid_argument("non-zero padding");
  }
}

Prefix::Prefix(uint8_t size, const Hash& hash) : size_(size) {
  if (size > 16) {
    throw std::length_error(
        "only prefixes up to 16 nibbles are currently supported");
  }

  val_ = 0;
  for (int i = 0; i < 8; ++i) {
    val_ *= 0x100;
    val_ += hash[i];
  }

  for (auto pos = size; pos < 16; ++pos) {
    set(pos, 0);
  }
}

bool Prefix::matches(const Hash& hash) const {
  for (uint8_t i = 0; i < size_ / 2; ++i) {
    if (byte(i) != hash[i]) {
      return false;
    }
  }
  if (size_ % 2 == 0) {
    return true;
  }
  Nibble last_nibble = (*this)[size_ - 1];
  return (hash[size_ / 2] >> 4) == last_nibble;
}

std::string Prefix::to_string() const {
  std::string str(size_ / 2 + size_ % 2, '\0');
  for (uint8_t i = 0; i < str.size(); ++i) {
    str[i] = byte(i);
  }
  return str;
}

std::pair<std::string, std::optional<std::string>> Prefix::string_range()
    const {
  const auto first = to_string();
  const auto next = *this + 1;
  if (next.val() == 0)
    return {first, {}};
  else
    return {first, next.to_string()};
}

Hash Prefix::padded() const {
  Hash array = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  for (uint8_t i = 0; i < 8; ++i) {
    array[i] = byte(i);
  }
  return array;
}

Prefix operator"" _prefix(const char* in, size_t n) {
  std::string str(8 * 2, '0');
  std::copy_n(in, n, str.begin());
  const std::string bytes = hex_string_to_bytes(str);
  const uint64_t val = rlp::from_big_endian(bytes);
  return Prefix{static_cast<uint8_t>(n), val};
}

}  // namespace silkworm
