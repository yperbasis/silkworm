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

#include "keccak.hpp"

#include <string>

#include <catch2/catch.hpp>

#include "util.hpp"

TEST_CASE("Keccak") {
  using namespace silkworm;

  // https://github.com/ethereum/wiki/wiki/JSON-RPC#web3_sha3
  SECTION("example") {
    std::string input = "68656c6c6f20776f726c64";
    std::string expected_hex =
        "47173285a8d7341e5e972fc677286384f802f8ef42a5ec5f03bbfa254cb01fad";

    std::array<uint8_t, 32> actual_bytes = keccak(hex_string_to_bytes(input));

    std::string actual_hex = bytes_to_hex_string(std::string_view(
        reinterpret_cast<const char*>(actual_bytes.data()), 32));

    REQUIRE(actual_hex == expected_hex);
  }
}
