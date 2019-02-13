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
  static constexpr auto kStateLevel = 5u;

  Node(DbBucket& db, uint32_t block_height) : state_(db, kStateLevel) {
    state_.init_from_db(block_height);
  }

  void sync();

  std::optional<uint32_t> synced_block() const;

  std::variant<sync::Reply, sync::Error> get_leaves(sync::Request);

 private:
  State state_;
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_NODE_HPP_
