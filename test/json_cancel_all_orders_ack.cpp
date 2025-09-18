/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/cancel_all_orders_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_cancel_all_orders_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758181333885,)"
                 R"("data":{)"
                 R"("list":[{)"
                 R"("orderId":"1352475910750756888",)"
                 R"("clientOid":"0AACLMUXXjQAAQAAAAAA")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::CancelAllOrdersAck obj{message, buffer};
  REQUIRE(std::size(obj.data.list) == 1);
}
