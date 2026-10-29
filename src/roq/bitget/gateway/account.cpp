/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/gateway/account.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/clock.hpp"

namespace roq {
namespace bitget {
namespace gateway {

// === IMPLEMENTATION ===

Account::Account(Settings const &settings, Config const &config, std::string_view const &name)
    : name(name), crypto_(config.get_api_key(name), config.get_secret(name), config.get_passphrase(name), settings.misc.paptrading) {
}

std::string Account::create_ws_login(TraceInfo const &trace_info) {
  return crypto_.create_ws_login(utils::safe_cast(trace_info.origin_create_time_utc));
}

std::string Account::create_headers(
    TraceInfo const &trace_info, web::http::Method method, std::string_view const &path, std::string_view const &query, std::string_view const &body) {
  return crypto_.create_headers(method, path, query, body, utils::safe_cast(trace_info.origin_create_time_utc));
}

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
