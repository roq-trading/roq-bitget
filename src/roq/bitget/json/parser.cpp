/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/bitget/json/message.hpp"
#include "roq/bitget/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace json {

// === CONSTANTS ===

namespace {
auto const PONG = "pong"sv;
}

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  if (message == PONG) {
    // note! drop
    return true;
  }
  Message message_2{message, buffer_stack};
  switch (message_2.event) {
    using enum Event::type_t;
    case UNDEFINED_INTERNAL:
      break;
    case UNKNOWN_INTERNAL:
      if (allow_unknown_event_types) {
        return false;
      }
      break;
    case ERROR: {
      auto error = Error{
          .event = message_2.event,
          .code = message_2.code,
          .msg = message_2.msg,
      };
      create_trace_and_dispatch(handler, trace_info, error);
      return true;
    }
    case SUBSCRIBE: {
      dispatch_helper<Subscribe>(handler, message, buffer_stack, trace_info);
      return true;
    }
    case LOGIN:
      dispatch_helper<Login>(handler, message, buffer_stack, trace_info);
      return true;
  }
  switch (message_2.arg.topic) {
    using enum Topic::type_t;
    case UNDEFINED_INTERNAL:
      break;
    case UNKNOWN_INTERNAL:
      return false;  // unexpected
    case TICKER:
      dispatch_helper<Ticker>(handler, message, buffer_stack, trace_info);
      return true;
    case PUBLIC_TRADE:
      dispatch_helper<PublicTrade>(handler, message, buffer_stack, trace_info);
      return true;
    case BOOKS:
    case BOOKS5:
    case BOOKS15: {
      dispatch_helper<Books>(handler, message, buffer_stack, trace_info);
      return true;
    }
    //
    case ACCOUNT:
      dispatch_helper<Account>(handler, message, buffer_stack, trace_info);
      return true;
    case POSITION:
      dispatch_helper<Position>(handler, message, buffer_stack, trace_info);
      return true;
    case ORDER:
      dispatch_helper<Order>(handler, message, buffer_stack, trace_info);
      return true;
    case FILL:
      dispatch_helper<Fill>(handler, message, buffer_stack, trace_info);
      return true;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace bitget
}  // namespace roq
