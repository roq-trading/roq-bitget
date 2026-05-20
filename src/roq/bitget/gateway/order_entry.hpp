/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/bitget/gateway/account.hpp"
#include "roq/bitget/gateway/shared.hpp"

#include "roq/bitget/json/account_assets_ack.hpp"
#include "roq/bitget/json/account_settings_ack.hpp"
#include "roq/bitget/json/current_positions_ack.hpp"
#include "roq/bitget/json/trade_fills_ack.hpp"
#include "roq/bitget/json/unfilled_orders_ack.hpp"

#include "roq/bitget/json/cancel_all_orders_ack.hpp"
#include "roq/bitget/json/cancel_order_ack.hpp"
#include "roq/bitget/json/modify_order_ack.hpp"
#include "roq/bitget/json/place_order_ack.hpp"

namespace roq {
namespace bitget {
namespace gateway {

class OrderEntry final : public web::rest::Client::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id, std::string_view const &request_id) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
  };

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  OrderEntry(OrderEntry const &) = delete;

  bool ready() const { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  enum class State {
    UNDEFINED = 0,
    ACCOUNT_SETTINGS,
    ACCOUNT_ASSETS,
    CURRENT_POSITIONS,
    UNFILLED_ORDERS,
    TRADE_FILLS,
    DONE,
  };

  uint32_t download(State);

  // account-settings

  void get_account_settings();
  void get_account_settings_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::AccountSettingsAck> const &);

  // account-assets

  void get_account_assets();
  void get_account_assets_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::AccountAssetsAck> const &);

  // current-positions

  void get_current_positions();
  void get_current_positions_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::CurrentPositionsAck> const &);

  // unfilled-orders

  void get_unfilled_orders();
  void get_unfilled_orders_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::UnfilledOrdersAck> const &);

  // trade-fills

  void get_trade_fills();
  void get_trade_fills_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::TradeFillsAck> const &);

  // place-order

  void place_order(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  void place_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::PlaceOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // modify-order

  void modify_order(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void modify_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::ModifyOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-order

  void cancel_order(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::CancelOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-all-orders

  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_orders_ack(Trace<web::rest::Response> const &, uint8_t user_id);
  void operator()(Trace<json::CancelAllOrdersAck> const &, uint8_t user_id);

  // countdown-cancel-all

  void countdown_cancel_all();
  void countdown_cancel_all_ack(Trace<web::rest::Response> const &);
  // void operator()(Trace<json::CancelAllOrdersAck> const &, uint8_t user_id);

  // helpers

  void process_response(web::rest::Response const &, auto error_handler, auto success_handler);

  template <typename... Args>
  void operator()(Trace<server::oms::Response> const &, uint8_t user_id, uint64_t order_id, Args &&...);

  void operator()(Trace<server::oms::OrderUpdate> const &, std::string_view const &client_order_id);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // connection
  std::unique_ptr<web::rest::Client> connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile account_settings, account_settings_ack,  //
        account_assets, account_assets_ack,                          //
        current_positions, current_positions_ack,                    //
        unfilled_orders, unfilled_orders_ack,                        //
        trade_fills, trade_fills_ack,                                //
        place_order, place_order_ack,                                //
        modify_order, modify_order_ack,                              //
        cancel_order, cancel_order_ack,                              //
        cancel_all_orders, cancel_all_orders_ack,                    //
        countdown_cancel_all, countdown_cancel_all_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // account
  Account &account_;
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download<State> download_;
  //
  std::string encode_buffer_;
  std::chrono::nanoseconds next_heartbeat_ = {};
};

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
