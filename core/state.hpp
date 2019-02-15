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

#ifndef SILKWORM_CORE_STATE_HPP_
#define SILKWORM_CORE_STATE_HPP_

#include <vector>

#include <boost/move/utility_core.hpp>

#include "db_bucket.hpp"
#include "sync.hpp"

namespace silkworm {

class State {
 public:
  State(DbBucket& db, unsigned level);

  unsigned level() const { return tree_.size() - 1; }

  void init_from_db(uint32_t data_valid_for_block);

  std::variant<sync::Reply, sync::Error> get_leaves(const sync::Request&) const;

  std::optional<sync::Request> next_sync_request();

  void process_sync_data(const sync::Reply&);

 private:
  BOOST_MOVABLE_BUT_NOT_COPYABLE(State)

  struct Node {
    std::array<Hash, 16> hash;
    uint16_t have_data = 0;  // bits
    uint32_t block = 0;
  };

  DbBucket& db_;

  // TODO unify with mptrie
  std::vector<std::vector<Node>> tree_;

  uint64_t sync_request_cursor_ = 0;
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_STATE_HPP_
