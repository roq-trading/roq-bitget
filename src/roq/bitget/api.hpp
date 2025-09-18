/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/bitget/settings.hpp"

namespace roq {
namespace bitget {

struct API final {
  std::string_view category;
  std::string_view inst_type;
  struct {
    std::string_view instruments;
  } market_data;

  struct {
    std::string_view account_info;
    std::string_view account_assets;
    std::string_view position_info;
    std::string_view open_orders;
    std::string_view fill_history;
    std::string_view place_order;
    std::string_view modify_order;
    std::string_view cancel_order;
    std::string_view cancel_all_orders;
    std::string_view countdown_cancel_all;
  } order_management;

  // factory
  static API create(Settings const &);
};

}  // namespace bitget
}  // namespace roq
