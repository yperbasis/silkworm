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

#ifndef SILKWORM_CORE_NODE_HPP_
#define SILKWORM_CORE_NODE_HPP_

#include <optional>
#include <variant>

#include "db_bucket.hpp"
#include "state.hpp"
#include "sync.hpp"

namespace silkworm {

class Node {
 public:
  Node(DbBucket& db, const sync::Hints&,
       std::optional<uint32_t> data_valid_for_block);

  // TODO multiple peers
  void sync(const Node& peer, sync::Stats& stats, uint64_t max_bytes);

  bool phase1_sync_done() const;

  bool sync_done() const;

  // TODO storage sync
  std::variant<sync::LeavesReply, sync::Error> get_state_leaves(
      sync::GetLeavesRequest request) const {
    return state_.get_leaves(request);
  }

 protected:
  State state_;
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_NODE_HPP_
