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

#include "rlp.hpp"

#include <iterator>
#include <optional>
#include <stdexcept>
#include <utility>

namespace {
using namespace silkworm::rlp;

// https://www.boost.org/doc/libs/1_69_0/doc/html/variant/tutorial.html#variant.tutorial.binary-visitation
struct Comparator : public boost::static_visitor<bool> {
  template <typename T, typename U>
  bool operator()(const T&, const U&) const {
    return false;  // cannot compare different types
  }

  template <typename T>
  bool operator()(const T& lhs, const T& rhs) const {
    return lhs == rhs;
  }
};

std::string encode_length(uint64_t len, char offset) {
  if (len < 56) {
    return {static_cast<char>(len + offset)};
  } else {
    const auto bl = to_big_endian(len);
    return static_cast<char>(bl.length() + offset + 55) + bl;
  }
}

size_t big_endian_length(uint64_t x) {
  size_t len = 0;
  for (; x != 0; x /= 256) {
    ++len;
  }
  return len;
}

size_t length_of_length(uint64_t len) {
  return len < 56 ? 1 : 1 + big_endian_length(len);
}

struct LengthEncoder : public boost::static_visitor<size_t> {
  size_t operator()(const std::string& in) const {
    size_t len = in.length();

    if (in.length() != 1 || static_cast<unsigned>(in[0]) >= 0x80) {
      len += length_of_length(in.length());
    }

    return len;
  }

  size_t operator()(const List& in) const {
    size_t len = 0;
    for (const auto& item : in) {
      len += boost::apply_visitor(LengthEncoder(), item);
    }
    return length_of_length(len) + len;
  }
};

class Encoder : public boost::static_visitor<> {
 public:
  explicit Encoder(std::string& out) : out_(out){};

  void operator()(const std::string& in) const {
    if (in.length() != 1 || static_cast<unsigned>(in[0]) >= 0x80) {
      out_ += encode_length(in.length(), 0x80);
    }

    out_ += in;
  }

  void operator()(const List& in) const {
    size_t len = 0;
    for (const auto& item : in) {
      len += boost::apply_visitor(LengthEncoder(), item);
    }
    out_ += encode_length(len, 0xc0);

    for (const auto& item : in) {
      encode(item, out_);
    }
  }

 private:
  std::string& out_;
};

struct DecodedLength {
  size_t data_len = 0;
  bool is_str = false;
};

DecodedLength decode_length(std::string_view& input) {
  if (input.empty()) {
    throw std::invalid_argument("input is null");
  }

  const unsigned char prefix = input[0];

  if (prefix <= 0x7f) {
    return {1, true};
  }

  input.remove_prefix(1);

  if (prefix <= 0xb7) {
    const auto str_len = prefix - 0x80u;
    if (input.length() < str_len) {
      throw std::invalid_argument("truncated string");
    }
    return {str_len, true};
  } else if (prefix <= 0xbf) {
    const auto len_of_len = prefix - 0xb7u;
    if (input.length() < len_of_len) {
      throw std::invalid_argument("truncated length");
    }
    const auto str_len = from_big_endian(input.substr(0, len_of_len));
    input.remove_prefix(len_of_len);
    if (input.length() < str_len) {
      throw std::invalid_argument("truncated string");
    }
    return {str_len, true};
  } else if (prefix <= 0xf7) {
    const auto list_len = prefix - 0xc0u;
    if (input.length() < list_len) {
      throw std::invalid_argument("truncated list");
    }
    return {list_len, false};
  } else {
    const auto len_of_len = prefix - 0xf7u;
    if (input.length() < len_of_len) {
      throw std::invalid_argument("truncated length");
    }
    const auto list_len = from_big_endian(input.substr(0, len_of_len));
    input.remove_prefix(len_of_len);
    if (input.length() < list_len) {
      throw std::invalid_argument("truncated list");
    }
    return {list_len, false};
  }
}

std::optional<Item> decode_impl(std::string_view& input) {
  if (input.empty()) {
    return {};
  }

  const auto x = decode_length(input);
  std::string_view sub_view = input.substr(0, x.data_len);

  Item output;
  if (x.is_str) {
    output = std::string{sub_view};
  } else {
    List list;
    while (const auto next = decode_impl(sub_view)) {
      list.push_back(std::move(*next));
    }
    output = list;
  }

  input.remove_prefix(x.data_len);
  return output;
}

}  // namespace

namespace silkworm::rlp {

void encode(const Item& in, std::string& out) {
  boost::apply_visitor(Encoder(out), in);
}

Item decode(std::string_view input) {
  const auto output = decode_impl(input);
  if (!output) {
    throw std::invalid_argument("input is null");
  }
  if (!input.empty()) {
    throw std::invalid_argument("extra input");
  }
  return *output;
}

std::string to_big_endian(uint64_t x) {
  if (x == 0)
    return "";
  else
    return to_big_endian(x / 256) + static_cast<char>(x % 256);
}

std::string to_big_endian(UInt256 x) {
  std::string out;
  export_bits(x, std::back_inserter(out), 8);
  return out;
}

uint64_t from_big_endian(std::string_view b) {
  uint64_t res = 0;
  for (uint8_t c : b) {
    res = res * 0x100 + c;
  }
  return res;
}

bool are_equal(const Item& lhs, const Item& rhs) {
  return boost::apply_visitor(Comparator(), lhs, rhs);
}
}  // namespace silkworm::rlp
