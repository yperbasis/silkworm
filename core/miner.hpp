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

#ifndef SILKWORM_CORE_MINER_HPP_
#define SILKWORM_CORE_MINER_HPP_

#include "account.hpp"
#include "node.hpp"

namespace silkworm {

class Miner : public Node {
 public:
  Miner(DbBucket& db, const sync::Hints& hints,
        std::optional<uint32_t> block_height)
      : Node(db, hints, block_height) {}

  void new_block();

  // TODO prohibit creation of non-zero balance accounts ex nihilo
  // Must be called after new_block and before seal_block.
  void create_account(const Address&, const Account&);

  void seal_block();

 private:
  uint32_t new_block_ = 0;
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_MINER_HPP_
