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
#ifndef SILKWORM_CORE_COMMON_HPP_
#define SILKWORM_CORE_COMMON_HPP_

#include <stdint.h>
#include <array>
#include <string>
#include <string_view>

#include <boost/multiprecision/cpp_int.hpp>

namespace silkworm {

using BigInt = boost::multiprecision::cpp_int;

std::string bytes_to_hex_string(std::string_view);
std::string hex_string_to_bytes(std::string_view);

using Hash = std::array<uint8_t, 32>;

// unsigned 32 byte hex literal
inline Hash operator"" _x32(const char* in, std::size_t n) {
  if (n != 64) {
    throw std::invalid_argument("expected 64 hex characters");
  }

  std::string out = hex_string_to_bytes({in, n});
  Hash array;
  std::copy_n(out.begin(), 32, array.begin());
  return array;
}

using Address = std::array<uint8_t, 20>;

template <std::size_t N>
std::string_view byte_view(const std::array<uint8_t, N>& a) {
  return std::string_view(reinterpret_cast<const char*>(a.data()), N);
}

using Nibble = char;

static constexpr auto kSzabo = 1'000'000'000'000ull;
static constexpr auto kFinney = kSzabo * 1000;
static constexpr auto kEther = kFinney * 1000;

}  // namespace silkworm

#endif  // SILKWORM_CORE_COMMON_HPP_
