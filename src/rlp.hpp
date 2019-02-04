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

#ifndef SILKWORM_RLP_HPP_
#define SILKWORM_RLP_HPP_

#include <string>
#include <utility>
#include <variant>
#include <vector>

// https://github.com/ethereum/wiki/wiki/RLP
namespace silkworm::rlp {
namespace impl {
struct ItemWrapper;

using List = std::vector<ItemWrapper>;
using Item = std::variant<std::string, List>;

struct ItemWrapper {
  Item _data;

  template <typename... T>
  ItemWrapper(T&&... x) : _data{std::forward<T>(x)...} {}
};
}  // namespace impl

using impl::Item, impl::List;

std::string encode(const Item&);
Item decode(const std::string&);
}  // namespace silkworm::rlp

#endif  // SILKWORM_RLP_HPP_
