/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/fill.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("snapshot", "[json_fill]") {
  auto message = R"({)"
                 R"("action":"snapshot",)"
                 R"("arg":{)"
                 R"("instType":"UTA",)"
                 R"("topic":"fill")"
                 R"(},)"
                 R"("data":[{)"
                 R"("orderId":"1352492639547711500",)"
                 R"("clientOid":"TQACZE92YDQAAQAAAAAA",)"
                 R"("execId":"1352492639559331841",)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("orderType":"limit",)"
                 R"("side":"sell",)"
                 R"("holdSide":"short",)"
                 R"("execQty":"0.0002",)"
                 R"("execValue":"23.4132",)"
                 R"("execPrice":"117066.1",)"
                 R"("execPnl":"0",)"
                 R"("tradeScope":"taker",)"
                 R"("feeDetail":[{)"
                 R"("feeCoin":"USDT",)"
                 R"("fee":"0.00819462")"
                 R"(})"
                 R"(],)"
                 R"("execTime":"1758185294046",)"
                 R"("updatedTime":"1758185294053",)"
                 R"("tradeSide":"open",)"
                 R"("execLinkId":"1352492639559331841")"
                 R"(})"
                 R"(],)"
                 R"("ts":1758185294056)"
                 R"(})";
  core::json::BufferStack buffer{8192, 2};
  json::Fill obj{message, buffer};
  auto &data = obj.data;
  REQUIRE(std::size(data) == 1);
}
