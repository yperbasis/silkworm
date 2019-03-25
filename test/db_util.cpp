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

#include "db_util.hpp"

#include <catch2/catch.hpp>

#include "lmdb_bucket.hpp"
#include "memdb_bucket.hpp"
#include "mptrie.hpp"

using namespace silkworm;
using namespace silkworm::db_util;

TEMPLATE_TEST_CASE("Database leaves by prefix", "[db_util]", MemDbBucket,
                   LmdbBucket) {
  Hash key1 =
      "15d2460186f7233c927e7002dcc703c0e500b653ca3227363333aa089d1745ec"_x32;
  Hash key2 =
      "15d2460186f7233c927e7002dcc703c0e500b653ca32273b7bfad8045d85a470"_x32;
  Hash key3 =
      "15d2470000000000000000000000000000000000000000000000000000000000"_x32;
  Hash key4 =
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff475"_x32;

  TestType db("prefix_test");
  db.put(byte_view(key2), "foo");
  db.put(byte_view(key1), "val");
  db.put(byte_view(key4), "bar");
  db.put(byte_view(key3), "kittie");

  std::vector<std::string> keys;
  auto key_accumulator = [&keys](std::string_view key, std::string_view) {
    keys.emplace_back(key);
  };

  const Prefix null_prefix(0);
  iterate(db, null_prefix, key_accumulator);
  REQUIRE(keys.size() == 4);

  keys.clear();
  iterate(db, "0ffdf"_prefix, key_accumulator);
  REQUIRE(keys.empty());

  keys.clear();
  iterate(db, "15d246"_prefix, key_accumulator);
  REQUIRE(keys.size() == 2);
  REQUIRE(keys[0] == byte_view(key1));
  REQUIRE(keys[1] == byte_view(key2));

  keys.clear();
  iterate(db, "15d247"_prefix, key_accumulator);
  REQUIRE(keys.size() == 1);

  keys.clear();
  iterate(db, "fffff"_prefix, key_accumulator);
  REQUIRE(keys.size() == 1);
  REQUIRE(keys[0] == byte_view(key4));

  del(db, "15d24"_prefix);

  keys.clear();
  iterate(db, "15d246"_prefix, key_accumulator);
  REQUIRE(keys.empty());

  keys.clear();
  iterate(db, "15d247"_prefix, key_accumulator);
  REQUIRE(keys.empty());

  keys.clear();
  iterate(db, "fffff"_prefix, key_accumulator);
  REQUIRE(keys.size() == 1);
}
