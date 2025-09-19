/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <span>
#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/error.hpp"
#include "roq/bitget/json/subscribe.hpp"

#include "roq/bitget/json/books.hpp"
#include "roq/bitget/json/public_trade.hpp"
#include "roq/bitget/json/ticker.hpp"

#include "roq/bitget/json/login.hpp"

#include "roq/bitget/json/account.hpp"
#include "roq/bitget/json/fill.hpp"
#include "roq/bitget/json/order.hpp"
#include "roq/bitget/json/position.hpp"

namespace roq {
namespace bitget {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<json::Error> const &) = 0;
    virtual void operator()(Trace<json::Subscribe> const &) = 0;
    //
    virtual void operator()(Trace<json::Ticker> const &) = 0;
    virtual void operator()(Trace<json::PublicTrade> const &) = 0;
    virtual void operator()(Trace<json::Books> const &) = 0;
    //
    virtual void operator()(Trace<json::Login> const &) = 0;
    virtual void operator()(Trace<json::Account> const &) = 0;
    virtual void operator()(Trace<json::Position> const &) = 0;
    virtual void operator()(Trace<json::Order> const &) = 0;
    virtual void operator()(Trace<json::Fill> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types = false);
};

}  // namespace json
}  // namespace bitget
}  // namespace roq
