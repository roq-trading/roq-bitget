/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/cancel_all_orders.hpp"
#include "roq/cancel_order.hpp"
#include "roq/create_order.hpp"
#include "roq/modify_order.hpp"

#include "roq/request_type.hpp"

#include "roq/server/oms/order.hpp"

namespace roq {
namespace bitget {
namespace json {

struct Encoder final {
  // REST

  static std::string_view place_order(
      std::string &buffer, CreateOrder const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &category);

  static std::string_view modify_order(std::string &buffer, ModifyOrder const &, server::oms::Order const &, std::string_view const &request_id);

  static std::string_view cancel_order(std::string &buffer, CancelOrder const &, server::oms::Order const &, std::string_view const &request_id);

  static std::string_view cancel_all_orders(std::string &buffer, CancelAllOrders const &, std::string_view const &request_id, std::string_view const &category);

  static std::string_view countdown_cancel_all(std::string &buffer, std::chrono::seconds countdown);

  // WS

  static std::string_view place_order_ws(
      std::string &buffer, CreateOrder const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &category);

  static std::string_view modify_order_ws(
      std::string &buffer,
      ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      std::string_view const &category);

  static std::string_view cancel_order_ws(
      std::string &buffer, CancelOrder const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);

  static std::pair<RequestType, std::string_view> parse_id(std::string_view const &id);
};

}  // namespace json
}  // namespace bitget
}  // namespace roq
