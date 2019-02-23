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

#include "common.hpp"

#include <algorithm>
#include <iterator>

#include <boost/algorithm/hex.hpp>

namespace {
template <std::size_t N>
std::array<uint8_t, N> to_byte_array(const char* in, std::size_t n) {
  if (n != N * 2) {
    throw std::invalid_argument("expected " + std::to_string(N * 2) +
                                " nibbles");
  }

  std::string str = silkworm::hex_string_to_bytes({in, n});
  std::array<uint8_t, N> array;
  std::copy_n(str.begin(), N, array.begin());
  return array;
}
}  // namespace

namespace silkworm {

std::string bytes_to_hex_string(std::string_view in) {
  std::string res;
  res.reserve(in.size() * 2);
  boost::algorithm::hex_lower(in.begin(), in.end(), std::back_inserter(res));
  return res;
}

std::string hex_string_to_bytes(std::string_view in) {
  std::string res;
  res.reserve(in.size() / 2);
  boost::algorithm::unhex(in.begin(), in.end(), std::back_inserter(res));
  return res;
}

std::array<uint8_t, 20> operator"" _x20(const char* in, std::size_t n) {
  return to_byte_array<20>(in, n);
}

std::array<uint8_t, 32> operator"" _x32(const char* in, std::size_t n) {
  return to_byte_array<32>(in, n);
}

}  // namespace silkworm
