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

#include "common.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Hex string") {
  using namespace silkworm;
  using namespace std::string_literals;

  REQUIRE(bytes_to_hex_string("\x09\xf1\x00\xa0"s) == "09f100a0");
  REQUIRE(hex_string_to_bytes("09f100a0") == "\x09\xf1\x00\xa0"s);
}
