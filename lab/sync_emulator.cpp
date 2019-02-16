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

#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "db_bucket.hpp"
#include "dust_generator.hpp"
#include "keccak.hpp"
#include "miner.hpp"

static const unsigned kSeed = 3548264823;

static const auto kInitialAccounts = 2'000'000;
// static const auto kNewAccountsPerBlock = 1000;
// static const auto kBlocks = 10;
// static const auto kLeechers = 15;

int main() {
  using namespace boost::posix_time;
  using namespace silkworm;
  using namespace silkworm::lab;

  const auto current_block = 7212230;

  auto time0 = microsec_clock::local_time();
  DbBucket big_state;
  RNG rng(kSeed);
  DustGenerator dust_gen(rng);
  for (auto i = 0u; i < kInitialAccounts; ++i) {
    Hash key = keccak(byte_view(dust_gen.random_address()));
    std::string val = to_rlp(dust_gen.random_account());
    big_state.put(key, val);
  }
  Miner miner(big_state, current_block);
  auto time1 = microsec_clock::local_time();
  std::cout << "Dust accounts generated in " << time1 - time0 << std::endl;

  DbBucket leecher_state;
  Node leecher(leecher_state, {});
  auto stats = leecher.sync(miner);
  auto time2 = microsec_clock::local_time();
  std::cout << "Sync done in " << time2 - time1 << std::endl;
  std::cout << "#requests           " << stats.num_requests << std::endl;
  std::cout << "request total bytes " << stats.request_total_bytes << std::endl;
  std::cout << "#replies            " << stats.num_replies << std::endl;
  std::cout << "reply total bytes   " << stats.reply_total_bytes << std::endl;
  std::cout << "reply total leaves  " << stats.reply_total_leaves << std::endl;

  double leaf_bytes = stats.reply_total_leaves * sync::kLeafSize;
  double overhead = stats.reply_total_bytes / leaf_bytes - 1;
  std::cout << "reply overhead " << static_cast<int>(overhead * 100) << "%\n";

  // a) TODO spawn leechers in separate threads
  // b) TODO mine kBlocks with kNewAccountsPerBlock
  // c) TODO wait for leechers to finish sync
}
