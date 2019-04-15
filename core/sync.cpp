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

#include <algorithm>

namespace silkworm::sync {

uint8_t Hints::depth_to_fit_in_memory() const {
  for (uint8_t i = 2; i <= 15; ++i) {
    if (tree_size_in_bytes(i) > max_memory) {
      return i - 1;
    }
  }
  return 15;
}

uint8_t Hints::optimal_phase2_depth() const {
  std::vector<double> v;
  for (uint8_t i = 0; i < 16; ++i) {
    v.push_back(rqs(i));
  }

  const auto min = std::min_element(v.cbegin(), v.cend());
  return static_cast<uint8_t>(std::distance(v.cbegin(), min));
}

uint8_t Hints::optimal_phase1_depth() const {
  for (uint8_t i = 1; i < 16; ++i) {
    const uint64_t num_requests = 1ull << (i * 4);

    if (num_leaves * leaf_size <= approx_max_reply_size * num_requests) {
      return i;
    }
  }
  return 16;
}

double Hints::inf_bandwidth_reply_overhead() const {
  const auto depth = optimal_phase1_depth();
  const uint64_t num_proofs = num_tree_nodes(depth);
  const uint64_t total_leaf_size = num_leaves * leaf_size;
  const uint64_t total_reply_size = num_proofs * node_size + total_leaf_size;

  return static_cast<double>(total_reply_size) / total_leaf_size - 1;
}

double Hints::rqs(const uint8_t depth) const {
  uint64_t c = 0;
  for (uint8_t i = 0; i < depth; ++i) {
    c += std::min(1ull << (i * 4),
                  static_cast<unsigned long long>(changes_per_block));
  }

  const double leaves_per_reply =
      static_cast<double>(num_leaves) / (1ull << (depth * 4));

  return c * node_size + changes_per_block * leaves_per_reply * leaf_size;
}

}  // namespace silkworm::sync
