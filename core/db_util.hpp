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

#ifndef SILKWORM_CORE_DB_UTIL_HPP_
#define SILKWORM_CORE_DB_UTIL_HPP_

#include "mptrie.hpp"
#include "prefix.hpp"

namespace silkworm::db_util {

template <class DbBucket>
void iterate(const DbBucket& db, Prefix p,
             const std::function<void(std::string_view, std::string_view)>& f) {
  const auto range = p.string_range();
  db.get(range.first, range.second, f);
}

template <class DbBucket>
LeafHasher hasher(const DbBucket& db, Prefix p) {
  LeafHasher hasher;
  iterate(db, p, [&hasher](std::string_view key, std::string_view val) {
    hasher.append(key, val);
  });
  return hasher;
}

template <class DbBucket>
void del(DbBucket& db, Prefix p) {
  const auto range = p.string_range();
  db.del(range.first, range.second);
}

}  // namespace silkworm::db_util

#endif  // SILKWORM_CORE_DB_UTIL_HPP_
