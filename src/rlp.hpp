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

#include <stdint.h>
#include <string>
#include <type_traits>
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

  // http://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
  // https://stackoverflow.com/questions/13296461/imperfect-forwarding-with-variadic-templates

  template <typename A, typename B>
  using disable_if_same_or_derived =
      std::enable_if_t<!std::is_base_of_v<A, std::remove_reference_t<B>>>;

  template <typename U, typename X = disable_if_same_or_derived<ItemWrapper, U>>
  ItemWrapper(U&& u) : _data(std::forward<U>(u)) {}
};
}  // namespace impl

using impl::Item, impl::List;

std::string encode(const Item&);
Item decode(const std::string&);

std::string to_binary(uint64_t);
}  // namespace silkworm::rlp

#endif  // SILKWORM_RLP_HPP_
