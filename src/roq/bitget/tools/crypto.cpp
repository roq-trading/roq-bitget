/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/tools/crypto.hpp"

#include <fmt/format.h>

#include <array>

#include "roq/utils/codec/base64.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace tools {

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &key, std::string_view const &secret, std::string_view const &passphrase)
    : key_{key}, mac_{secret}, passphrase_{passphrase} {
}

std::string Crypto::create_ws_login(std::chrono::milliseconds timestamp) {
  auto tmp = fmt::format("{}GET/user/verify"sv, timestamp.count());
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_);
  std::string signature;
  utils::codec::Base64::encode(signature, digest, false, false);
  auto result = fmt::format(
      R"({{)"
      R"("op":"login",)"
      R"("args":[{{)"
      R"("apiKey":"{}",)"
      R"("passphrase":"{}",)"
      R"("timestamp":"{}",)"
      R"("sign":"{}")"
      R"(}})"
      R"(])"
      R"(}})"sv,
      key_,
      passphrase_,
      timestamp.count(),
      signature);
  return result;
}

std::string Crypto::create_headers(
    web::http::Method method, std::string_view const &path, std::string_view const &query, std::string_view const &body, std::chrono::milliseconds timestamp) {
  assert(!std::empty(path));
  auto tmp = fmt::format("{}{}{}{}{}"sv, timestamp.count(), method, path, query, body);
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_);
  std::string signature;
  utils::codec::Base64::encode(signature, digest, false, false);
  auto result = fmt::format(
      "ACCESS-KEY: {}\r\n"
      "ACCESS-SIGN: {}\r\n"
      "ACCESS-TIMESTAMP: {}\r\n"
      "ACCESS-PASSPHRASE: {}\r\n"sv,
      key_,
      signature,
      timestamp.count(),
      passphrase_);
  return result;
}

}  // namespace tools
}  // namespace bitget
}  // namespace roq
