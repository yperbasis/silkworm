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

#include <catch2/catch.hpp>

// https://github.com/ethereum/wiki/wiki/RLP
TEST_CASE("Recursive Length Prefix", "[rlp]") {
  using namespace silkworm::rlp;

  SECTION("encode examples") {
    REQUIRE(encode("dog") ==
            "\x83"
            "dog");

    REQUIRE(encode(List{"cat", "dog"}) ==
            "\xc8\x83"
            "cat"
            "\x83"
            "dog");

    REQUIRE(encode("") == "\x80");
    REQUIRE(encode(List{}) == "\xc0");

    // TODO: big endian integers, incl. 0

    REQUIRE(encode("\x00") == "\x00");
    REQUIRE(encode("\x0f") == "\x0f");
    REQUIRE(encode("\x04\x00") == "\x82\x04\x00");

    REQUIRE(encode(List{List{}, List{List{}}, List{List{}, List{List{}}}}) ==
            "\xc7\xc0\xc1\xc0\xc3\xc0\xc1\xc0");

    REQUIRE(
        encode("Lorem ipsum dolor sit amet, consectetur adipisicing elit") ==
        "\xb8\x38"
        "Lorem ipsum dolor sit amet, consectetur adipisicing elit");
  }

  // TODO: decode tests
}
