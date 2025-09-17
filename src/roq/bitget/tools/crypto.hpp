/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/utils/mac/hmac.hpp"

namespace roq {
namespace bitget {
namespace tools {

class Crypto final {
 public:
  Crypto(std::string_view const &key, std::string_view const &secret, std::string_view const &passphrase);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  std::string create_ws_login(std::chrono::milliseconds now);

  std::string create_headers(
      web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body, std::chrono::milliseconds now);

 private:
  using MAC = utils::mac::HMAC<utils::hash::SHA256>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  std::string const key_;
  MAC mac_;
  Digest digest_;
  std::string const passphrase_;
};

}  // namespace tools
}  // namespace bitget
}  // namespace roq
