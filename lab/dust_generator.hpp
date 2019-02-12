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

#ifndef SILKWORM_LAB_DUST_GENERATOR_HPP_
#define SILKWORM_LAB_DUST_GENERATOR_HPP_

#include <random>

#include "account.hpp"
#include "db_bucket.hpp"

namespace silkworm::lab {

using RNG = std::minstd_rand;

class DustGenerator {
 public:
  explicit DustGenerator(RNG& rng) : rng_(rng) {}
  void generate(uint64_t num_accounts);

 private:
  DbBucket state_db_;
  RNG& rng_;
};

}  // namespace silkworm::lab

#endif  // SILKWORM_LAB_DUST_GENERATOR_HPP_
