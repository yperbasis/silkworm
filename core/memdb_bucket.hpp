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

#ifndef SILKWORM_CORE_MEMDB_BUCKET_HPP_
#define SILKWORM_CORE_MEMDB_BUCKET_HPP_

#include <map>
#include <string>

#include "db_bucket.hpp"

namespace silkworm {

class MemDbBucket : public DbBucket {
 public:
  explicit MemDbBucket(std::string_view = "") {}

  virtual ~MemDbBucket() = default;

  void put(std::string_view key, std::string_view val) override {
    data_[std::string(key)] = val;
  }

  void put(const std::function<std::optional<KeyVal>()>& gen) override;

  std::optional<std::string_view> get(std::string_view key) const override;

  // Iterate over entries with lower <= key < upper
  // and call f(key, val) for each entry.
  void get(std::string_view lower, std::optional<std::string_view> upper,
           const std::function<void(std::string_view, std::string_view)>& f)
      const override;

  // Delete all entries with lower <= key < upper.
  void del(std::string_view lower,
           std::optional<std::string_view> upper) override;

  bool has_same_data(const MemDbBucket& other) const;

 private:
  std::map<std::string, std::string> data_;
};

}  // namespace silkworm

#endif  // SILKWORM_CORE_MEMDB_BUCKET_HPP_
