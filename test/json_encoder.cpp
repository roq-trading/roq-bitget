/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget/json/encoder.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

// create

TEST_CASE("create_order_market", "[json_encoder]") {
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
      .leverage = NaN,
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

TEST_CASE("create_order_limit", "[json_encoder]") {
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
      .leverage = NaN,
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

TEST_CASE("create_order_limit_post_only", "[json_encoder]") {
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
      .leverage = NaN,
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

// modify
// client_order_id

TEST_CASE("modify_order_client_order_id_quantity", "[json_encoder]") {
  std::string buffer;
  auto modify_order = ModifyOrder{
      .account = {},
      .order_id = {},
      .request_template = {},
      .quantity = 0.1234,
      .price = NaN,
      .routing_id = {},
      .version = {},
      .conditional_on_version = {},
  };
  server::oms::Order order;
  order.symbol = "BTCUSDT"sv;
  order.external_order_id = {};
  order.client_order_id = "1234"sv;
  order.quantity_precision = {
      .increment = 0.0001,
      .precision = Precision::_4,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "2345"sv;
  auto message = json::Encoder::modify_order(buffer, modify_order, order, request_id);
  CHECK(
      message == R"({)"
                 R"("clientOid":"1234",)"
                 R"("qty":"0.1234",)"
                 R"("autoCancel":"no")"
                 R"(})"sv);
}

TEST_CASE("modify_order_client_order_id_price", "[json_encoder]") {
  std::string buffer;
  auto modify_order = ModifyOrder{
      .account = {},
      .order_id = {},
      .request_template = {},
      .quantity = NaN,
      .price = 123.4,
      .routing_id = {},
      .version = {},
      .conditional_on_version = {},
  };
  server::oms::Order order;
  order.symbol = "BTCUSDT"sv;
  order.external_order_id = {};
  order.client_order_id = "1234"sv;
  order.quantity_precision = {
      .increment = 0.0001,
      .precision = Precision::_4,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "2345"sv;
  auto message = json::Encoder::modify_order(buffer, modify_order, order, request_id);
  CHECK(
      message == R"({)"
                 R"("clientOid":"1234",)"
                 R"("price":"123.4",)"
                 R"("autoCancel":"no")"
                 R"(})"sv);
}

// order_id

TEST_CASE("modify_order_order_id_quantity", "[json_encoder]") {
  std::string buffer;
  auto modify_order = ModifyOrder{
      .account = {},
      .order_id = {},
      .request_template = {},
      .quantity = 0.1234,
      .price = NaN,
      .routing_id = {},
      .version = {},
      .conditional_on_version = {},
  };
  server::oms::Order order;
  order.symbol = "BTCUSDT"sv;
  order.external_order_id = "2345"sv;
  order.client_order_id = "1234"sv;
  order.quantity_precision = {
      .increment = 0.0001,
      .precision = Precision::_4,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "2345"sv;
  auto message = json::Encoder::modify_order(buffer, modify_order, order, request_id);
  CHECK(
      message == R"({)"
                 R"("orderId":"2345",)"
                 R"("qty":"0.1234",)"
                 R"("autoCancel":"no")"
                 R"(})"sv);
}

TEST_CASE("modify_order_order_id_price", "[json_encoder]") {
  std::string buffer;
  auto modify_order = ModifyOrder{
      .account = {},
      .order_id = {},
      .request_template = {},
      .quantity = NaN,
      .price = 123.4,
      .routing_id = {},
      .version = {},
      .conditional_on_version = {},
  };
  server::oms::Order order;
  order.symbol = "BTCUSDT"sv;
  order.external_order_id = "2345"sv;
  order.client_order_id = "1234"sv;
  order.quantity_precision = {
      .increment = 0.0001,
      .precision = Precision::_4,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "2345"sv;
  auto message = json::Encoder::modify_order(buffer, modify_order, order, request_id);
  CHECK(
      message == R"({)"
                 R"("orderId":"2345",)"
                 R"("price":"123.4",)"
                 R"("autoCancel":"no")"
                 R"(})"sv);
}

// cancel

TEST_CASE("cancel_order_client_order_id", "[json_encoder]") {
  std::string buffer;
  auto cancel_order = CancelOrder{
      .account = {},
      .order_id = {},
      .request_template = {},
      .routing_id = {},
      .version = {},
      .conditional_on_version = {},
  };
  server::oms::Order order;
  order.symbol = "BTCUSDT"sv;
  order.external_order_id = {};
  order.client_order_id = "1234"sv;
  order.quantity_precision = {
      .increment = 0.0001,
      .precision = Precision::_4,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "2345"sv;
  auto message = json::Encoder::cancel_order(buffer, cancel_order, order, request_id);
  CHECK(
      message == R"({)"
                 R"("clientOid":"1234")"
                 R"(})"sv);
}

TEST_CASE("cancel_order_order_id", "[json_encoder]") {
  std::string buffer;
  auto cancel_order = CancelOrder{
      .account = {},
      .order_id = {},
      .request_template = {},
      .routing_id = {},
      .version = {},
      .conditional_on_version = {},
  };
  server::oms::Order order;
  order.symbol = "BTCUSDT"sv;
  order.external_order_id = "1234"sv, order.client_order_id = "2345"sv;
  order.quantity_precision = {
      .increment = 0.0001,
      .precision = Precision::_4,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "2345"sv;
  auto message = json::Encoder::cancel_order(buffer, cancel_order, order, request_id);
  CHECK(
      message == R"({)"
                 R"("orderId":"1234")"
                 R"(})"sv);
}

// cancel-all

TEST_CASE("cancel_all_orders_simple", "[json_encoder]") {
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

TEST_CASE("cancel_all_orders_symbol", "[json_encoder]") {
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
