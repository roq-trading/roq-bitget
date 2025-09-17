/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget/json/encoder.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

TEST_CASE("cancel_all_orders_simple", "[json_cancel_all_orders]") {
  std::string buffer;
  auto cancel_all_orders = CancelAllOrders{
      .account = {},
      .order_id = {},
      .exchange = {},
      .symbol = {},
      .strategy_id = {},
      .side = {},
  };
  auto request_id = "1234"sv;
  auto inst_type = "USDT-FUTURES"sv;
  auto margin_coin = "USDT";  // why?
  auto message = json::Encoder::cancel_all_orders(buffer, cancel_all_orders, request_id, {}, inst_type, margin_coin);
  CHECK(
      message == R"({)"
                 R"("productType":"USDT-FUTURES",)"
                 R"("marginCoin":"USDT")"
                 R"(})"sv);
}
