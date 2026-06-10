/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <span>
#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/protocol/json/error.hpp"
#include "roq/bitget/protocol/json/subscribe.hpp"

#include "roq/bitget/protocol/json/books.hpp"
#include "roq/bitget/protocol/json/public_trade.hpp"
#include "roq/bitget/protocol/json/ticker.hpp"

#include "roq/bitget/protocol/json/login.hpp"

#include "roq/bitget/protocol/json/account.hpp"
#include "roq/bitget/protocol/json/fill.hpp"
#include "roq/bitget/protocol/json/order.hpp"
#include "roq/bitget/protocol/json/position.hpp"

#include "roq/bitget/protocol/json/cancel_order.hpp"
#include "roq/bitget/protocol/json/modify_order.hpp"
#include "roq/bitget/protocol/json/place_order.hpp"

namespace roq {
namespace bitget {
namespace protocol {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<protocol::json::Error> const &) = 0;
    virtual void operator()(Trace<protocol::json::Subscribe> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::Ticker> const &) = 0;
    virtual void operator()(Trace<protocol::json::PublicTrade> const &) = 0;
    virtual void operator()(Trace<protocol::json::Books> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::Login> const &) = 0;
    virtual void operator()(Trace<protocol::json::Account> const &) = 0;
    virtual void operator()(Trace<protocol::json::Position> const &) = 0;
    virtual void operator()(Trace<protocol::json::Order> const &) = 0;
    virtual void operator()(Trace<protocol::json::Fill> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::PlaceOrder> const &) = 0;
    virtual void operator()(Trace<protocol::json::ModifyOrder> const &) = 0;
    virtual void operator()(Trace<protocol::json::CancelOrder> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace protocol
}  // namespace bitget
}  // namespace roq
