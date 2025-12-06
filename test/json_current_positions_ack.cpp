/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/current_positions_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::CurrentPositionsAck;

TEST_CASE("empty", "[json_current_positions_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758164734988,)"
                 R"("data":{)"
                 R"("list":null)"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 0);
    REQUIRE(obj.msg == "success"sv);
    REQUIRE(std::size(obj.data.list) == 0);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("simple", "[json_current_positions_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758185566985,)"
                 R"("data":{)"
                 R"("list":[{)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("marginCoin":"USDT",)"
                 R"("holdMode":"hedge_mode",)"
                 R"("posSide":"short",)"
                 R"("marginMode":"crossed",)"
                 R"("positionBalance":"2.35132493",)"
                 R"("available":"0.0002",)"
                 R"("frozen":"0",)"
                 R"("total":"0.0002",)"
                 R"("leverage":"10",)"
                 R"("curRealisedPnl":"0",)"
                 R"("avgPrice":"117066.1",)"
                 R"("positionStatus":"normal",)"
                 R"("unrealisedPnl":"-0.01802",)"
                 R"("liquidationPrice":"3271910",)"
                 R"("mmr":"0.004",)"
                 R"("profitRate":"-0.0076637642760841",)"
                 R"("markPrice":"117156.3",)"
                 R"("breakEvenPrice":"116984.1",)"
                 R"("totalFunding":"0",)"
                 R"("openFeeTotal":"-0.00819462",)"
                 R"("closeFeeTotal":"0",)"
                 R"("createdTime":"1758057734271",)"
                 R"("updatedTime":"1758185294053")"
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
