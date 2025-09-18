/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget/json/encoder.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

// client_order_id

TEST_CASE("client_order_id_quantity", "[json_modify_order]") {
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

TEST_CASE("client_order_id_price", "[json_modify_order]") {
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

TEST_CASE("order_id_quantity", "[json_modify_order]") {
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

TEST_CASE("order_id_price", "[json_modify_order]") {
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
