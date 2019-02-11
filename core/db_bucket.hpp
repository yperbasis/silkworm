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

#ifndef SILKWORM_DB_BUCKET_HPP_
#define SILKWORM_DB_BUCKET_HPP_

#include <map>
#include <string>
#include <vector>

#include <boost/move/utility_core.hpp>

#include "common.hpp"

namespace silkworm {

class DbBucket {
 private:
  BOOST_MOVABLE_BUT_NOT_COPYABLE(DbBucket)

  std::map<Hash, std::string> data_;
};

}  // namespace silkworm

#endif  // SILKWORM_DB_BUCKET_HPP_
