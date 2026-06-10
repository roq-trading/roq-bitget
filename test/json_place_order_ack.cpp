/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/protocol/json/place_order_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::PlaceOrderAck;

TEST_CASE("simple", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758192111898,)"
                 R"("data":{)"
                 R"("orderId":"1352521235708403742",)"
                 R"("clientOid":"JQAClCOGZDQAAQAAAAAA")"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 0);
    REQUIRE(obj.msg == "success"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("no_position_available_to_close", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("code":"25227",)"
                 R"("msg":"No position available to close",)"
                 R"("requestTime":1758192088206,)"
                 R"("data":null)"
                 R"(})";
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 25227);
    REQUIRE(obj.msg == "No position available to close"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("less_than_the_minimum_amount", "[json_place_order_ack]") {
  auto message = R"({)"
                 R"("code":"45110",)"
                 R"("msg":"less than the minimum amount 5 USDT",)"
                 R"("requestTime":1758192103376,)"
                 R"("data":null)"
                 R"(})";
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 45110);
    REQUIRE(obj.msg == "less than the minimum amount 5 USDT"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
