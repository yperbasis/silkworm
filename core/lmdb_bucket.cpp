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

#include "lmdb_bucket.hpp"

#include <boost/filesystem.hpp>

namespace {
lmdb::val to_val(const std::string_view view) {
  return {view.data(), view.size()};
}

lmdb::dbi create_dbi(const std::string_view name, lmdb::env& env) {
  auto wtxn = lmdb::txn::begin(env);
  auto dbi = lmdb::dbi::open(wtxn, name.data(), MDB_CREATE);
  wtxn.commit();
  return dbi;
}
}  // namespace

namespace silkworm {

LmdbEnvironment::LmdbEnvironment() : env_{lmdb::env::create()} {
  env_.set_max_dbs(kMaxDBs);
  env_.set_mapsize(kMaxSizeGiB * 1024ull * 1024ull * 1024ull);

  using namespace boost::filesystem;

  const auto tmp_dir = unique_path();
  create_directories(tmp_dir);
  env_.open(tmp_dir.string().c_str(), 0, 0664);
}

LmdbBucket::LmdbBucket(const std::string_view name, LmdbEnvironment& env)
    : env_{env.env_}, dbi_{create_dbi(name, env_)} {}

void LmdbBucket::put(const std::string_view key, const std::string_view val) {
  auto wtxn = lmdb::txn::begin(env_);
  dbi_.put(wtxn, to_val(key), to_val(val));
  wtxn.commit();
}

std::optional<std::string_view> LmdbBucket::get(
    const std::string_view key) const {
  auto rtxn = lmdb::txn::begin(env_, nullptr, MDB_RDONLY);
  lmdb::val val;
  bool res = dbi_.get(rtxn, to_val(key), val);
  rtxn.abort();

  if (res)
    return std::string_view{val.data(), val.size()};
  else
    return {};
}

}  // namespace silkworm