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
  State(DbBucket& db, unsigned depth, unsigned phase1_depth);

  unsigned depth() const { return tree_.size(); }

  void init_from_db(uint32_t data_valid_for_block);

  void put(Hash key, std::string val);

  sync::LeavesReply get_leaves(const sync::GetLeavesRequest&) const;

  std::optional<sync::NodeReply> get_nodes(const sync::GetNodeRequest&) const;

  bool phase1_sync_done() const { return phase1_sync_done_; }

  std::optional<sync::GetLeavesRequest> next_sync_request();

  void process_sync_data(Prefix, const sync::LeavesReply&);

  int32_t synced_block() const {
    return root().synced.all() ? root().block : -1;
  }

 private:
  BOOST_MOVABLE_BUT_NOT_COPYABLE(State)

  struct Node {
    int32_t block = -1;  // -1 means not fully initialized yet
    std::bitset<16> empty = std::bitset<16>{}.flip();
    std::array<Hash, 16> hash;

    // may only be true if the corresponding subtree is fully
    // consistent with the parent and has all its leaves in the db
    std::bitset<16> synced;
  };

  DbBucket& db_;

  // TODO unify with mptrie
  // Invariant: parent.block >= child.block if parent.block != -1.
  std::vector<std::vector<Node>> tree_;

  Prefix phase1_cursor_;
  Prefix phase2_cursor_;

  bool phase1_sync_done_ = false;

  std::optional<sync::GetLeavesRequest> phase1_sync_request();
  std::optional<sync::GetLeavesRequest> phase2_sync_request();
  std::optional<sync::GetLeavesRequest> next_sync_request(Prefix&, bool phase1);

  static size_t node_index(unsigned level, Prefix prefix) {
    return level == 0 ? 0 : prefix.val() >> (64 - level * 4);
  }

  const Node& node(unsigned level, Prefix prefix) const {
    return tree_[level][node_index(level, prefix)];
  }

  Node& node(unsigned level, Prefix prefix) {
    return tree_[level][node_index(level, prefix)];
  }

  Node& root() { return tree_[0][0]; }
  const Node& root() const { return tree_[0][0]; }

  void update_blocks_down_path(Prefix);

  unsigned consistent_path_depth(Prefix) const;

  static bool nibble_obsolete(const Node&, Nibble, bool new_empty,
                              const Hash& new_hash);
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_STATE_HPP_
