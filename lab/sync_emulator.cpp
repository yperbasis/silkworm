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

#include "dust_generator.hpp"
#include "keccak.hpp"
#include "lmdb_bucket.hpp"
#include "miner.hpp"

using namespace silkworm;

static const auto kInitialAccounts = 10'000'000;
static const auto kNewAccountsPerBlock = 300;
static const auto kBlockTime = 15;             // sec
static const auto kBandwidth = 1'000'000 / 8;  // bytes per sec

void print_hints(const sync::Hints& hints) {
  static constexpr double kKibibyte = 1024;
  static constexpr double kMebibyte = 1024 * 1024;

  std::cout << "Hints for " << hints.num_leaves * 1e-6
            << "M dust accounts with reply size <~ "
            << hints.approx_max_reply_size / kKibibyte << " KiB:\n";
  const auto mem_depth = hints.depth_to_fit_in_memory();
  std::cout << "depth to fit in memory " << static_cast<int>(mem_depth) << " ("
            << std::setprecision(3)
            << hints.tree_size_in_bytes(mem_depth) / kMebibyte << " MiB)\n";
  const auto d1 = hints.optimal_phase1_depth();
  std::cout << "optimal phase 1 depth  " << static_cast<int>(d1) << std::endl;
  const auto d2 = hints.optimal_phase2_depth();
  std::cout << "optimal phase 2 depth  " << static_cast<int>(d2) << std::endl;
  std::cout << "overhead (∞ bandwidth) " << std::setprecision(2)
            << hints.inf_bandwidth_reply_overhead() * 100 << "%\n";
  std::cout << "RQS                    " << std::setprecision(2)
            << hints.rqs(d2) / kMebibyte << " MiB \n\n";
}

int main() {
  using namespace silkworm::lab;
  using namespace boost::posix_time;

  static const auto kStartBlock = 7212230u;
  static const auto kSeed = 3548264823u;

  sync::Hints hints;
  hints.changes_per_block = kNewAccountsPerBlock;
  print_hints(hints);

  hints.num_leaves = kInitialAccounts;
  print_hints(hints);

  const auto time0 = microsec_clock::local_time();
  LmdbBucket miner_state("miner_state");
  RNG rng(kSeed);
  DustGenerator dust_gen(rng);

  // create random dust accounts
  static constexpr auto kBatchSize = 5'000'000;
  for (int i = 0; i < kInitialAccounts / kBatchSize + 1; ++i) {
    int j = 0;

    miner_state.put([&dust_gen, i, &j]() -> std::optional<DbBucket::KeyVal> {
      auto account_num = i * kBatchSize + j;
      if (account_num >= kInitialAccounts || j >= kBatchSize) {
        return {};
      }
      ++j;
      if (account_num % 1'000'000 == 0) {
        std::cout << account_num / 1'000'000 << "M accounts generated\n";
      }

      Hash key = keccak(byte_view(dust_gen.random_address()));
      std::string val = to_rlp(dust_gen.random_account());
      return std::pair{byte_view(key), val};
    });
  }

  Miner miner(miner_state, hints, kStartBlock);
  const auto time1 = microsec_clock::local_time();
  std::cout << "Dust accounts generated in " << time1 - time0 << "\n\n";

  LmdbBucket leecher_state("leecher_state");
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
  std::cout << "reply total nodes   " << stats.reply_total_nodes << std::endl;
  std::cout << "generated leaves    " << generated_leaves << std::endl;

  const auto leaf_bytes =
      static_cast<double>(generated_leaves * sync::kLeafSize);
  const auto overhead = stats.reply_total_bytes / leaf_bytes - 1;
  std::cout << "reply overhead      " << std::setprecision(2) << overhead * 100
            << "%\n\n";

  const bool same = miner_state.has_same_data(leecher_state);

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
