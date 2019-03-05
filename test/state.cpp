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

#include "state.hpp"

#include <catch2/catch.hpp>

using namespace silkworm;

TEST_CASE("GetNode Request", "[sync]") {
  const auto depth = 5u;
  const auto phase1_depth = 3u;
  const auto block = 74;

  DbBucket db;
  db.put("27407374bb099f172303644baef2dcc703c0e500b653ca82273b7b045d85a470"_x32,
         "crypto kitties");
  db.put("274cc374bb09f9172122dcc70c03036123e0e178b654cd82273b7b045d85a499"_x32,
         "teh DAO");

  State state(db, depth, phase1_depth);
  state.init_from_db(block);

  const uint64_t req_id = 6643894947723747ull;

  const sync::GetNodeRequest node_request{
      req_id,
      {},
      {"000"_prefix, "0fd7"_prefix, "274"_prefix, "ffff"_prefix, "274c"_prefix},
      block};

  const auto node_reply = state.get_nodes(node_request);
  REQUIRE(node_reply->req_id == req_id);
  REQUIRE(node_reply->block_number == block);
  REQUIRE(node_reply->nodes.size() == 5);

  for (size_t i = 0; i < 5; ++i) {
    REQUIRE(node_reply->nodes[0]);
  }

  REQUIRE(node_reply->nodes[0]->empty.all());
  REQUIRE(node_reply->nodes[1]->empty.all());
  REQUIRE(node_reply->nodes[2]->empty ==
          std::bitset<16>{0b1110'1111'1111'1110});
  //               nibbles  fedc'ba98'7654'3210
  REQUIRE(node_reply->nodes[3]->empty.all());
  REQUIRE(node_reply->nodes[4]->empty ==
          std::bitset<16>{0b1110'1111'1111'1111});
}

TEST_CASE("Phase 1 sync", "[sync]") {
  const auto depth = 3u;
  const auto phase1_depth = 2u;

  const auto block = 74;

  DbBucket leecher_db;
  State leecher(leecher_db, depth, phase1_depth);

  SECTION("frozen block") {
    auto request = leecher.next_sync_request();
    REQUIRE(request);
    REQUIRE(!request->block_number);
    REQUIRE(!request->hash_of_leaves);

    const auto no_data = leecher.get_leaves(*request);
    REQUIRE(std::holds_alternative<sync::Error>(no_data));
    REQUIRE(std::get<sync::Error>(no_data) == sync::kDontHaveData);

    Hash hash = request->prefix.padded();
    hash[23] = '\xf7';
    hash[29] = '\x19';
    hash[30] = '\x70';

    DbBucket seeder_db;
    seeder_db.put(hash, "crypto kitties");

    hash[29] = '\x0b';
    seeder_db.put(hash, "teh DAO");

    State seeder(seeder_db, depth, phase1_depth);
    seeder.init_from_db(block);

    const auto reply_wrapper = seeder.get_leaves(*request);
    REQUIRE(std::holds_alternative<sync::LeavesReply>(reply_wrapper));
    const auto reply = std::get<sync::LeavesReply>(reply_wrapper);
    REQUIRE(reply.prefix == request->prefix);
    REQUIRE(reply.block_number == block);
    REQUIRE(reply.leaves);

    const auto& leaves = *reply.leaves;
    REQUIRE(leaves.size() == 2);

    // check the leaves are sorted
    REQUIRE(leaves[0].first[29] == '\x0b');
    REQUIRE(leaves[0].second == "teh DAO");
    REQUIRE(leaves[1].first[29] == '\x19');
    REQUIRE(leaves[1].second == "crypto kitties");

    leecher.process_sync_data(reply);
    // leecher turning into seeder
    const auto new_reply_wrapper = leecher.get_leaves(*request);
    REQUIRE(std::holds_alternative<sync::LeavesReply>(new_reply_wrapper));
    const auto new_reply = std::get<sync::LeavesReply>(new_reply_wrapper);
    REQUIRE(new_reply.prefix == request->prefix);
    REQUIRE(new_reply.block_number == block);
    REQUIRE(new_reply.leaves);
    REQUIRE(*new_reply.leaves == leaves);
  }

  // TODO test phase 2 sync
}
