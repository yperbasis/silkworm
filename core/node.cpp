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

Node::Node(DbBucket& db, const sync::Hints& hints,
           std::optional<uint32_t> data_valid_for_block)
    : state_{db, std::min(hints.optimal_phase1_depth(),
                          hints.depth_to_fit_in_memory())} {
  if (data_valid_for_block) {
    state_.init_from_db(*data_valid_for_block);
  }
}

sync::Stats Node::sync(const Node& peer) {
  // TODO error handling, incl resend of timed-out request
  // TODO separate the wheat from the chaff (good vs bad peers)
  // TODO storage sync
  auto& state = state_;
  sync::Stats stats;
  // TODO buffered request/reply dequeue?
  while (auto request = state.next_sync_request()) {
    ++stats.num_requests;
    stats.request_total_bytes += request->byte_size();
    auto reply = peer.get_state_leaves(*request);
    std::visit(
        [&state, &stats](auto&& arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, sync::Reply>) {
            ++stats.num_replies;
            stats.reply_total_bytes += arg.byte_size();
            if (arg.leaves) {
              stats.reply_total_leaves += arg.leaves->size();
            }
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
  return stats;
}

}  // namespace silkworm
