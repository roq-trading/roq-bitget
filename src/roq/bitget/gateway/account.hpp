/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/bitget/gateway/config.hpp"

#include "roq/bitget/tools/crypto.hpp"

namespace roq {
namespace bitget {
namespace gateway {

struct Account final {
  Account(Config const &, std::string_view const &name);

  Account(Account const &) = delete;

  std::string create_ws_login();

  std::string create_headers(web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body);

  std::string const name;

 private:
  tools::Crypto crypto_;
};

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
