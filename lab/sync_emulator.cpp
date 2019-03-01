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

using namespace silkworm;

static const auto kInitialAccounts = 10'000'000;
static const auto kNewAccountsPerBlock = 300;
static const auto kBlockTime = 15;                   // sec
static const auto kBandwidth = 2 * 1024 * 1024 / 8;  // bytes per sec

void print_hints(const sync::Hints& hints) {
  std::cout << "Hints for " << hints.num_leaves * 1e-6
            << "M dust accounts with reply size <~ "
            << hints.approx_max_reply_size / 1024 << "KB:\n";
  const auto mem_depth = hints.depth_to_fit_in_memory();
  std::cout << "depth to fit in memory " << mem_depth << " ("
            << std::setprecision(3)
            << hints.tree_size_in_bytes(mem_depth) * 1e-6 << " MB)\n";
  std::cout << "optimal phase 1 depth  " << hints.optimal_phase1_depth()
            << std::endl;
  std::cout << "optimal phase 2 depth  " << hints.optimal_phase2_depth()
            << std::endl;
  std::cout << "overhead (∞ bandwidth) " << std::setprecision(2)
            << hints.inf_bandwidth_reply_overhead() * 100 << "%\n\n";
}

int main() {
  using namespace silkworm::lab;
  using namespace boost::posix_time;

  static const auto kStartBlock = 7212230;
  static const unsigned kSeed = 3548264823;

  sync::Hints hints;
  print_hints(hints);

  hints.num_leaves = kInitialAccounts;
  print_hints(hints);

  const auto time0 = microsec_clock::local_time();
  DbBucket miner_state;
  RNG rng(kSeed);
  DustGenerator dust_gen(rng);
  for (int i = 0; i < kInitialAccounts; ++i) {
    Hash key = keccak(byte_view(dust_gen.random_address()));
    std::string val = to_rlp(dust_gen.random_account());
    miner_state.put(key, val);
  }
  Miner miner(miner_state, hints, kStartBlock);
  const auto time1 = microsec_clock::local_time();
  std::cout << "Dust accounts generated in " << time1 - time0 << "\n\n";

  DbBucket leecher_state;
  Node leecher(leecher_state, hints, {});
  sync::Stats stats;
  auto new_blocks = 0;
  auto generated_leaves = kInitialAccounts;

  while (true) {
    std::cout << "new block " << new_blocks << " phase "
              << (leecher.phase1_sync_done() + 1) << std::endl;

    leecher.sync(miner, stats, kBandwidth * kBlockTime);

    std::cout << "leaves received " << stats.reply_total_leaves
              << " vs generated " << generated_leaves << std::endl;

    if (leecher.sync_done()) {
      break;
    }

    miner.new_block();
    for (int i = 0; i < kNewAccountsPerBlock; ++i) {
      miner.create_account(dust_gen.random_address(),
                           dust_gen.random_account());
    }
    miner.seal_block();

    generated_leaves += kNewAccountsPerBlock;
    ++new_blocks;
  }

  std::cout << "\nSync done? " << std::boolalpha << leecher.sync_done()
            << "\n\n";

  const auto time2 = microsec_clock::local_time();
  const auto emulated_time = seconds((new_blocks + 1) * kBlockTime);

  std::cout << "CPU time            " << time2 - time1 << std::endl;
  std::cout << "Emulated time       " << emulated_time << std::endl;
  std::cout << "#new blocks         " << new_blocks << std::endl;
  std::cout << "#requests           " << stats.num_requests << std::endl;
  std::cout << "request total bytes " << stats.request_total_bytes << std::endl;
  std::cout << "#replies            " << stats.num_replies << std::endl;
  std::cout << "reply total bytes   " << stats.reply_total_bytes << std::endl;
  std::cout << "reply total leaves  " << stats.reply_total_leaves << std::endl;
  std::cout << "generated leaves    " << generated_leaves << std::endl;

  double leaf_bytes = generated_leaves * sync::kLeafSize;
  double overhead = stats.reply_total_bytes / leaf_bytes - 1;
  std::cout << "reply overhead      " << std::setprecision(2) << overhead * 100
            << "%\n\n";

  const Prefix null_prefix(0);
  const auto miner_leaves = miner_state.leaves(null_prefix);
  const auto leecher_leaves = leecher_state.leaves(null_prefix);
  const bool same = std::equal(miner_leaves.first, miner_leaves.second,
                               leecher_leaves.first, leecher_leaves.second);
  if (same) {
    std::cout << "Sync verified 😅\n";
  } else {
    std::cout << "Epic Fail 🤬\n";
  }

  // TODO multiple leechers
  // a) spawn leechers in separate threads
  // b) keep mining new blocks
  // c) wait for leechers to finish sync
}
