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

#include "dust_generator.hpp"

namespace silkworm::lab {

Address DustGenerator::random_address() {
  static_assert(kAddressBytes % 4 == 0);
  std::uniform_int_distribution<uint32_t> uint32_dist;

  Address address;
  for (size_t j = 0; j < kAddressBytes;) {
    uint32_t word = uint32_dist(rng_);
    for (auto k = 0; k < 4; ++k) {
      address[j++] = word & 0xff;
      word = word >> 8;
    }
  }
  return address;
}

Account DustGenerator::random_account() {
  std::uniform_int_distribution<uint64_t> balance_dist(1, kFinney);

  Account account;
  account.balance = balance_dist(rng_);
  return account;
}
}  // namespace silkworm::lab
