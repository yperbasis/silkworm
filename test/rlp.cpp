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

#include <utility>

#include <catch2/catch.hpp>

// https://github.com/ethereum/wiki/wiki/RLP
TEST_CASE("Recursive Length Prefix", "[rlp]") {
  using namespace silkworm::rlp;
  using namespace std::string_literals;

  // List{List{}} would trigger the copy constructor
  List listOfOne;
  listOfOne.push_back(List{});

  const std::vector<std::pair<Item, std::string>> kExamples = {
      // The string "dog" = [ 0x83, 'd', 'o', 'g' ]
      {"dog",
       "\x83"
       "dog"},
      // The list [ "cat", "dog" ] =
      // [ 0xc8, 0x83, 'c', 'a', 't', 0x83, 'd', 'o', 'g' ]
      {List{"cat", "dog"},
       "\xc8\x83"
       "cat"
       "\x83"
       "dog"},
      // The empty string ('null') = [ 0x80 ]
      {"", "\x80"},
      // The empty list = [ 0xc0 ]
      {List{}, "\xc0"},
      // // The integer 0 = [ 0x80 ]
      {to_binary(0), "\x80"},
      // The encoded integer 0 ('\x00') = [ 0x00 ]
      {"\x00"s, "\x00"s},
      // The encoded integer 15 ('\x0f') = [ 0x0f ]
      {"\x0f", "\x0f"},
      // The encoded integer 1024 ('\x04\x00') = [ 0x82, 0x04, 0x00 ]
      {"\x04\x00"s, "\x82\x04\x00"s},
      // The set theoretical representation of three, [ [], [[]], [ [], [[]] ] ]
      // = [ 0xc7, 0xc0, 0xc1, 0xc0, 0xc3, 0xc0, 0xc1, 0xc0 ]
      {List{List{}, listOfOne, List{List{}, listOfOne}},
       "\xc7\xc0\xc1\xc0\xc3\xc0\xc1\xc0"},
      // The string "Lorem ipsum dolor sit amet, consectetur adipisicing elit" =
      // [ 0xb8, 0x38, 'L', 'o', 'r', 'e', 'm', ' ', ... , 'e', 'l', 'i', 't' ]
      {"Lorem ipsum dolor sit amet, consectetur adipisicing elit",
       "\xb8\x38"
       "Lorem ipsum dolor sit amet, consectetur adipisicing elit"},
      // a single byte outside of the [0x00, 0x7f] range
      {to_binary(0x90), "\x81\x90"},
  };

  SECTION("encode examples") {
    for (const auto& x : kExamples) {
      REQUIRE(encode(x.first) == x.second);
    }
  }

  SECTION("decode examples") {
    for (const auto& x : kExamples) {
      INFO("decoding " + x.second);
      REQUIRE(are_equal(decode(x.second), x.first));
    }
  }

  // TODO: bad inputs
}
