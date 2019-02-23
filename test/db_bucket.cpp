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

#include "db_bucket.hpp"

#include <catch2/catch.hpp>

using namespace silkworm;

TEST_CASE("Database leaves by prefix") {
  Hash key1 =
      "15d2460186f7233c927e7002dcc703c0e500b653ca3227363333aa089d1745ec"_x32;
  Hash key2 =
      "15d2460186f7233c927e7002dcc703c0e500b653ca32273b7bfad8045d85a470"_x32;
  Hash key3 =
      "15d2470000000000000000000000000000000000000000000000000000000000"_x32;
  Hash key4 =
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff475"_x32;

  DbBucket db;
  db.put(key2, "foo");
  db.put(key1, "val");
  db.put(key4, "bar");
  db.put(key3, "kittie");

  const Prefix null_prefix(0);
  const auto r0 = db.leaves(null_prefix);
  REQUIRE(r0.first != r0.second);

  const auto r1 = db.leaves("0ffdf"_prefix);
  REQUIRE(r1.first == r1.second);

  const auto r2 = db.leaves("15d246"_prefix);
  auto it = r2.first;
  REQUIRE(it != r2.second);
  REQUIRE(it->first == key1);
  REQUIRE(it->second == "val");
  ++it;
  REQUIRE(it->first == key2);
  ++it;
  REQUIRE(it == r2.second);

  const auto r3 = db.leaves("15d247"_prefix);
  REQUIRE(r3.first != r3.second);

  const auto r4 = db.leaves("fffff"_prefix);
  it = r4.first;
  REQUIRE(it != r4.second);
  REQUIRE(it->first == key4);
  ++it;
  REQUIRE(it == r4.second);

  db.erase("15d24"_prefix);
  const auto r5 = db.leaves("15d246"_prefix);
  REQUIRE(r5.first == r5.second);
  const auto r6 = db.leaves("15d247"_prefix);
  REQUIRE(r6.first == r6.second);
  const auto r7 = db.leaves("fffff"_prefix);
  REQUIRE(r7.first != r7.second);
}
