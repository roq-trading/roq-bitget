/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/protocol/json/cancel_order_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::CancelOrderAck;

TEST_CASE("simple", "[json_cancel_order_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758192128217,)"
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

TEST_CASE("order_does_not_exist", "[json_cancel_order_ack]") {
  auto message = R"({)"
                 R"("code":"25204",)"
                 R"("msg":"Order does not exist",)"
                 R"("requestTime":1758192130006,)"
                 R"("data":null)"
                 R"(})";
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 25204);
    REQUIRE(obj.msg == "Order does not exist"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
