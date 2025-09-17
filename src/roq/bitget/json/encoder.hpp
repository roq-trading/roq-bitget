/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/cancel_all_orders.hpp"
#include "roq/cancel_order.hpp"
#include "roq/create_order.hpp"
#include "roq/modify_order.hpp"

#include "roq/server/oms/order.hpp"

namespace roq {
namespace bitget {
namespace json {

struct Encoder final {
  static std::string_view place_order(
      std::string &buffer,
      CreateOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::chrono::milliseconds recv_window,
      std::string_view const &inst_type,
      std::string_view const &margin_coin);

  static std::string_view modify_order(
      std::string &buffer,
      ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::chrono::milliseconds recv_window,
      std::string_view const &inst_type,
      std::string_view const &margin_coin);

  static std::string_view cancel_order(
      std::string &buffer,
      CancelOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::chrono::milliseconds recv_window,
      std::string_view const &inst_type,
      std::string_view const &margin_coin);

  static std::string_view cancel_all_orders(
      std::string &buffer,
      CancelAllOrders const &,
      std::string_view const &request_id,
      std::chrono::milliseconds recv_window,
      std::string_view const &inst_type,
      std::string_view const &margin_coin);
};

}  // namespace json
}  // namespace bitget
}  // namespace roq
