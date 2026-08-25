/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/error.hpp"

#include "roq/core/json/parser.hpp"

namespace roq {
namespace bitget {
namespace protocol {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  using result_type = std::remove_cvref_t<decltype(result)>;
  result = core::json::get<result_type>(value);
}

extern roq::Error guess_error(int32_t code);

extern bool is_logon_error(int32_t code);

}  // namespace json
}  // namespace protocol
}  // namespace bitget
}  // namespace roq
