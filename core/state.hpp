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

#include <bitset>
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
    int32_t block = -1;  // -1 means not fully initialized yet
    std::bitset<16> empty = std::bitset<16>{}.flip();
    std::array<Hash, 16> hash;
    std::bitset<16> leaves_in_db;
  };

  DbBucket& db_;

  // TODO unify with mptrie
  // TODO invariant: parent.block >= child.block if parent.block != -1
  std::vector<std::vector<Node>> tree_;

  std::optional<Prefix> sync_request_cursor_;

  const Node& node(unsigned level, Prefix prefix) const {
    return tree_[level][prefix.val() >> (64 - level * 4)];
  }

  Node& node(unsigned level, Prefix prefix) {
    return const_cast<Node&>(
        static_cast<const State*>(this)->node(level, prefix));
  }

  Node& root() { return tree_[0][0]; }
  const Node& root() const { return tree_[0][0]; }

  unsigned consistent_path_depth(Prefix) const;
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_STATE_HPP_
