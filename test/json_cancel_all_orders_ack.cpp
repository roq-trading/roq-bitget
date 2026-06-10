/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/protocol/json/cancel_all_orders_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::CancelAllOrdersAck;

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
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 0);
    REQUIRE(obj.msg == "success"sv);
    REQUIRE(std::size(obj.data.list) == 1);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
