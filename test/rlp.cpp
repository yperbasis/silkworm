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

#include "rlp.hpp"

// This tells Catch to provide a main() – only do this in one cpp file
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// https://github.com/ethereum/wiki/wiki/RLP
TEST_CASE("Recursive Length Prefix", "[rlp]") {
  using namespace silkworm::rlp;

  SECTION("encode examples") {
    REQUIRE(encode("dog") == "\x83""dog");
    // REQUIRE(encode(List{"cat", "dog"}) == "\xc8\x83""cat""\x83""dog");
    // TODO: other examples
  }

  // TODO: decode tests
}
