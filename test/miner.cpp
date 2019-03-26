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

#include "miner.hpp"

#include <catch2/catch.hpp>

#include "memdb_bucket.hpp"

using namespace silkworm;

TEST_CASE("Block creation", "[miner]") {
  const auto block = 46732;

  MemDbBucket db;
  sync::Hints perf_hints;
  perf_hints.max_memory = 8 * 1024 * 1024;
  Miner miner(db, perf_hints, block);

  const Address address = "0a234567901c345679012345679001774567fbb3"_x20;
  Account account;
  account.balance = 54823904;

  const auto key = keccak(byte_view(address));

  const auto prefix = Prefix(2, static_cast<uint64_t>(key[0]) << (7 * 8));
  const auto sync_request = sync::GetLeavesRequest{prefix};

  // empty state
  auto reply = miner.get_state_leaves(sync_request);
  REQUIRE(reply.status == sync::LeavesReply::kOK);
  REQUIRE(reply.block_number == block);
  REQUIRE(reply.leaves);
  REQUIRE(reply.leaves->empty());

  // create account and new block
  miner.new_block();
  miner.create_account(address, account);
  miner.seal_block();

  // state with the account
  reply = miner.get_state_leaves(sync_request);
  REQUIRE(reply.status == sync::LeavesReply::kOK);
  REQUIRE(reply.block_number == block + 1);
  REQUIRE(reply.leaves);
  REQUIRE(reply.leaves->size() == 1);

  const auto leaf = (*reply.leaves)[0];
  REQUIRE(leaf.first == key);
  REQUIRE(leaf.second == to_rlp(account));
}
