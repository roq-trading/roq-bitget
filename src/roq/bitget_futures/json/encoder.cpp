/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget_futures/json/encoder.hpp"

#include <fmt/format.h>

#include "roq/decimal.hpp"

#include "roq/bitget_futures/json/map.hpp"
#include "roq/bitget_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget_futures {
namespace json {

// Force POST_ONLY
// reduceOnly
// stpMode
std::string_view Encoder::place_order(
    std::string &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::chrono::milliseconds recv_window,
    std::string_view const &inst_type,
    std::string_view const &margin_coin) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("symbol":"{}",)"
      R"("productType":"{}",)"
      R"("size":"{}",)"
      R"("price":"{}",)"
      R"("side":"{}",)"
      R"("orderType":"{}",)"
      R"("force":"{}",)"sv,
      create_order.symbol,
      inst_type,
      Decimal{create_order.quantity, order.quantity_precision.precision},
      Decimal{create_order.price, order.price_precision.precision},
      map(create_order.side).template get<json::Side>().as_raw_text(),
      map(create_order.order_type).template get<json::OrderType>().as_raw_text(),
      map(create_order.time_in_force).template get<json::Force>().as_raw_text());
  if (create_order.margin_mode != roq::MarginMode{}) {
    fmt::format_to(std::back_inserter(buffer), R"("marginMode":"{}",)"sv, map(create_order.margin_mode).template get<json::MarginMode>().as_raw_text());
  }
  if (create_order.position_effect != roq::PositionEffect{}) {
    fmt::format_to(std::back_inserter(buffer), R"("tradeSide":"{}",)"sv, map(create_order.position_effect).template get<json::TradeSide>().as_raw_text());
  }
  if (!std::empty(margin_coin)) {
    fmt::format_to(
        std::back_inserter(buffer),
        R"("marginCoin":"{}",)"sv,  // ???
        margin_coin);
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"("clientOid":"{}")"
      R"(}})"sv,
      request_id);
  return buffer;
}

std::string_view Encoder::modify_order(
    std::string &buffer,
    ModifyOrder const &,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::chrono::milliseconds recv_window,
    std::string_view const &inst_type,
    std::string_view const &margin_coin) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("symbol":"{}",)"
      R"("productType":"{}",)"
      R"("marginCoin":"{}",)"sv,  // ???
      order.symbol,
      inst_type,
      margin_coin);
  if (!std::empty(order.external_order_id)) {
    fmt::format_to(
        std::back_inserter(buffer),
        R"("orderId":"{}")"
        R"(}})"sv,
        order.external_order_id);
  } else {
    fmt::format_to(
        std::back_inserter(buffer),
        R"("clientOid":"{}")"
        R"(}})"sv,
        order.client_order_id);
  }
  return buffer;
}

std::string_view Encoder::cancel_order(
    std::string &buffer,
    CancelOrder const &,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::chrono::milliseconds recv_window,
    std::string_view const &inst_type,
    std::string_view const &margin_coin) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("symbol":"{}",)"
      R"("productType":"{}",)"
      R"("marginCoin":"{}",)"sv,  // ???
      order.symbol,
      inst_type,
      margin_coin);
  if (!std::empty(order.external_order_id)) {
    fmt::format_to(
        std::back_inserter(buffer),
        R"("orderId":"{}")"
        R"(}})"sv,
        order.external_order_id);
  } else {
    fmt::format_to(
        std::back_inserter(buffer),
        R"("clientOid":"{}")"
        R"(}})"sv,
        order.client_order_id);
  }
  return buffer;
}

std::string_view Encoder::cancel_all_orders(
    std::string &buffer,
    CancelAllOrders const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::chrono::milliseconds recv_window,
    std::string_view const &inst_type,
    std::string_view const &margin_coin) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("productType":"{}",)"
      R"("marginCoin":"{}")"  // ???
      R"(}})"sv,
      inst_type,
      margin_coin);
  return buffer;
}

}  // namespace json
}  // namespace bitget_futures
}  // namespace roq
