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
  explicit Prefix(size_t size, uint64_t val = 0);

  size_t size() const { return size_; }

  Nibble operator[](size_t pos) const { return (val_ << (pos * 4)) >> 60; }

  void set(size_t pos, Nibble x) {
    const auto shift = 60 - pos * 4;
    val_ = (val_ & ~(0xfull << shift)) + (static_cast<uint64_t>(x) << shift);
  }

  std::optional<Prefix> next() const;

  Hash padded() const;

  friend Prefix operator"" _prefix(const char* in, std::size_t n);

  friend inline bool operator==(const Prefix& a, const Prefix& b) {
    return a.val_ == b.val_ && a.size_ == b.size_;
  }

 private:
  uint64_t val_ = 0;
  size_t size_ = 0;
};

inline bool operator!=(const Prefix& a, const Prefix& b) { return !(a == b); }

Prefix operator"" _prefix(const char* in, std::size_t n);

}  // namespace silkworm

#endif  // SILKWORM_CORE_PREFIX_HPP_
