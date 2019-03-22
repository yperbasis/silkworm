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

  MemDbBucket db;
  db.put(
      byte_view(
          "27407374bb099f172303644baef2dcc703c0e500b653ca82273b7b045d85a470"_x32),
      "crypto kitties");
  db.put(
      byte_view(
          "274cc374bb09f9172122dcc70c03036123e0e178b654cd82273b7b045d85a499"_x32),
      "teh DAO");

  State state(db, depth, phase1_depth);
  state.init_from_db(block);

  const sync::GetNodeRequest node_request{{},
                                          {
                                              "000"_prefix,
                                              "0fd7"_prefix,
                                              "274"_prefix,
                                              "ffff"_prefix,
                                              "274c"_prefix,
                                              ""_prefix,
                                          },
                                          block};

  const auto node_reply = state.get_nodes(node_request);
  REQUIRE(node_reply->block_number == block);
  REQUIRE(node_reply->nodes.size() == 6);

  for (const auto& node : node_reply->nodes) {
    REQUIRE(node);
  }

  REQUIRE(node_reply->nodes[0]->empty.all());
  REQUIRE(node_reply->nodes[1]->empty.all());
  REQUIRE(node_reply->nodes[2]->empty ==
          std::bitset<16>{0b1110'1111'1111'1110});
  //               nibbles  fedc'ba98'7654'3210
  REQUIRE(node_reply->nodes[3]->empty.all());
  REQUIRE(node_reply->nodes[4]->empty ==
          std::bitset<16>{0b1110'1111'1111'1111});
  REQUIRE(node_reply->nodes[5]->empty ==
          std::bitset<16>{0b1111'1111'1111'1011});
}

TEST_CASE("Phase 1 sync", "[sync]") {
  const auto depth = 3u;
  const auto phase1_depth = 2u;

  const auto block = 74;

  MemDbBucket leecher_db;
  State leecher(leecher_db, depth, phase1_depth);

  SECTION("frozen block") {
    auto request_variant = leecher.next_sync_request();
    auto request = std::get_if<sync::GetLeavesRequest>(&request_variant);
    REQUIRE(request);
    REQUIRE(!request->block_number);

    const auto no_data = leecher.get_leaves(*request);
    REQUIRE(no_data.status == sync::LeavesReply::kDontHaveData);

    const auto prefix = request->prefix;
    Hash hash = prefix.padded();
    hash[23] = 0xf7;
    hash[29] = 0x19;
    hash[30] = 0x70;

    MemDbBucket seeder_db;
    seeder_db.put(byte_view(hash), "crypto kitties");

    hash[29] = 0x0b;
    seeder_db.put(byte_view(hash), "teh DAO");

    State seeder(seeder_db, depth, phase1_depth);
    seeder.init_from_db(block);

    const auto reply = seeder.get_leaves(*request);
    REQUIRE(reply.status == sync::LeavesReply::kOK);
    REQUIRE(reply.block_number == block);
    REQUIRE(reply.leaves);

    const auto& leaves = *reply.leaves;
    REQUIRE(leaves.size() == 2);

    // check the leaves are sorted
    REQUIRE(leaves[0].first[29] == '\x0b');
    REQUIRE(leaves[0].second == "teh DAO");
    REQUIRE(leaves[1].first[29] == '\x19');
    REQUIRE(leaves[1].second == "crypto kitties");

    leecher.process_leaves_reply(prefix, reply);
    // leecher turning into seeder
    const auto new_reply = leecher.get_leaves(*request);
    REQUIRE(new_reply.status == sync::LeavesReply::kOK);
    REQUIRE(new_reply.block_number == block);
    REQUIRE(new_reply.leaves);
    REQUIRE(*new_reply.leaves == leaves);
  }

  // TODO test phase 2 sync
}
