/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/modify_order_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_modify_order_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758192123365,)"
                 R"("data":{)"
                 R"("orderId":"1352521235708403742",)"
                 R"("clientOid":"JQAClCOGZDQAAQAAAAAA")"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::ModifyOrderAck obj{message, buffer};
}

TEST_CASE("less_than_the_minimum_amount", "[json_modify_order_ack]") {
  auto message = R"({)"
                 R"("code":"45110",)"
                 R"("msg":"less than the minimum amount 5 USDT",)"
                 R"("requestTime":1758192642497,)"
                 R"("data":null)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::ModifyOrderAck obj{message, buffer};
}
