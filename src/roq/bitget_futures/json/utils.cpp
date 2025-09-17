/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget_futures/json/utils.hpp"

#include "roq/bitget_futures/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace bitget_futures {
namespace json {

Error guess_error(int code) {
  // XXX FIXME TODO 22001 => no orders to cancel ==> need Error mapping
  return Error::UNKNOWN;
}

}  // namespace json
}  // namespace bitget_futures
}  // namespace roq
