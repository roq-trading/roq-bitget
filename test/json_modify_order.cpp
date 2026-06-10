/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::ModifyOrder;

TEST_CASE("simple", "[json_modify_order]") {
  auto message = R"({)"
                 R"("event":"trade",)"
                 R"("id":"M:oAACpuF5lUwAAgAAAAAA",)"
                 R"("topic":"modify-order",)"
                 R"("args":[{)"
                 R"("orderId":"1396100161334247426",)"
                 R"("clientOid":"owACpuF5lUwAAQAAAAAA")"
                 R"(}],)"
                 R"("code":"0",)"
                 R"("msg":"Success",)"
                 R"("ts":"1768582143239")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.event == protocol::json::Event::TRADE);
    CHECK(obj.id == "M:oAACpuF5lUwAAgAAAAAA"sv);
    CHECK(obj.topic == protocol::json::WSAPITopic::MODIFY_ORDER);
    REQUIRE(std::size(obj.args) == 1);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
