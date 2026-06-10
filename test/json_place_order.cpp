/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::PlaceOrder;

TEST_CASE("simple", "[json_place_order]") {
  auto message = R"({)"
                 R"("event":"trade",)"
                 R"("id":"P:3wAC11XFkEwAAQAAAAAA",)"
                 R"("category":"usdt-futures",)"
                 R"("topic":"place-order",)"
                 R"("args":[{)"
                 R"("symbol":"BTCUSDT",)"
                 R"("orderId":"1396067064152686608",)"
                 R"("clientOid":"3wAC11XFkEwAAQAAAAAA",)"
                 R"("cTime":"1768574246443")"
                 R"(})"
                 R"(],)"
                 R"("code":"0",)"
                 R"("msg":"Success",)"
                 R"("ts":"1768574246444")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event == protocol::json::Event::TRADE);
    CHECK(obj.id == "P:3wAC11XFkEwAAQAAAAAA"sv);
    CHECK(obj.topic == protocol::json::WSAPITopic::PLACE_ORDER);
    REQUIRE(std::size(obj.args) == 1);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
