/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/compat.hpp"

#include <memory>
#include <string>
#include <vector>

#include "roq/server.hpp"

#include "roq/io/context.hpp"

#include "roq/bitget/gateway/account.hpp"
#include "roq/bitget/gateway/config.hpp"
#include "roq/bitget/gateway/request.hpp"
#include "roq/bitget/gateway/settings.hpp"
#include "roq/bitget/gateway/shared.hpp"

#include "roq/bitget/gateway/drop_copy.hpp"
#include "roq/bitget/gateway/market_data.hpp"
#include "roq/bitget/gateway/order_entry.hpp"
#include "roq/bitget/gateway/rest.hpp"

namespace roq {
namespace bitget {
namespace gateway {

struct Controller final : public Base<Controller>,
                          public server::Handler,
                          public Rest::Handler,
                          public OrderEntry::Handler,
                          public DropCopy::Handler,
                          public MarketData::Handler {
  ROQ_PUBLIC static std::unique_ptr<server::Handler> create(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  ROQ_PUBLIC static uint8_t parse_api(Settings const &);

  Controller(server::Dispatcher &, Settings const &, Config const &, io::Context &);

  Controller(Controller const &) = delete;

 protected:
  friend base_type;

  // server::Handler

  void operator()(Trace<Start> const &) override;
  void operator()(Trace<Stop> const &) override;
  void operator()(Trace<Timer> const &) override;

  void operator()(Trace<Connected> const &, uint8_t user_id) override;
  void operator()(Trace<Disconnected> const &, uint8_t user_id) override;

  void operator()(Trace<Subscribe> const &) override;
  void operator()(Trace<Control> const &) override;

  void operator()(Trace<server::oms::OrderActionBatchBegin> const &) override;
  void operator()(Trace<server::oms::OrderActionBatchCommit> const &) override;
  void operator()(Trace<server::oms::OrderActionBatchRollback> const &) override;

  uint16_t operator()(Trace<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Trace<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Trace<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Trace<CancelAllOrders> const &, std::string_view const &request_id) override;

  uint16_t operator()(Trace<MassQuote> const &) override;

  uint16_t operator()(Trace<CancelQuotes> const &) override;

  void operator()(metrics::Writer &) const override;

  // Rest::Handler

  void operator()(Trace<SymbolsUpdate> const &) override;

  // utilities

  void ensure_symbol_slices(TraceInfo const &, size_t size);

  template <typename... Args>
  void dispatch(Args &&...);

  template <typename... Args>
  static void dispatch_helper(auto &self, Args &&...);

  template <typename Res, typename T, typename... Args>
  Res dispatch_order_action(Trace<T> const &, Args &&...);

  struct DropCopyRR final {
    DropCopyRR(std::vector<std::unique_ptr<DropCopy>> &&);

    server::OrderActionStream &get_next();

    std::vector<std::unique_ptr<DropCopy>> drop_copy;

   private:
    size_t index_ = {};
  };

 private:
  server::Dispatcher &dispatcher_;
  //
  std::string const master_account_;
  //
  io::Context &context_;
  Shared shared_;
  uint16_t stream_id_ = {};
  //
  utils::unordered_map<std::string, std::unique_ptr<Account>> const accounts_;
  utils::unordered_map<std::string, std::unique_ptr<Request>> const requests_;
  //
  std::unique_ptr<Rest> rest_;
  utils::unordered_map<std::string, std::unique_ptr<OrderEntry>> order_entry_;
  utils::unordered_map<std::string, DropCopyRR> drop_copy_;
  std::vector<std::unique_ptr<MarketData>> market_data_;
};

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
