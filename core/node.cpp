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

#include "node.hpp"

#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace {
// https://en.cppreference.com/w/cpp/utility/variant/visit
template <class T>
struct AlwaysFalse : std::false_type {};
}  // namespace

namespace silkworm {

void Node::sync(const Node& peer) {
  // TODO error handling, incl resend of timed-out request
  // TODO separate the wheat from the chaff (good vs bad peers)
  // TODO storage sync
  auto& state = state_;
  // TODO buffered request/reply dequeue?
  while (auto request = state.next_sync_request()) {
    auto reply = peer.get_state_leaves(*request);
    std::visit(
        [&state](auto&& arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, sync::Reply>) {
            state.process_sync_data(arg);
          } else if constexpr (std::is_same_v<T, sync::Error>) {
            std::cerr << "sync error " << arg << std::endl;
            throw std::runtime_error("TODO better error handling");
          } else {
            static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor!");
          }
        },
        reply);
  }
}

}  // namespace silkworm
