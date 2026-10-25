/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/gateway/request.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace gateway {

// === IMPLEMENTATION ===

Request::Request(Shared &shared)
    : account_settings{*this}, account_assets{*this}, current_positions{*this}, unfilled_orders{*this}, trade_fills{*this}, shared_{shared} {
}

bool Request::ready() const {
  return account_settings.ready() && account_assets.ready() && current_positions.ready() && unfilled_orders.ready() && trade_fills.ready();
}

bool Request::can_request(TraceInfo const &trace_info) {
  return shared_.rate_limiter.can_request(trace_info);
}

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
