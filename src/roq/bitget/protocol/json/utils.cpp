/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/protocol/json/utils.hpp"

#include "roq/bitget/protocol/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace protocol {
namespace json {

bool is_logon_error(int32_t code) {
  switch (code) {
    case 30005:  // Login failed
    case 30011:  // Invalid ACCESS_KEY
    case 30012:  // Invalid ACCESS_PASSPHRASE
    case 30013:  // Invalid ACCESS_TIMESTAMP
    case 30015:  // Invalid signature
      return true;
  }
  return false;
}

}  // namespace json
}  // namespace protocol
}  // namespace bitget
}  // namespace roq
