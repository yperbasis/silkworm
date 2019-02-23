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

using namespace silkworm;

TEST_CASE("Block creation", "[miner]") {
  const auto block = 46732;

  DbBucket db;
  sync::Hints perf_hints;
  perf_hints.max_memory = 8 * 1024 * 1024;
  Miner miner(db, perf_hints, block);

  const Address address = "0a234567901c345679012345679001774567fbb3"_x20;
  Account account;
  account.balance = 54823904;

  const auto key = keccak(byte_view(address));

  const auto prefix = Prefix(2, static_cast<uint64_t>(key[0]) << (7 * 8));
  const auto sync_request = sync::Request{prefix};

  // empty state
  auto reply_wrapper = miner.get_state_leaves(sync_request);
  REQUIRE(std::holds_alternative<sync::Reply>(reply_wrapper));
  auto reply = std::get<sync::Reply>(reply_wrapper);
  REQUIRE(reply.prefix == prefix);
  REQUIRE(reply.block == block);
  REQUIRE(reply.leaves);
  REQUIRE(reply.leaves->empty());

  // create account and new block
  REQUIRE(miner.new_block());
  miner.create_account(address, account);
  REQUIRE(miner.seal_block());

  // state with the account
  reply_wrapper = miner.get_state_leaves(sync_request);
  REQUIRE(std::holds_alternative<sync::Reply>(reply_wrapper));
  reply = std::get<sync::Reply>(reply_wrapper);
  REQUIRE(reply.prefix == prefix);
  REQUIRE(reply.block == block + 1);
  REQUIRE(reply.leaves);
  REQUIRE(reply.leaves->size() == 1);

  const auto leaf = (*reply.leaves)[0];
  REQUIRE(leaf.first == key);
  REQUIRE(leaf.second == to_rlp(account));
}
