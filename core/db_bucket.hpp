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

#include <functional>
#include <optional>
#include <string_view>
#include <utility>

#include <boost/move/utility_core.hpp>

namespace silkworm {

class DbBucket {
 public:
  using KeyVal = std::pair<std::string_view, std::string_view>;

  virtual ~DbBucket() = default;

  virtual void put(std::string_view key, std::string_view val) = 0;

  virtual void put(const std::function<std::optional<KeyVal>()>& gen) = 0;

  virtual std::optional<std::string_view> get(std::string_view key) const = 0;

  // Iterate over entries with lower <= key < upper
  // and call f(key, val) for each entry.
  virtual void get(
      std::string_view lower, std::optional<std::string_view> upper,
      const std::function<void(std::string_view, std::string_view)>& f)
      const = 0;

  // Delete all entries with lower <= key < upper.
  virtual void del(std::string_view lower,
                   std::optional<std::string_view> upper) = 0;

 protected:
  DbBucket() = default;

 private:
  BOOST_MOVABLE_BUT_NOT_COPYABLE(DbBucket)
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_DB_BUCKET_HPP_
