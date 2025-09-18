/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget/json/encoder.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

TEST_CASE("simple", "[json_cancel_all_orders]") {
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
  auto category = "USDT-FUTURES"sv;
  auto message = json::Encoder::cancel_all_orders(buffer, cancel_all_orders, request_id, category);
  CHECK(
      message == R"({)"
                 R"("category":"USDT-FUTURES")"
                 R"(})"sv);
}

TEST_CASE("symbol", "[json_cancel_all_orders]") {
  std::string buffer;
  auto cancel_all_orders = CancelAllOrders{
      .account = {},
      .order_id = {},
      .exchange = {},
      .symbol = "BTCUSDT"sv,
      .strategy_id = {},
      .side = {},
  };
  auto request_id = "1234"sv;
  auto category = "USDT-FUTURES"sv;
  auto message = json::Encoder::cancel_all_orders(buffer, cancel_all_orders, request_id, category);
  CHECK(
      message == R"({)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT")"
                 R"(})"sv);
}
