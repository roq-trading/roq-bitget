/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/bitget/gateway/shared.hpp"

#include "roq/bitget/protocol/json/parser.hpp"

namespace roq {
namespace bitget {
namespace gateway {

struct MarketData final : public web::socket::Client::Handler, public protocol::json::Parser::Handler {
  struct Handler {};

  MarketData(Handler &, io::Context &, uint16_t stream_id, Shared &, size_t index);

  MarketData(MarketData const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  void subscribe(size_t start_from = 0);

 protected:
  // web::socket::Client::Handler

  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

  // helpers

  uint16_t stream_id() const { return stream_id_; }

  bool ready() const { return connection_status_ == ConnectionStatus::READY; }

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  void subscribe(std::span<Symbol const> const &symbols);
  void subscribe(std::string_view const &topic, std::span<Symbol const> const &symbols);

  void parse(std::string_view const &message);

  // protocol::json::Parser::Handler

  void operator()(Trace<protocol::json::Error> const &) override;
  void operator()(Trace<protocol::json::Subscribe> const &) override;

  void operator()(Trace<protocol::json::Ticker> const &) override;
  void operator()(Trace<protocol::json::PublicTrade> const &) override;
  void operator()(Trace<protocol::json::Books> const &) override;

  void operator()(Trace<protocol::json::Login> const &) override;
  void operator()(Trace<protocol::json::Account> const &) override;
  void operator()(Trace<protocol::json::Position> const &) override;
  void operator()(Trace<protocol::json::Order> const &) override;
  void operator()(Trace<protocol::json::Fill> const &) override;

  void operator()(Trace<protocol::json::PlaceOrder> const &) override;
  void operator()(Trace<protocol::json::ModifyOrder> const &) override;
  void operator()(Trace<protocol::json::CancelOrder> const &) override;

 private:
  [[maybe_unused]] Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  size_t const index_;
  // web socket
  std::unique_ptr<web::socket::Client> connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse, error, subscribe, ticker, public_trade, books;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  std::chrono::nanoseconds next_ping_ = {};
};

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
