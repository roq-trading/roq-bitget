/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Fill;

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
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::SNAPSHOT);
    REQUIRE(std::size(obj.data) == 1);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
