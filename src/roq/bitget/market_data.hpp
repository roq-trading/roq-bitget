/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/bitget/shared.hpp"

#include "roq/bitget/json/parser.hpp"

namespace roq {
namespace bitget {

class MarketData final : public web::socket::Client::Handler, public json::Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
    virtual void operator()(Trace<TopOfBook> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<TradeSummary> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate> const &, bool is_last) = 0;
  };

  MarketData(Handler &, io::Context &, uint16_t stream_id, Shared &, size_t index);

  MarketData(MarketData const &) = delete;

  uint16_t stream_id() const { return stream_id_; }

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  void subscribe(size_t start_from = 0);

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  void subscribe(std::span<Symbol const> const &symbols);
  void subscribe(std::string_view const &topic, std::span<Symbol const> const &symbols);

  void parse(std::string_view const &message);

  void operator()(Trace<json::Error> const &) override;
  void operator()(Trace<json::Subscribe> const &) override;

  void operator()(Trace<json::Ticker> const &) override;
  void operator()(Trace<json::PublicTrade> const &) override;
  void operator()(Trace<json::Books> const &) override;

  void operator()(Trace<json::Login> const &) override;
  void operator()(Trace<json::Account> const &) override;
  void operator()(Trace<json::Position> const &) override;
  void operator()(Trace<json::Order> const &) override;
  void operator()(Trace<json::Fill> const &) override;

 private:
  Handler &handler_;
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
  ConnectionStatus status_ = {};
  std::chrono::nanoseconds next_ping_ = {};
};

}  // namespace bitget
}  // namespace roq
