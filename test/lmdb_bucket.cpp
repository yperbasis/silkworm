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

#include <catch2/catch.hpp>

using namespace silkworm;

TEST_CASE("put/get") {
  using namespace std::string_literals;

  LmdbBucket db("test");

  // both key & val may contain 0 chars
  const auto key = "AB\0BA"s;
  const auto val = "fghjar(#\0\0]0oo"s;

  REQUIRE(!db.get(key));

  db.put(key, val);

  const auto res = db.get(key);
  REQUIRE(res);
  REQUIRE(*res == val);
}
