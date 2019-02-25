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

unsigned depth(const silkworm::sync::Hints& hints) {
  return std::min(hints.fine_grained_depth(), hints.depth_to_fit_in_memory());
}

unsigned phase1_depth(const silkworm::sync::Hints& hints) {
  return std::min(hints.optimal_phase1_depth(), depth(hints));
}

}  // namespace

namespace silkworm {

Node::Node(DbBucket& db, const sync::Hints& hints,
           std::optional<uint32_t> data_valid_for_block)
    : state_{db, depth(hints), phase1_depth(hints)} {
  if (data_valid_for_block) {
    state_.init_from_db(*data_valid_for_block);
  }
}

void Node::sync(const Node& peer, sync::Stats& stats, uint64_t max_bytes) {
  // TODO error handling, incl resend of timed-out request
  // TODO separate the wheat from the chaff (good vs bad peers)
  // TODO buffered request/reply dequeue?

  auto& state = state_;
  uint64_t bytes_used = 0;

  while (auto request = state.next_sync_request()) {
    ++stats.num_requests;
    stats.request_total_bytes += request->byte_size();
    auto reply = peer.get_state_leaves(*request);

    std::visit(
        [&state, &stats, &bytes_used](auto&& rpl) {
          using T = std::decay_t<decltype(rpl)>;
          if constexpr (std::is_same_v<T, sync::Reply>) {
            ++stats.num_replies;
            bytes_used += rpl.byte_size();
            stats.reply_total_bytes += rpl.byte_size();
            if (rpl.leaves) {
              stats.reply_total_leaves += rpl.leaves->size();
            }
            state.process_sync_data(rpl);
          } else if constexpr (std::is_same_v<T, sync::Error>) {
            std::cerr << "sync error " << rpl << std::endl;
            throw std::runtime_error("TODO better error handling");
          } else {
            static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor!");
          }
        },
        reply);

    if (bytes_used >= max_bytes) {
      return;
    }
  }
  return;
}

bool Node::phase1_sync_done() const { return state_.phase1_sync_done(); }

bool Node::sync_done() const { return state_.synced_block() >= 0; }

}  // namespace silkworm
