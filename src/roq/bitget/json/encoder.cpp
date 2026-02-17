/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/json/encoder.hpp"

#include <fmt/format.h>

#include "roq/decimal.hpp"

#include "roq/bitget/json/map.hpp"
#include "roq/bitget/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace json {

// REST

// TimeInForce POST_ONLY
// reduceOnly
// stpMode
std::string_view Encoder::place_order(
    std::string &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &category) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("category":"{}",)"
      R"("symbol":"{}",)"
      R"("qty":"{}",)"sv,
      category,
      create_order.symbol,
      Decimal{create_order.quantity, ref_data.quantity.precision});
  if (create_order.order_type == roq::OrderType::LIMIT) {
    auto time_in_force = [&]() -> json::TimeInForce {
      if (create_order.execution_instructions.has(ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE)) {
        return json::TimeInForce::POST_ONLY;
      }
      return map(create_order.time_in_force).template get<json::TimeInForce>();
    }();
    fmt::format_to(
        std::back_inserter(buffer),
        R"("price":"{}",)"
        R"("timeInForce":"{}",)"sv,
        Decimal{create_order.price, ref_data.price.precision},
        time_in_force.as_raw_text());
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"("side":"{}",)"
      R"("orderType":"{}",)"sv,
      map(create_order.side).template get<json::Side>().as_raw_text(),
      map(create_order.order_type).template get<json::OrderType>().as_raw_text());
  if (create_order.position_effect != roq::PositionEffect{}) {
    fmt::format_to(
        std::back_inserter(buffer), R"("posSide":"{}",)"sv, map(create_order.position_effect, create_order.side).template get<json::PosSide>().as_raw_text());
  }
  fmt::format_to(std::back_inserter(buffer), R"("clientOid":"{}",)"sv, request_id);
  fmt::format_to(
      std::back_inserter(buffer),
      R"("reduceOnly":"no",)"
      R"("stpMode":"none")"
      R"(}})"sv);
  return buffer;
}

std::string_view Encoder::modify_order(
    std::string &buffer,
    ModifyOrder const &modify_order,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    [[maybe_unused]] std::string_view const &request_id) {
  buffer.clear();
  if (std::empty(order.external_order_id)) {
    fmt::format_to(
        std::back_inserter(buffer),
        R"({{)"
        R"("clientOid":"{}")"sv,
        order.client_order_id);
  } else {
    fmt::format_to(
        std::back_inserter(buffer),
        R"({{)"
        R"("orderId":"{}")"sv,
        order.external_order_id);
  }
  if (!std::isnan(modify_order.quantity)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"qty":"{}")"sv, Decimal{modify_order.quantity, ref_data.quantity.precision});
  }
  if (!std::isnan(modify_order.price)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"price":"{}")"sv, Decimal{modify_order.price, ref_data.price.precision});
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"(,"autoCancel":"no")"
      R"(}})"sv);
  return buffer;
}

std::string_view Encoder::cancel_order(
    std::string &buffer,
    CancelOrder const &,
    server::oms::Order const &order,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id) {
  buffer.clear();
  if (std::empty(order.external_order_id)) {
    fmt::format_to(
        std::back_inserter(buffer),
        R"({{)"
        R"("clientOid":"{}")"
        R"(}})"sv,
        order.client_order_id);
  } else {
    fmt::format_to(
        std::back_inserter(buffer),
        R"({{)"
        R"("orderId":"{}")"
        R"(}})"sv,
        order.external_order_id);
  }
  return buffer;
}

std::string_view Encoder::cancel_all_orders(
    std::string &buffer, CancelAllOrders const &cancel_all_orders, [[maybe_unused]] std::string_view const &request_id, std::string_view const &category) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("category":"{}")"sv,
      category);
  if (!std::empty(cancel_all_orders.symbol)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"symbol":"{}")"sv, cancel_all_orders.symbol);
  }
  fmt::format_to(std::back_inserter(buffer), R"(}})"sv);
  return buffer;
}

std::string_view Encoder::countdown_cancel_all(std::string &buffer, std::chrono::seconds countdown) {
  buffer.clear();
  int64_t count = countdown.count();
  auto tmp = std::min<int64_t>(std::max<int64_t>(count, 5), 60);  // note! docs say allowed range is [5;60]
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("countdown":"{}")"
      R"(}})"sv,
      tmp);
  return buffer;
}

// WS

