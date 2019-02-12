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

namespace silkworm {

class State {
  explicit State(DbBucket& db, unsigned level)
      : db_(db), level_(level), chunks_(16ll << level) {
    // TODO build the chunks from the database
    // (calculate hashes w/o extension nodes to start with)
  }

 private:
  BOOST_MOVABLE_BUT_NOT_COPYABLE(State)

  struct Node {
    std::array<Hash, 16> hash;
    std::array<int32_t, 16> block = {-1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1};
  };

  DbBucket& db_;
  unsigned level_;
  std::vector<Node> chunks_;
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_STATE_HPP_
