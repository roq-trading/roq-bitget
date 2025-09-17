/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/bitget_futures/settings.hpp"

namespace roq {
namespace bitget_futures {

struct API final {
  std::string_view inst_type;
  struct {
    std::string_view currencies;
    std::string_view contracts;
  } market_data;

  struct {
    std::string_view all_accounts;
    std::string_view all_positions;
    std::string_view orders_history;
    std::string_view order_fill_details;
    std::string_view place_order;
    std::string_view modify_order;
    std::string_view cancel_order;
    std::string_view cancel_all_orders;
  } order_management;

  // factory
  static API create(Settings const &);
};

}  // namespace bitget_futures
}  // namespace roq
