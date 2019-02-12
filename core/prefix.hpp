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

#ifndef SILKWORM_CORE_PREFIX_HPP_
#define SILKWORM_CORE_PREFIX_HPP_

#include <algorithm>
#include <optional>
#include <utility>

#include "common.hpp"

namespace silkworm {

class Prefix {
 public:
  Prefix(std::string bytes, bool odd) : bytes_(std::move(bytes)), odd_(odd) {}

  size_t size() const { return bytes_.size() * 2 - odd_; }

  Nibble operator[](size_t pos) const {
    uint8_t byte = bytes_[pos / 2];
    if (pos % 2)
      return byte & 0x0f;
    else
      return byte >> 4;
  }

  void set(size_t pos, Nibble val) {
    uint8_t byte = bytes_[pos / 2];
    if (pos % 2)
      bytes_[pos / 2] = (byte & 0xf0) + val;
    else
      bytes_[pos / 2] = (byte & 0x0f) + (val << 4);
  }

  std::optional<Prefix> next() const;

  Hash padded() const {
    Hash array;
    auto it = std::copy(bytes_.begin(), bytes_.end(), array.begin());
    std::fill(it, array.end(), 0);
    return array;
  }

  friend inline bool operator==(const Prefix& a, const Prefix& b) {
    return a.odd_ == b.odd_ && a.bytes_ == b.bytes_;
  }

 private:
  std::string bytes_;
  bool odd_ = false;
};

inline bool operator!=(const Prefix& a, const Prefix& b) { return !(a == b); }

Prefix operator"" _prefix(const char* in, std::size_t n);

}  // namespace silkworm

#endif  // SILKWORM_CORE_PREFIX_HPP_
