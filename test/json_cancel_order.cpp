/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget/json/encoder.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

TEST_CASE("client_order_id", "[json_cancel_order]") {
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

TEST_CASE("order_id", "[json_cancel_order]") {
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
