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

#ifndef SILKWORM_CORE_RLP_HPP_
#define SILKWORM_CORE_RLP_HPP_

#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

#include <boost/variant.hpp>

#include "common.hpp"

// https://github.com/ethereum/wiki/wiki/RLP
namespace silkworm::rlp {

using Item =
    boost::make_recursive_variant<std::string,
                                  std::vector<boost::recursive_variant_>>::type;
using List = std::vector<Item>;

// operator== doesn't compile with Xcode 10 for some reason
// TODO: submit a bugreport to boost
bool are_equal(const Item&, const Item&);

std::string encode(const Item&);
Item decode(std::string_view);

std::string to_big_endian(uint64_t);
std::string to_big_endian(UInt256);

// TODO UInt256
uint64_t from_big_endian(std::string_view b);
}  // namespace silkworm::rlp

#endif  // SILKWORM_CORE_RLP_HPP_
