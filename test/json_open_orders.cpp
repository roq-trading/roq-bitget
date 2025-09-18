/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/open_orders.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_open_orders_empty", "[json_open_orders]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758166069242,)"
                 R"("data":{)"
                 R"("list":[],)"
                 R"("cursor":null)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::OpenOrders obj{message, buffer};
  REQUIRE(std::size(obj.data.list) == 0);
}
