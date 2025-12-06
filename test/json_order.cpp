/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Order;

TEST_CASE("snapshot", "[json_order]") {
  auto message = R"({)"
                 R"("action":"snapshot",)"
                 R"("arg":{)"
                 R"("instType":"UTA",)"
                 R"("topic":"order"},)"
                 R"("data":[{)"
                 R"("category":"usdt-futures",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("orderId":"1352473539865894955",)"
                 R"("clientOid":"FQACwTnAXTQAAQAAAAAA",)"
                 R"("price":"32000",)"
                 R"("qty":"0.0002",)"
                 R"("holdMode":"hedge_mode",)"
                 R"("holdSide":"long",)"
                 R"("tradeSide":"open",)"
                 R"("orderType":"limit",)"
                 R"("timeInForce":"gtc",)"
                 R"("side":"buy",)"
                 R"("marginMode":"crossed",)"
                 R"("marginCoin":"USDT",)"
                 R"("reduceOnly":"no",)"
                 R"("cumExecQty":"0",)"
                 R"("cumExecValue":"0",)"
                 R"("avgPrice":"0",)"
                 R"("totalProfit":"0",)"
                 R"("orderStatus":"live",)"
                 R"("cancelReason":"",)"
                 R"("leverage":"10",)"
                 R"("feeDetail":[],)"
                 R"("createdTime":"1758180740324",)"
                 R"("updatedTime":"1758180740324",)"
                 R"("stpMode":"none",)"
                 R"("tpTriggerBy":"",)"
                 R"("slTriggerBy":"",)"
                 R"("takeprofit":"",)"
                 R"("stoploss":"",)"
                 R"("tpOrderType":"market",)"
                 R"("slOrderType":"market",)"
                 R"("tpLimitPrice":"",)"
                 R"("slLimitPrice":"",)"
                 R"("amount":"0")"
                 R"(})"
                 R"(],)"
                 R"("ts":1758180740327)"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::SNAPSHOT);
    REQUIRE(std::size(obj.data) == 1);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 2);
}
