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
    case USDT_FUTURES:
      return {
          .inst_type = "USDT-FUTURES"sv,
          .market_data{
              .currencies = "/api/v2/spot/public/coins"sv,
              .contracts = "/api/v2/mix/market/contracts?productType=USDT-FUTURES"sv,
          },
          .order_management{
              .all_accounts = "/api/v2/mix/account/accounts?productType=USDT-FUTURES"sv,
              .all_positions = "/api/v2/mix/position/all-position?productType=USDT-FUTURES"sv,
              .orders_history = "/api/v2/mix/order/orders-history?productType=USDT-FUTURES"sv,
              .order_fill_details = "/api/v2/mix/order/fills?productType=USDT-FUTURES"sv,
              .place_order = "/api/v2/mix/order/place-order"sv,
              .modify_order = "/api/v2/mix/order/modify-order"sv,
              .cancel_order = "/api/v2/mix/order/cancel-order"sv,
              .cancel_all_orders = "/api/v2/mix/order/cancel-all-orders"sv,
          },
      };
    case USDC_FUTURES:
      return {
          .inst_type = "USDC-FUTURES"sv,
          .market_data{
              .currencies = "/api/v2/spot/public/coins"sv,
              .contracts = "/api/v2/mix/market/contracts?productType=USDC-FUTURES"sv,
          },
          .order_management{
              .all_accounts = "/api/v2/mix/account/accounts?productType=USDC-FUTURES"sv,
              .all_positions = "/api/v2/mix/position/all-position?productType=USDC-FUTURES"sv,
              .orders_history = "/api/v2/mix/order/orders-history?productType=USDC-FUTURES"sv,
              .order_fill_details = "/api/v2/mix/order/fills?productType=USDC-FUTURES"sv,
              .place_order = "/api/v2/mix/order/place-order"sv,
              .modify_order = "/api/v2/mix/order/modify-order"sv,
              .cancel_order = "/api/v2/mix/order/cancel-order"sv,
              .cancel_all_orders = "/api/v2/mix/order/cancel-all-orders"sv,
          },
      };
    case COIN_FUTURES:
      return {
          .inst_type = "COIN-FUTURES"sv,
          .market_data{
              .currencies = "/api/v2/spot/public/coins"sv,
              .contracts = "/api/v2/mix/market/contracts?productType=COIN-FUTURES"sv,
          },
          .order_management{
              .all_accounts = "/api/v2/mix/account/accounts?productType=COIN-FUTURES"sv,
              .all_positions = "/api/v2/mix/position/all-position?productType=COIN-FUTURES"sv,
              .orders_history = "/api/v2/mix/order/orders-history?productType=COIN-FUTURES"sv,
              .order_fill_details = "/api/v2/mix/order/fills?productType=COIN-FUTURES"sv,
              .place_order = "/api/v2/mix/order/place-order"sv,
              .modify_order = "/api/v2/mix/order/modify-order"sv,
              .cancel_order = "/api/v2/mix/order/cancel-order"sv,
              .cancel_all_orders = "/api/v2/mix/order/cancel-all-orders"sv,
          },
      };
  }
  log::fatal("Unexpected"sv);
}

}  // namespace bitget
}  // namespace roq
