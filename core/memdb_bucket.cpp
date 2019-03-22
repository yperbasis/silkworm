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

#include "memdb_bucket.hpp"

namespace silkworm {

MemDbBucket::Range MemDbBucket::leaves(Prefix p) const {
  if (p.size() == 0) {
    return {data_.begin(), data_.end()};
  }

  auto lb = data_.lower_bound(p.to_string());

  if (lb == data_.end()) {
    return {lb, data_.end()};
  }

  ++p;
  if (p.val() == 0) {
    return {lb, data_.end()};
  }

  std::string next_hash = p.to_string();

  if (lb->first >= next_hash) {
    return {data_.end(), data_.end()};
  } else {
    return {lb, data_.lower_bound(next_hash)};
  }
}

void MemDbBucket::del(std::string_view lower,
                      std::optional<std::string_view> upper) {
  if (upper && lower.compare(*upper) >= 0) {
    return;
  }

  const auto first = data_.lower_bound(std::string(lower));
  const auto last =
      upper ? data_.lower_bound(std::string(*upper)) : data_.end();
  data_.erase(first, last);
}

void MemDbBucket::erase(Prefix p) {
  const auto range = p.string_range();
  del(range.first, range.second);
}

}  // namespace silkworm
