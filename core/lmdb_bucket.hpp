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

#ifndef SILKWORM_CORE_LMDB_BUCKET_HPP_
#define SILKWORM_CORE_LMDB_BUCKET_HPP_

#include <functional>
#include <optional>
#include <string_view>

#include <boost/move/utility_core.hpp>

#include "lmdb++.h"

namespace silkworm {

class LmdbEnvironment {
 public:
  static constexpr size_t kMaxDBs = 16;
  static constexpr size_t kMaxSizeGiB = 1024;

  static LmdbEnvironment& temporaryInstance() {
    static LmdbEnvironment instance;
    return instance;
  }

  LmdbEnvironment(LmdbEnvironment const&) = delete;
  void operator=(LmdbEnvironment const&) = delete;

 private:
  LmdbEnvironment();
  lmdb::env env_;
  friend class LmdbBucket;
};

class LmdbBucket {
 public:
  explicit LmdbBucket(
      std::string_view name,
      LmdbEnvironment& env = LmdbEnvironment::temporaryInstance());

  void put(std::string_view key, std::string_view val);

  std::optional<std::string_view> get(std::string_view key) const;

  // Iterate over entries with lower <= key < upper
  // and call f(key, val) for each entry.
  void get(
      std::string_view lower, std::optional<std::string_view> upper,
      const std::function<void(std::string_view, std::string_view)>& f) const;

 private:
  BOOST_MOVABLE_BUT_NOT_COPYABLE(LmdbBucket)

  lmdb::env& env_;
  lmdb::dbi dbi_;
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_LMDB_BUCKET_HPP_
