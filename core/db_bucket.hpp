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

#ifndef SILKWORM_CORE_DB_BUCKET_HPP_
#define SILKWORM_CORE_DB_BUCKET_HPP_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <boost/move/utility_core.hpp>

#include "common.hpp"
#include "prefix.hpp"

namespace silkworm {

class DbBucket {
 public:
  using Map = std::map<Hash, std::string>;
  using ConstIterator = Map::const_iterator;
  using Range = std::pair<ConstIterator, ConstIterator>;

  DbBucket() = default;

  void put(Hash key, std::string val) {
    data_.insert_or_assign(std::move(key), std::move(val));
  }

  Range prefix_range(const Prefix&) const;

  ConstIterator not_found() const { return data_.end(); }

 private:
  BOOST_MOVABLE_BUT_NOT_COPYABLE(DbBucket)

  Map data_;
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_DB_BUCKET_HPP_
