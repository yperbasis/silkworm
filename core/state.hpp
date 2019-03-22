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

#include "memdb_bucket.hpp"
#include "sync.hpp"

namespace silkworm {

class State {
 public:
  static constexpr size_t kMaxNodesPerRequest = 64;

  State(MemDbBucket& db, uint8_t depth, uint8_t phase1_depth);

  uint8_t depth() const { return static_cast<uint8_t>(tree_.size()); }

  void init_from_db(uint32_t data_valid_for_block);

  void put(Hash key, std::string val);

  sync::LeavesReply get_leaves(const sync::GetLeavesRequest&) const;

  std::optional<sync::NodeReply> get_nodes(const sync::GetNodeRequest&) const;

  bool phase1_sync_done() const { return phase1_sync_done_; }

  std::variant<std::monostate, sync::GetLeavesRequest, sync::GetNodeRequest>
  next_sync_request();

  void process_leaves_reply(Prefix, const sync::LeavesReply&);

  void process_node_reply(const sync::GetNodeRequest&, const sync::NodeReply&);

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

  MemDbBucket& db_;

  // TODO unify with mptrie
  // Invariant: parent.block >= child.block if parent.block != -1.
  std::vector<std::vector<Node>> tree_;

  Prefix phase1_cursor_;
  Prefix phase2_leaf_cursor_;
  Prefix phase2_node_cursor_ = Prefix(1);

  bool phase1_sync_done_ = false;

  sync::GetNodeRequest next_node_request();
  std::optional<sync::GetLeavesRequest> next_leaves_request(Prefix&,
                                                            bool phase1);

  static uint64_t node_index(uint8_t level, Prefix prefix) {
    return level == 0 ? 0 : prefix.val() >> (64 - level * 4);
  }

  const Node& node(uint8_t level, Prefix prefix) const {
    return tree_[level][node_index(level, prefix)];
  }

  Node& node(uint8_t level, Prefix prefix) {
    return tree_[level][node_index(level, prefix)];
  }

  Node& root() { return tree_[0][0]; }
  const Node& root() const { return tree_[0][0]; }

  void update_blocks_down_path(Prefix);
  bool update_block_at(Prefix, uint8_t level);

  uint8_t consistent_path_depth(Prefix) const;

  void propagate_synced_up(Prefix, uint8_t from_level);

  void update_node(Node&, const sync::Proof& new_data, int32_t new_block);

  static bool nibble_obsolete(const Node&, Nibble, bool new_empty,
                              const Hash& new_hash);
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_STATE_HPP_
