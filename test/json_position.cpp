/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/position.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("snapshot", "[json_position]") {
  auto message = R"({)"
                 R"("action":"snapshot",)"
                 R"("arg":{)"
                 R"("instType":"UTA",)"
                 R"("topic":"position")"
                 R"(},)"
                 R"("data":[{)"
                 R"("symbol":"BTCUSDT",)"
                 R"("marginCoin":"USDT",)"
                 R"("marginSize":"2.35064255",)"
                 R"("marginMode":"crossed",)"
                 R"("holdMode":"hedge_mode",)"
                 R"("posSide":"short",)"
                 R"("size":"0.0002",)"
                 R"("available":"0.0002",)"
                 R"("frozen":"0",)"
                 R"("avgPrice":"117066.1",)"
                 R"("leverage":"10",)"
                 R"("curRealisedPnl":"0",)"
                 R"("unrealisedPnl":"-0.01122",)"
                 R"("liqPrice":"3271394.4",)"
                 R"("mmr":"0.004",)"
                 R"("markPrice":"117122.2",)"
                 R"("breakEvenPrice":"116984.1",)"
                 R"("profitRate":"-0.0047731629804795",)"
                 R"("totalFundingFee":"0",)"
                 R"("openFeeTotal":"-0.00819462",)"
                 R"("closeFeeTotal":"0",)"
                 R"("positionStatus":"opening",)"
                 R"("createdTime":"1758057734271",)"
                 R"("updatedTime":"1758185294053")"
                 R"(})"
                 R"(],)"
                 R"("ts":1758186381497)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::Position obj{message, buffer};
  REQUIRE(std::size(obj.data) == 1);
}