std::string_view Encoder::place_order_ws(
    std::string &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &category) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("op":"trade",)"
      R"("id":"P:{}",)"
      R"("category":"{}",)"
      R"("topic":"place-order",)"
      R"("args":[)"sv,
      request_id,
      category);
  //
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("symbol":"{}",)"
      R"("qty":"{}",)"sv,
      create_order.symbol,
      Decimal{create_order.quantity, ref_data.quantity.precision});
  if (create_order.order_type == roq::OrderType::LIMIT) {
    auto time_in_force = [&]() -> json::TimeInForce {
      if (create_order.execution_instructions.has(ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE)) {
        return json::TimeInForce::POST_ONLY;
      }
      return map(create_order.time_in_force).template get<json::TimeInForce>();
    }();
    fmt::format_to(
        std::back_inserter(buffer),
        R"("price":"{}",)"
        R"("timeInForce":"{}",)"sv,
        Decimal{create_order.price, ref_data.price.precision},
        time_in_force.as_raw_text());
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"("side":"{}",)"
      R"("orderType":"{}",)"sv,
      map(create_order.side).template get<json::Side>().as_raw_text(),
      map(create_order.order_type).template get<json::OrderType>().as_raw_text());
  if (create_order.position_effect != roq::PositionEffect{}) {
    fmt::format_to(
        std::back_inserter(buffer), R"("posSide":"{}",)"sv, map(create_order.position_effect, create_order.side).template get<json::PosSide>().as_raw_text());
  }
  fmt::format_to(std::back_inserter(buffer), R"("clientOid":"{}",)"sv, request_id);
  fmt::format_to(
      std::back_inserter(buffer),
      R"("reduceOnly":"NO",)"
      R"("stpMode":"none")"
      R"(}})"sv);
  //
  fmt::format_to(
      std::back_inserter(buffer),
      R"(])"
      R"(}})"sv);
  return buffer;
}

std::string_view Encoder::modify_order_ws(
    std::string &buffer,
    ModifyOrder const &modify_order,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    std::string_view const &category) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"M:{}",)"
      R"("op":"trade",)"
      R"("category":"{}",)"
      R"("topic":"modify-order",)"
      R"("args":[)"sv,
      request_id,
      category);
  //
  if (std::empty(order.external_order_id)) {
    fmt::format_to(
        std::back_inserter(buffer),
        R"({{)"
        R"("clientOid":"{}")"sv,
        order.client_order_id);
  } else {
    fmt::format_to(
        std::back_inserter(buffer),
        R"({{)"
        R"("orderId":"{}")"sv,
        order.external_order_id);
  }
  if (!std::isnan(modify_order.quantity)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"qty":"{}")"sv, Decimal{modify_order.quantity, ref_data.quantity.precision});
  }
  if (!std::isnan(modify_order.price)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"price":"{}")"sv, Decimal{modify_order.price, ref_data.price.precision});
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"(,"autoCancel":"no")"
      R"(}})"sv);
  //
  fmt::format_to(
      std::back_inserter(buffer),
      R"(])"
      R"(}})"sv);
  return buffer;
}

std::string_view Encoder::cancel_order_ws(
    std::string &buffer,
    CancelOrder const &,
    server::oms::Order const &order,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"C:{}",)"
      R"("op":"trade",)"
      R"("topic":"cancel-order",)"
      R"("args":[)"sv,
      request_id);
  //
  if (std::empty(order.external_order_id)) {
    fmt::format_to(
        std::back_inserter(buffer),
        R"({{)"
        R"("clientOid":"{}")"
        R"(}})"sv,
        order.client_order_id);
  } else {
    fmt::format_to(
        std::back_inserter(buffer),
        R"({{)"
        R"("orderId":"{}")"
        R"(}})"sv,
        order.external_order_id);
  }
  //
  fmt::format_to(
      std::back_inserter(buffer),
      R"(])"
      R"(}})"sv);
  return buffer;
}

std::pair<RequestType, std::string_view> Encoder::parse_id(std::string_view const &id) {
  if (std::size(id) > 2 && id[1] == ':') {
    switch (id[0]) {
      case 'P':
        return {RequestType::CREATE_ORDER, id.substr(2)};
      case 'M':
        return {RequestType::MODIFY_ORDER, id.substr(2)};
      case 'C':
        return {RequestType::CANCEL_ORDER, id.substr(2)};
    }
  }
  return {};
}

}  // namespace json
}  // namespace bitget
}  // namespace roq
