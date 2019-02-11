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

  SECTION("examples") {
    REQUIRE(keccak("") == kEmptyStringHash);

    REQUIRE(  // https://github.com/ethereum/wiki/wiki/JSON-RPC#web3_sha3
        keccak(hex_string_to_bytes("68656c6c6f20776f726c64")) ==
        "47173285a8d7341e5e972fc677286384f802f8ef42a5ec5f03bbfa254cb01fad"_x32);
  }
}
