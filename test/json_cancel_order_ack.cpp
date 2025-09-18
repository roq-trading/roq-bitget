/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/cancel_order_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

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
  core::json::BufferStack buffer{8192, 1};
  json::CancelOrderAck obj{message, buffer};
}

TEST_CASE("order_does_not_exist", "[json_cancel_order_ack]") {
  auto message = R"({)"
                 R"("code":"25204",)"
                 R"("msg":"Order does not exist",)"
                 R"("requestTime":1758192130006,)"
                 R"("data":null)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::CancelOrderAck obj{message, buffer};
}
