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

#include "sync.hpp"

namespace silkworm::sync {

unsigned Hints::depth_to_fit_in_memory() const {
  for (unsigned i = 2; i <= 15; ++i) {
    if (tree_size_in_bytes(i) > max_memory) {
      return i - 1;
    }
  }
  return 15;
}

unsigned Hints::fine_grained_depth() const {
  for (unsigned i = 1; i < 16; ++i) {
    const uint64_t num_bottom_hashes = 1ull << (i * 4);
    if (num_bottom_hashes >= num_leaves) {
      return i;
    }
  }
  return 16;
}

unsigned Hints::optimal_phase1_depth() const {
  for (unsigned i = 1; i < 16; ++i) {
    const double num_requests = 1ull << (i * 4);
    const double leaves_per_reply = num_leaves / num_requests;
    const double reply_size =
        i * proof_size + leaves_per_reply * leaf_size + reply_overhead;

    if (reply_size <= approx_max_reply_size) {
      return i;
    }
  }
  return 16;
}

double Hints::phase1_reply_overhead() const {
  const auto depth = optimal_phase1_depth();
  const uint64_t num_requests = 1ull << (depth * 4);
  const uint64_t num_proofs = num_tree_nodes(depth);
  const uint64_t total_leaf_size = num_leaves * leaf_size;
  const uint64_t total_reply_size =
      num_proofs * proof_size + total_leaf_size + num_requests * reply_overhead;

  return static_cast<double>(total_reply_size) / total_leaf_size - 1;
}

}  // namespace silkworm::sync
