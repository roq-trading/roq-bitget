/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget/json/encoder.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

TEST_CASE("modify_order_simple", "[json_modify_order]") {
  std::string buffer;
  auto modify_order = ModifyOrder{
      .account = {},
      .order_id = {},
      .request_template = {},
      .quantity = NaN,
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
  auto inst_type = "USDT-FUTURES"sv;
  auto margin_coin = "USDT";  // why?
  auto message = json::Encoder::modify_order(buffer, modify_order, order, request_id, {}, inst_type, margin_coin);
  CHECK(
      message == R"({)"
                 R"("symbol":"BTCUSDT",)"
                 R"("productType":"USDT-FUTURES",)"
                 R"("marginCoin":"USDT",)"
                 R"("clientOid":"1234")"
                 R"(})"sv);
}
