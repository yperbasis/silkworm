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

#include "prefix.hpp"

#include <catch2/catch.hpp>

using namespace silkworm;

TEST_CASE("Even prefix") {
  const auto matching_hash =
      "0fd714a130000000000000000000000000000006770000000000000000000004"_x32;

  auto prefix = "0fd714"_prefix;
  REQUIRE(prefix.size() == 6);
  REQUIRE(
      prefix.padded() ==
      "0fd7140000000000000000000000000000000000000000000000000000000000"_x32);

  REQUIRE(prefix.matches(matching_hash));
  REQUIRE(Prefix(6, matching_hash) == prefix);

  REQUIRE(!prefix.matches(
      "0fd715a130000000000000000000000000000006770000000000000000000004"_x32));

  REQUIRE(prefix[0] == 0x0);
  REQUIRE(prefix[1] == 0xf);
  REQUIRE(prefix[2] == 0xd);
  REQUIRE(prefix[3] == 0x7);
  REQUIRE(prefix[4] == 0x1);
  REQUIRE(prefix[5] == 0x4);
  REQUIRE(prefix.last() == 0x4);

  prefix.set(1, 0xa);
  REQUIRE(prefix == "0ad714"_prefix);
  prefix.set(4, 0xe);
  REQUIRE(prefix == "0ad7e4"_prefix);

  REQUIRE((prefix + 1) == "0ad7e5"_prefix);
  REQUIRE(++prefix == "0ad7e5"_prefix);
  REQUIRE(++"0ad7ff"_prefix == "0ad800"_prefix);
  REQUIRE(++"ffffff"_prefix == "000000"_prefix);
}

TEST_CASE("Odd prefix") {
  const auto matching_hash =
      "0fd715a130000000000000000000000000000006770000000000000000000004"_x32;

  auto prefix = "0fd71"_prefix;
  REQUIRE(prefix.size() == 5);
  REQUIRE(
      prefix.padded() ==
      "0fd7100000000000000000000000000000000000000000000000000000000000"_x32);

  REQUIRE(prefix.matches(matching_hash));
  REQUIRE(Prefix(5, matching_hash) == prefix);

  REQUIRE(prefix != "0ad710"_prefix);

  REQUIRE(prefix[0] == 0x0);
  REQUIRE(prefix[1] == 0xf);
  REQUIRE(prefix[2] == 0xd);
  REQUIRE(prefix[3] == 0x7);
  REQUIRE(prefix[4] == 0x1);
  REQUIRE(prefix.last() == 0x1);

  prefix.set(1, 0xa);
  REQUIRE(prefix == "0ad71"_prefix);
  prefix.set(4, 0xe);
  REQUIRE(prefix == "0ad7e"_prefix);

  REQUIRE((prefix + 1) == "0ad7f"_prefix);
  REQUIRE(++prefix == "0ad7f"_prefix);
  REQUIRE(++prefix == "0ad80"_prefix);

  prefix += 0xa7;
  REQUIRE(prefix == "0ae27"_prefix);

  REQUIRE(++"fffff"_prefix == "00000"_prefix);
}
