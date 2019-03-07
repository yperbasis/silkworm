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

uint8_t depth(const silkworm::sync::Hints& hints) {
  return std::min(hints.optimal_phase2_depth(), hints.depth_to_fit_in_memory());
}

uint8_t phase1_depth(const silkworm::sync::Hints& hints) {
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

  while (true) {
    const auto request = state.next_sync_request();

    if (std::holds_alternative<std::monostate>(request)) {
      return;
    }

    ++stats.num_requests;

    if (auto leaves_request = std::get_if<sync::GetLeavesRequest>(&request)) {
      stats.request_total_bytes += leaves_request->byte_size();

      const auto reply = peer.get_state_leaves(*leaves_request);

      if (reply.status != sync::LeavesReply::kOK) {
        std::cerr << "sync error " << reply.status << std::endl;
        throw std::runtime_error("TODO better error handling");
      }

      bytes_used += reply.byte_size();
      stats.reply_total_bytes += reply.byte_size();
      if (reply.leaves) {
        stats.reply_total_leaves += reply.leaves->size();
      }

      state.process_leaves_reply(leaves_request->prefix, reply);
    } else if (auto node_request =
                   std::get_if<sync::GetNodeRequest>(&request)) {
      stats.request_total_bytes += node_request->byte_size();

      const auto reply = peer.get_state_nodes(*node_request);

      if (!reply) {
        throw std::runtime_error("Unexpected null NodeReply");
      }

      bytes_used += reply->byte_size();
      stats.reply_total_bytes += reply->byte_size();

      state.process_node_reply(*node_request, *reply);
    }

    ++stats.num_replies;

    if (bytes_used >= max_bytes) {
      return;
    }
  }
  return;
}

bool Node::phase1_sync_done() const { return state_.phase1_sync_done(); }

bool Node::sync_done() const { return state_.synced_block() >= 0; }

}  // namespace silkworm
