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
#include <utility>

#include "common.hpp"

namespace silkworm {

class Prefix {
 public:
  explicit Prefix(uint8_t size, uint64_t val = 0);
  Prefix(uint8_t size, const Hash&);

  uint8_t size() const { return size_; }

  uint64_t val() const { return val_; }

  Nibble operator[](uint8_t pos) const { return (val_ << (pos * 4)) >> 60; }

  Nibble last() const { return (*this)[size_ - 1]; }

  void set(uint8_t pos, Nibble x) {
    const auto shift = 60 - pos * 4;
    val_ = (val_ & ~(0xfull << shift)) + (static_cast<uint64_t>(x) << shift);
  }

  Prefix& operator++() { return (*this) += 1; }

  Prefix& operator+=(uint64_t inc) {
    if (size_) {
      const auto shift = 64 - size_ * 4;
      val_ += inc << shift;
    }
    return *this;
  }

  friend Prefix operator+(Prefix prfx, uint64_t inc) {
    prfx += inc;
    return prfx;
  }

  Hash padded() const;

  std::string to_string() const;

  bool matches(const Hash&) const;

  friend Prefix operator"" _prefix(const char* in, size_t n);

  friend inline bool operator==(const Prefix& a, const Prefix& b) {
    return a.val_ == b.val_ && a.size_ == b.size_;
  }

 private:
  uint8_t size_ = 0;
  uint64_t val_ = 0;

  uint8_t byte(uint8_t i) const { return (val_ << (8 * i)) >> (8 * 7); }
};

inline bool operator!=(const Prefix& a, const Prefix& b) { return !(a == b); }

Prefix operator"" _prefix(const char* in, std::size_t n);

}  // namespace silkworm

#endif  // SILKWORM_CORE_PREFIX_HPP_
