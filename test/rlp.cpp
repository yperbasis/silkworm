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
  using namespace std::string_literals;

  SECTION("encode examples") {
    // The string "dog" = [ 0x83, 'd', 'o', 'g' ]
    REQUIRE(encode("dog") ==
            "\x83"
            "dog");

    // The list [ "cat", "dog" ] =
    // [ 0xc8, 0x83, 'c', 'a', 't', 0x83, 'd', 'o', 'g' ]
    REQUIRE(encode(List{"cat", "dog"}) ==
            "\xc8\x83"
            "cat"
            "\x83"
            "dog");

    // The empty string ('null') = [ 0x80 ]
    REQUIRE(encode("") == "\x80");

    // The empty list = [ 0xc0 ]
    REQUIRE(encode(List{}) == "\xc0");

    // The integer 0 = [ 0x80 ]
    REQUIRE(encode(to_binary(0)) == "\x80");

    // The encoded integer 0 ('\x00') = [ 0x00 ]
    REQUIRE(encode("\x00"s) == "\x00"s);

    // The encoded integer 15 ('\x0f') = [ 0x0f ]
    REQUIRE(encode("\x0f") == "\x0f");

    // The encoded integer 1024 ('\x04\x00') = [ 0x82, 0x04, 0x00 ]
    REQUIRE(encode("\x04\x00"s) == "\x82\x04\x00"s);

    // List{List{}} would trigger the copy constructor
    List listOfOne;
    listOfOne.push_back(List{});

    // The set theoretical representation of three, [ [], [[]], [ [], [[]] ] ] =
    // [ 0xc7, 0xc0, 0xc1, 0xc0, 0xc3, 0xc0, 0xc1, 0xc0 ]
    REQUIRE(encode(List{List{}, listOfOne, List{List{}, listOfOne}}) ==
            "\xc7\xc0\xc1\xc0\xc3\xc0\xc1\xc0");

    // The string "Lorem ipsum dolor sit amet, consectetur adipisicing elit" =
    // [ 0xb8, 0x38, 'L', 'o', 'r', 'e', 'm', ' ', ... , 'e', 'l', 'i', 't' ]
    REQUIRE(
        encode("Lorem ipsum dolor sit amet, consectetur adipisicing elit") ==
        "\xb8\x38"
        "Lorem ipsum dolor sit amet, consectetur adipisicing elit");

    // a single byte outside of the [0x00, 0x7f] range
    REQUIRE(encode(to_binary(0x90)) == "\x81\x90");
  }

  // TODO: decode tests, incl bad inputs
}
