/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/compat.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "roq/server.hpp"

#include "roq/io/context.hpp"

#include "roq/bitget/gateway/account.hpp"
#include "roq/bitget/gateway/config.hpp"
#include "roq/bitget/gateway/settings.hpp"
#include "roq/bitget/gateway/shared.hpp"

#include "roq/bitget/gateway/drop_copy.hpp"
#include "roq/bitget/gateway/market_data.hpp"
#include "roq/bitget/gateway/order_entry.hpp"
#include "roq/bitget/gateway/rest.hpp"

namespace roq {
namespace bitget {
namespace gateway {

struct Controller final : public server::Handler, public Rest::Handler, public OrderEntry::Handler, public DropCopy::Handler, public MarketData::Handler {
  ROQ_PUBLIC static std::unique_ptr<server::Handler> create(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Controller(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Controller(Controller const &) = delete;

  virtual ~Controller() = default;

 protected:
  // server::Handler

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Control> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  void operator()(Event<Subscribe> const &) override;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  uint16_t operator()(Event<MassQuote> const &) override;

  uint16_t operator()(Event<CancelQuotes> const &) override;

  void operator()(metrics::Writer &) const override;

  // streams

  void operator()(Trace<StreamStatus> const &) override;
  void operator()(Trace<ExternalLatency> const &) override;
  void operator()(Trace<ReferenceData> const &, bool is_last) override;
  void operator()(Trace<MarketStatus> const &, bool is_last) override;
  void operator()(Trace<TopOfBook> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) override;
  void operator()(Trace<TradeSummary> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate> const &, bool is_last) override;
  void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id, std::string_view const &request_id) override;
  void operator()(Trace<FundsUpdate> const &, bool is_last) override;
  void operator()(Trace<PositionUpdate> const &, bool is_last) override;

  void operator()(Rest::SymbolsUpdate &) override;

  // utilities

  void ensure_symbol_slices(size_t size);

  template <typename... Args>
  void dispatch(Args &&...);

  template <typename... Args>
  static void dispatch_helper(auto &self, Args &&...);

  OrderEntry &get_order_entry(std::string_view const &account);
  DropCopy &get_drop_copy(std::string_view const &account);

  struct OrderEntryRR final {
    OrderEntryRR(std::vector<std::unique_ptr<OrderEntry>> &&);

    template <typename... Args>
    void operator()(Args &&...);

    template <typename... Args>
    void operator()(Args &&...) const;

    OrderEntry &get_next();

   private:
    std::vector<std::unique_ptr<OrderEntry>> order_entry_;
    size_t index_ = {};
  };

 private:
  server::Dispatcher &dispatcher_;
  // config
  std::string const master_account_;
  // accounts
  utils::unordered_map<std::string, std::unique_ptr<Account>> const accounts_;
  // io
  io::Context &context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  Rest rest_;
  utils::unordered_map<std::string, OrderEntryRR> order_entry_;
  utils::unordered_map<std::string, std::unique_ptr<DropCopy>> drop_copy_;
  std::vector<std::unique_ptr<MarketData>> market_data_;
};

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
