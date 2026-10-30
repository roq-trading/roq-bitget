/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/gateway/request.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace gateway {

// === IMPLEMENTATION ===

Request::Request(Shared &shared) : account{*this}, balances{*this}, positions{*this}, open_orders{*this}, lost_orders{*this}, fills{*this}, shared_{shared} {
}

bool Request::ready() const {
  return account.ready() && balances.ready() && positions.ready() && open_orders.ready() && lost_orders.ready() && fills.ready();
}

bool Request::can_request(TraceInfo const &trace_info) {
  return shared_.rate_limiter.can_request(trace_info);
}

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
