/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget/json/encoder.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

TEST_CASE("market", "[json_place_order]") {
  std::string buffer;
  auto create_order = CreateOrder{
      .account = {},
      .order_id = {},
      .exchange = {},
      .symbol = "BTCUSDT"sv,
      .side = Side::BUY,
      .position_effect = {},
      .margin_mode = MarginMode::ISOLATED,
      .quantity_type = {},
      .max_show_quantity = NaN,
      .order_type = OrderType::MARKET,
      .time_in_force = TimeInForce::GTC,
      .execution_instructions = {},
      .request_template = {},
      .quantity = 0.001,
      .price = NaN,
      .stop_price = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  order.quantity_precision = {
      .increment = 0.0001,
      .precision = Precision::_4,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "1234"sv;
  auto category = "USDT-FUTURES"sv;
  auto message = json::Encoder::place_order(buffer, create_order, order, request_id, category);
  CHECK(
      message == R"({)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("qty":"0.0010",)"
                 R"("side":"buy",)"
                 R"("orderType":"market",)"
                 R"("clientOid":"1234",)"
                 R"("reduceOnly":"no",)"
                 R"("stpMode":"none")"
                 R"(})"sv);
}

TEST_CASE("limit", "[json_place_order]") {
  std::string buffer;
  auto create_order = CreateOrder{
      .account = {},
      .order_id = {},
      .exchange = {},
      .symbol = "BTCUSDT"sv,
      .side = Side::BUY,
      .position_effect = {},
      .margin_mode = MarginMode::ISOLATED,
      .quantity_type = {},
      .max_show_quantity = NaN,
      .order_type = OrderType::LIMIT,
      .time_in_force = TimeInForce::GTC,
      .execution_instructions = {},
      .request_template = {},
      .quantity = 0.001,
      .price = 32000.0,
      .stop_price = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  order.quantity_precision = {
      .increment = 0.0001,
      .precision = Precision::_4,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "1234"sv;
  auto category = "USDT-FUTURES"sv;
  auto message = json::Encoder::place_order(buffer, create_order, order, request_id, category);
  CHECK(
      message == R"({)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("qty":"0.0010",)"
                 R"("price":"32000.0",)"
                 R"("timeInForce":"gtc",)"
                 R"("side":"buy",)"
                 R"("orderType":"limit",)"
                 R"("clientOid":"1234",)"
                 R"("reduceOnly":"no",)"
                 R"("stpMode":"none")"
                 R"(})"sv);
}

TEST_CASE("limit_post_only", "[json_place_order]") {
  std::string buffer;
  auto create_order = CreateOrder{
      .account = {},
      .order_id = {},
      .exchange = {},
      .symbol = "BTCUSDT"sv,
      .side = Side::BUY,
      .position_effect = {},
      .margin_mode = MarginMode::ISOLATED,
      .quantity_type = {},
      .max_show_quantity = NaN,
      .order_type = OrderType::LIMIT,
      .time_in_force = TimeInForce::GTC,
      .execution_instructions = {ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE},
      .request_template = {},
      .quantity = 0.001,
      .price = 32000.0,
      .stop_price = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  order.quantity_precision = {
      .increment = 0.0001,
      .precision = Precision::_4,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "1234"sv;
  auto category = "USDT-FUTURES"sv;
  auto message = json::Encoder::place_order(buffer, create_order, order, request_id, category);
  CHECK(
      message == R"({)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("qty":"0.0010",)"
                 R"("price":"32000.0",)"
                 R"("timeInForce":"post_only",)"
                 R"("side":"buy",)"
                 R"("orderType":"limit",)"
                 R"("clientOid":"1234",)"
                 R"("reduceOnly":"no",)"
                 R"("stpMode":"none")"
                 R"(})"sv);
}
