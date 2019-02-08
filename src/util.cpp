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

#include "util.hpp"

#include <iterator>

#include <boost/algorithm/hex.hpp>

namespace silkworm {

std::string bytes_to_hex_string(std::string_view in) {
  std::string res;
  res.reserve(in.size() * 2);
  boost::algorithm::hex_lower(in.begin(), in.end(), std::back_inserter(res));
  return res;
}

std::string hex_string_to_bytes(std::string_view in) {
  std::string res;
  res.reserve(in.size() / 2);
  boost::algorithm::unhex(in.begin(), in.end(), std::back_inserter(res));
  return res;
}

}  // namespace silkworm
