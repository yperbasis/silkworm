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

#include "db_bucket.hpp"

#include <boost/filesystem.hpp>

namespace silkworm {

DbBucket::DbBucket() : env_{lmdb::env::create()} {
  env_.set_mapsize(kMaxDbSizeInGiB * 1024ull * 1024ull * 1024ull);

  using namespace boost::filesystem;

  auto tmp_dir = unique_path();
  create_directories(tmp_dir);

  env_.open(tmp_dir.c_str(), 0, 0664);
}

DbBucket::Range DbBucket::leaves(Prefix p) const {
  if (p.size() == 0) {
    return {data_.begin(), data_.end()};
  }

  auto lb = data_.lower_bound(p.padded());

  if (lb == data_.end()) {
    return {lb, data_.end()};
  }

  ++p;
  if (p.val() == 0) {
    return {lb, data_.end()};
  }

  Hash next_hash = p.padded();

  if (lb->first >= next_hash) {
    return {data_.end(), data_.end()};
  } else {
    return {lb, data_.lower_bound(next_hash)};
  }
}

void DbBucket::erase(Prefix p) {
  const auto range = leaves(p);
  data_.erase(range.first, range.second);
}

}  // namespace silkworm
