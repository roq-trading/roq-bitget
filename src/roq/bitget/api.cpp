/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/api.hpp"

#include "roq/logging.hpp"

#include "roq/utils/enum.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {

// === HELPERS ===

namespace {
enum class Key {
  SPOT,
  MARGIN,
  USDT_FUTURES,
  USDC_FUTURES,
  COIN_FUTURES,
};

auto parse_api(auto &api) {
  std::string tmp{api};
  std::replace(tmp.begin(), tmp.end(), '-', '_');
  return utils::parse_enum<Key>(tmp);
}
}  // namespace

// === IMPLEMENTATION ===

API API::create(Settings const &settings) {
  auto key = parse_api(settings.api);
  switch (key) {
    using enum Key;
    case SPOT:
      return {
          .category = "SPOT"sv,
          .inst_type = "spot"sv,
          .market_data{
              .instruments = "/api/v3/market/instruments?category=SPOT"sv,
          },
          .order_management{
              .account_info = "/api/v3/account/settings"sv,
              .account_assets = "/api/v3/account/assets"sv,
              .position_info = "/api/v3/position/current-position?category=SPOT"sv,
              .open_orders = "/api/v3/trade/unfilled-orders?category=SPOT"sv,
              .fill_history = "/api/v3/trade/fills?category=SPOT"sv,
              .place_order = "/api/v3/trade/place-order?category=SPOT"sv,
              .modify_order = "/api/v3/trade/modify-order?category=SPOT"sv,
              .cancel_order = "/api/v3/trade/cancel-order?category=SPOT"sv,
              .cancel_all_orders = "/api/v3/trade/cancel-symbol-order?category=SPOT"sv,
              .countdown_cancel_all = "/api/v3/trade/countdown-cancel-all"sv,
          },
      };
    case MARGIN:
      return {
          .category = "MARGIN"sv,
          .inst_type = "margin"sv,
          .market_data{
              .instruments = "/api/v3/market/instruments?category=MARGIN"sv,
          },
          .order_management{
              .account_info = "/api/v3/account/settings"sv,
              .account_assets = "/api/v3/account/assets"sv,
              .position_info = "/api/v3/position/current-position?category=MARGIN"sv,
              .open_orders = "/api/v3/trade/unfilled-orders?category=MARGIN"sv,
              .fill_history = "/api/v3/trade/fills?category=MARGIN"sv,
              .place_order = "/api/v3/trade/place-order?category=MARGIN"sv,
              .modify_order = "/api/v3/trade/modify-order?category=MARGIN"sv,
              .cancel_order = "/api/v3/trade/cancel-order?category=MARGIN"sv,
              .cancel_all_orders = "/api/v3/trade/cancel-symbol-order?category=MARGIN"sv,
              .countdown_cancel_all = "/api/v3/trade/countdown-cancel-all"sv,
          },
      };
    case USDT_FUTURES:
      return {
          .category = "USDT-FUTURES"sv,
          .inst_type = "usdt-futures"sv,
          .market_data{
              .instruments = "/api/v3/market/instruments?category=USDT-FUTURES"sv,
          },
          .order_management{
              .account_info = "/api/v3/account/settings"sv,
              .account_assets = "/api/v3/account/assets"sv,
              .position_info = "/api/v3/position/current-position?category=USDT-FUTURES"sv,
              .open_orders = "/api/v3/trade/unfilled-orders?category=USDT-FUTURES"sv,
              .fill_history = "/api/v3/trade/fills?category=USDT-FUTURES"sv,
              .place_order = "/api/v3/trade/place-order?category=USDT-FUTURES"sv,
              .modify_order = "/api/v3/trade/modify-order?category=USDT-FUTURES"sv,
              .cancel_order = "/api/v3/trade/cancel-order?category=USDT-FUTURES"sv,
              .cancel_all_orders = "/api/v3/trade/cancel-symbol-order?category=USDT-FUTURES"sv,
              .countdown_cancel_all = "/api/v3/trade/countdown-cancel-all"sv,
          },
      };
    case USDC_FUTURES:
      return {
          .category = "USDC-FUTURES"sv,
          .inst_type = "usdc-futures"sv,
          .market_data{
              .instruments = "/api/v3/market/instruments?category=USDC-FUTURES"sv,
          },
          .order_management{
              .account_info = "/api/v3/account/settings"sv,
              .account_assets = "/api/v3/account/assets"sv,
              .position_info = "/api/v3/position/current-position?category=USDC-FUTURES"sv,
              .open_orders = "/api/v3/trade/unfilled-orders?category=USDC-FUTURES"sv,
              .fill_history = "/api/v3/trade/fills?category=USDC-FUTURES"sv,
              .place_order = "/api/v3/trade/place-order?category=USDC-FUTURES"sv,
              .modify_order = "/api/v3/trade/modify-order?category=USDC-FUTURES"sv,
              .cancel_order = "/api/v3/trade/cancel-order?category=USDC-FUTURES"sv,
              .cancel_all_orders = "/api/v3/trade/cancel-symbol-order?category=USDC-FUTURES"sv,
              .countdown_cancel_all = "/api/v3/trade/countdown-cancel-all"sv,
          },
      };
    case COIN_FUTURES:
      return {
          .category = "COIN-FUTURES"sv,
          .inst_type = "coin-futures"sv,
          .market_data{
              .instruments = "/api/v3/market/instruments?category=COIN-FUTURES"sv,
          },
          .order_management{
              .account_info = "/api/v3/account/settings"sv,
              .account_assets = "/api/v3/account/assets"sv,
              .position_info = "/api/v3/position/current-position?category=COIN-FUTURES"sv,
              .open_orders = "/api/v3/trade/unfilled-orders?category=COIN-FUTURES"sv,
              .fill_history = "/api/v3/trade/fills?category=COIN-FUTURES"sv,
              .place_order = "/api/v3/trade/place-order?category=COIN-FUTURES"sv,
              .modify_order = "/api/v3/trade/modify-order?category=COIN-FUTURES"sv,
              .cancel_order = "/api/v3/trade/cancel-order?category=COIN-FUTURES"sv,
              .cancel_all_orders = "/api/v3/trade/cancel-symbol-order?category=COIN-FUTURES"sv,
              .countdown_cancel_all = "/api/v3/trade/countdown-cancel-all"sv,
          },
      };
  }
  log::fatal("Unexpected"sv);
}

}  // namespace bitget
}  // namespace roq
