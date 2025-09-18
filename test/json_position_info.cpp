/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/position_info.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_position_info_empty", "[json_position_info]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758164734988,)"
                 R"("data":{)"
                 R"("list":null)"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::PositionInfo obj{message, buffer};
  REQUIRE(std::size(obj.data.list) == 0);
}
