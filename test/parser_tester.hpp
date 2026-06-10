/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget/protocol/json/parser.hpp"

namespace roq {
namespace bitget {

template <typename T>
struct ParserTester final : public protocol::json::Parser::Handler {
  using value_type = std::remove_cvref_t<T>;
  using callback_type = std::function<void(value_type const &)>;

  static void dispatch(callback_type const &callback, std::string_view const &message, size_t buffer_size, size_t max_depth) {
    core::json::BufferStack buffers{buffer_size, max_depth};
    // simple
    // XXX FIXME TODO catch2 block ???
    T obj{message, buffers};
    callback(obj);
    // parser
    // XXX FIXME TODO catch2 block ???
    ParserTester handler{callback};
    auto res = protocol::json::Parser::dispatch(handler, message, buffers, {}, false);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit ParserTester(callback_type const &callback) : callback_{callback} {}

  void operator()(Trace<protocol::json::Error> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Subscribe> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Ticker> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::PublicTrade> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Books> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Login> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Account> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Position> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Order> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::Fill> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::PlaceOrder> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::ModifyOrder> const &event) override { dispatch_helper(event); }
  void operator()(Trace<protocol::json::CancelOrder> const &event) override { dispatch_helper(event); }

  template <typename U>
  void dispatch_helper(Trace<U> const &event) {
    if constexpr (std::is_invocable_v<callback_type, U>) {
      found_ = true;
      callback_(event);
    } else {
      FAIL();
    }
  }

 private:
  callback_type const callback_;
  bool found_ = false;
};

}  // namespace bitget
}  // namespace roq
