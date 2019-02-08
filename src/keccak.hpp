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
#ifndef SILKWORM_KECCAK_HPP_
#define SILKWORM_KECCAK_HPP_

#include <stdint.h>
#include <array>
#include <string_view>

#include "keccak-tiny.h"

namespace silkworm {
std::array<uint8_t, 32> keccak(std::string_view in) {
  std::array<uint8_t, 32> out;
  // https://stackoverflow.com/questions/10151834/why-cant-i-static-cast-between-char-and-unsigned-char
  sha3_256(out.data(), 32, reinterpret_cast<const uint8_t*>(in.data()),
           in.size());
  return out;
}

std::array<uint8_t, 64> keccak512(std::string_view in) {
  std::array<uint8_t, 64> out;
  // https://stackoverflow.com/questions/10151834/why-cant-i-static-cast-between-char-and-unsigned-char
  sha3_512(out.data(), 64, reinterpret_cast<const uint8_t*>(in.data()),
           in.size());
  return out;
}

}  // namespace silkworm

#endif  // SILKWORM_KECCAK_HPP_
