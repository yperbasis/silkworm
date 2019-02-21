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

TEST_CASE("State sync") {
  const auto depth = 3u;
  const auto phase1_depth = 2u;

  const auto block = 74;

  DbBucket leecher_db;
  State leecher(leecher_db, depth, phase1_depth);

  SECTION("frozen block") {
    auto request = leecher.next_sync_request();
    REQUIRE(request);
    REQUIRE(!request->block);
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
    REQUIRE(std::holds_alternative<sync::Reply>(reply_wrapper));
    const auto reply = std::get<sync::Reply>(reply_wrapper);
    REQUIRE(reply.prefix == request->prefix);
    REQUIRE(reply.block == block);
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
    REQUIRE(std::holds_alternative<sync::Reply>(new_reply_wrapper));
    const auto new_reply = std::get<sync::Reply>(new_reply_wrapper);
    REQUIRE(new_reply.prefix == request->prefix);
    REQUIRE(new_reply.block == block);
    REQUIRE(new_reply.leaves);
    REQUIRE(*new_reply.leaves == leaves);
  }
}
