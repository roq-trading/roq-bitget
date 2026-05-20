/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/gateway/api.hpp"

#include "roq/logging.hpp"

#include "roq/utils/enum.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace gateway {

// === IMPLEMENTATION ===

API API::create(Settings const &settings) {
  auto helper = [&](auto const &category, auto const &inst_type) -> API {
    return {
        .category = category,
        .inst_type = inst_type,
        .market_data{
            .instruments = "/api/v3/market/instruments"sv,
        },
        .order_management{
            .account_settings = "/api/v3/account/settings"sv,
            .account_assets = "/api/v3/account/assets"sv,
            .current_positions = "/api/v3/position/current-position"sv,
            .unfilled_orders = "/api/v3/trade/unfilled-orders"sv,
            .trade_fills = "/api/v3/trade/fills"sv,
            .place_order = "/api/v3/trade/place-order"sv,
            .modify_order = "/api/v3/trade/modify-order"sv,
            .cancel_order = "/api/v3/trade/cancel-order"sv,
            .cancel_all_orders = "/api/v3/trade/cancel-symbol-order"sv,
            .countdown_cancel_all = "/api/v3/trade/countdown-cancel-all"sv,
        },
    };
  };
  auto key = parse_api(settings);
  switch (key) {
    using enum Key;
    case SPOT:
      return helper("SPOT"sv, "spot"sv);
    case MARGIN:
      return helper("MARGIN"sv, "margin"sv);
    case USDT_FUTURES:
      return helper("USDT-FUTURES"sv, "usdt-futures"sv);
    case USDC_FUTURES:
      return helper("USDC-FUTURES"sv, "usdc-futures"sv);
    case COIN_FUTURES:
      return helper("COIN-FUTURES"sv, "coin-futures"sv);
  }
  log::fatal("Unexpected"sv);
}

API::Key API::parse_api(Settings const &settings) {
  std::string tmp{settings.api};
  std::replace(tmp.begin(), tmp.end(), '-', '_');
  return utils::parse_enum<Key>(tmp);
}

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
