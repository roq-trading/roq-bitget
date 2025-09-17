/* Copyright (c) 2017-2025, Hans Erik Thrane */

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

#include "roq/bitget/account.hpp"
#include "roq/bitget/order_entry_state.hpp"
#include "roq/bitget/shared.hpp"

#include "roq/bitget/json/all_accounts.hpp"
#include "roq/bitget/json/all_positions.hpp"
#include "roq/bitget/json/order_fill_details.hpp"
#include "roq/bitget/json/orders_history.hpp"

#include "roq/bitget/json/cancel_all_orders_ack.hpp"
#include "roq/bitget/json/cancel_order_ack.hpp"
#include "roq/bitget/json/modify_order_ack.hpp"
#include "roq/bitget/json/place_order_ack.hpp"

namespace roq {
namespace bitget {

class OrderEntry final : public web::rest::Client::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
  };

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  OrderEntry(OrderEntry const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id);
  uint16_t operator()(Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  uint16_t operator()(Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

 protected:
  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;

  void operator()(ConnectionStatus);

  uint32_t download(OrderEntryState state);

  // all_accounts
  void get_all_accounts();
  void get_all_accounts_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::AllAccounts> const &);

  // all_positions
  void get_all_positions();
  void get_all_positions_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::AllPositions> const &);

  // orders_history
  void get_orders_history();
  void get_orders_history_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::OrdersHistory> const &);

  // order_fill_details
  void get_order_fill_details();
  void get_order_fill_details_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::OrderFillDetails> const &);

  // place_order
  void place_order(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id);
  void place_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::PlaceOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // modify_order
  void modify_order(Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  void modify_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::ModifyOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel_order
  void cancel_order(Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::CancelOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel_all_orders
  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_orders_ack(Trace<web::rest::Response> const &, uint8_t user_id);
  void operator()(Trace<json::CancelAllOrdersAck> const &, uint8_t user_id);

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
    utils::metrics::Profile all_accounts, all_accounts_ack,  //
        all_positions, all_positions_ack,                    //
        orders_history, orders_history_ack,                  //
        order_fill_details, order_fill_details_ack,          //
        place_order, place_order_ack,                        //
        modify_order, modify_order_ack,                      //
        cancel_order, cancel_order_ack,                      //
        cancel_all_orders, cancel_all_orders_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // account
  Account &account_;
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
  core::Download<OrderEntryState> download_;
  //
  std::string encode_buffer_;
  utils::unordered_map<std::string, MarginMode> margin_coins_;
};

}  // namespace bitget
}  // namespace roq
