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

#include "lmdb_bucket.hpp"
#include "memdb_bucket.hpp"

#include <string>
#include <vector>

#include <catch2/catch.hpp>

using namespace silkworm;

TEMPLATE_TEST_CASE("put/get", "[db]", MemDbBucket, LmdbBucket) {
  using namespace std::string_literals;

  TestType db("test1");

  // both key & val may contain 0 chars
  const auto key = "AB\0BA"s;
  const auto val = "fghjar(#\0\0]0oo"s;

  REQUIRE(!db.get(key));

  db.put(key, val);

  const auto res = db.get(key);
  REQUIRE(res);
  REQUIRE(*res == val);
}

TEMPLATE_TEST_CASE("has_same_data", "[db]", MemDbBucket, LmdbBucket) {
  TestType db1("db1");
  TestType db2("db2");

  REQUIRE(db1.has_same_data(db2));
  REQUIRE(db2.has_same_data(db1));

  std::vector<std::pair<std::string, std::string>> data = {
      {"dem", "_RER78"}, {"dehrrer", "d532742u"},     {"abba", "ffdEEo)"},
      {"dezzz", "qqqq"}, {"e66434424z", "reyYYEIk_"}, {"dv", "-6437"},
  };

  for (const auto& x : data) {
    db1.put(x.first, x.second);
  }

  REQUIRE(!db1.has_same_data(db2));
  REQUIRE(!db2.has_same_data(db1));

  for (const auto& x : data) {
    db2.put(x.first, x.second);
  }

  REQUIRE(db2.has_same_data(db1));
  REQUIRE(db1.has_same_data(db2));

  db2.put("dezzz", "43255235");
  REQUIRE(!db2.has_same_data(db1));
  REQUIRE(!db1.has_same_data(db2));

  db2.put("dezzz", "qqqq");
  REQUIRE(db2.has_same_data(db1));
  REQUIRE(db1.has_same_data(db2));

  db1.put("zz78", "_fdsg");
  REQUIRE(!db2.has_same_data(db1));
  REQUIRE(!db1.has_same_data(db2));
}

TEMPLATE_TEST_CASE("ranges", "[db]", MemDbBucket, LmdbBucket) {
  TestType db("test2");

  std::vector<std::pair<std::string, std::string>> data = {
      {"dem", "_RER78"}, {"dehrrer", "d532742u"},     {"abba", "ffdEEo)"},
      {"dezzz", "qqqq"}, {"e66434424z", "reyYYEIk_"}, {"dv", "-6437"},
  };

  for (const auto& x : data) {
    db.put(x.first, x.second);
  }

  std::vector<std::string> res;
  const auto key_aggregator = [&res](std::string_view key,
                                     std::string_view /*val*/) {
    res.emplace_back(key);
  };

  // get a range
  db.get("da", "df", key_aggregator);

  REQUIRE(res.size() == 3);
  REQUIRE(res[0] == "dehrrer");
  REQUIRE(res[1] == "dem");
  REQUIRE(res[2] == "dezzz");

  // get all entries
  res.clear();
  db.get("", {}, key_aggregator);
  REQUIRE(res.size() == 6);

  // delete a range
  db.del("da", "df");

  // get all entries
  res.clear();
  db.get("", {}, key_aggregator);

  REQUIRE(res.size() == 3);
  REQUIRE(res[0] == "abba");
  REQUIRE(res[1] == "dv");
  REQUIRE(res[2] == "e66434424z");
}
