/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/json/utils.hpp"

#include "roq/bitget/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace json {

Error guess_error(int code) {
  switch (code) {
    case 429:  // Too Many Requests
      return Error::REQUEST_RATE_LIMIT_REACHED;
    case 22001:  // XXX cancel "no orders to cancel"
      return Error::TOO_LATE_TO_MODIFY_OR_CANCEL;
    case 25202:  // Insufficient balance
      return Error::INSUFFICIENT_FUNDS;
    case 25203:  // Insufficient margin
      return Error::INSUFFICIENT_FUNDS;
  }
  return Error::UNKNOWN;
}

}  // namespace json
}  // namespace bitget
}  // namespace roq
