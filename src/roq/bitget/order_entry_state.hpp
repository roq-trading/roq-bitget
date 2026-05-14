/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace bitget {

enum class OrderEntryState : uint8_t {
  UNDEFINED = 0,
  ACCOUNT_SETTINGS,
  ACCOUNT_ASSETS,
  CURRENT_POSITIONS,
  UNFILLED_ORDERS,
  TRADE_FILLS,
  DONE,
};

}  // namespace bitget
}  // namespace roq
