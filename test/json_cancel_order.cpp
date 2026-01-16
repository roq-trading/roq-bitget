/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::CancelOrder;

TEST_CASE("simple", "[json_cancel_order]") {
  auto message = R"({)"
                 R"("event":"trade",)"
                 R"("id":"C:AAAC-qXAlEwAAgAAAAAA",)"
                 R"("topic":"cancel-order",)"
                 R"("args":[{)"
                 R"("orderId":"1396095071110135834",)"
                 R"("clientOid":"AwAC-qXAlEwAAQAAAAAA")"
                 R"(})"
                 R"(],)"
                 R"("code":"0",)"
                 R"("msg":"Success",)"
                 R"("ts":"1768580937683")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event == json::Event::TRADE);
    CHECK(obj.id == "C:AAAC-qXAlEwAAgAAAAAA"sv);
    CHECK(obj.topic == json::WSAPITopic::CANCEL_ORDER);
    REQUIRE(std::size(obj.args) == 1);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
