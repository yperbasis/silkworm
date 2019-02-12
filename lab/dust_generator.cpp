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
void DustGenerator::generate(uint64_t num_accounts) {
  RNG& rng = rng_;

  std::uniform_int_distribution<uint64_t> balance_dist(1, kFinney);

  auto rand_address_gen = [&rng]() {
    static_assert(kAddressBytes % 4 == 0);
    std::uniform_int_distribution<uint32_t> uint32_dist;

    Address address;
    for (size_t j = 0; j < kAddressBytes;) {
      uint32_t word = uint32_dist(rng);
      for (auto k = 0; k < 4; ++k) {
        address[j++] = word & 0xff;
        word = word >> 8;
      }
    }
    return address;
  };

  for (uint64_t i = 0; i < num_accounts; ++i) {
    Account account;
    account.balance = balance_dist(rng);
    Address address = rand_address_gen();
    state_db_.put(keccak(byte_view(address)), to_rlp(account));
  }
}
}  // namespace silkworm::lab
