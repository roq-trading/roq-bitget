/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/protocol/json/unfilled_orders_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::UnfilledOrdersAck;

TEST_CASE("empty", "[json_unfilled_orders_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758166069242,)"
                 R"("data":{)"
                 R"("list":[],)"
                 R"("cursor":null)"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 0);
    REQUIRE(obj.msg == "success"sv);
    REQUIRE(std::size(obj.data.list) == 0);
  };
  core::json::BufferStack buffers{8192, 2};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("working", "[json_unfilled_orders_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758179218375,)"
                 R"("data":{)"
                 R"("list":[{)"
                 R"("orderId":"1352466809719251013",)"
                 R"("clientOid":"RwACZMPMXDQAAQAAAAAA",)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("orderType":"limit",)"
                 R"("side":"buy",)"
                 R"("price":"32000",)"
                 R"("qty":"0.0002",)"
                 R"("amount":"0",)"
                 R"("cumExecQty":"0",)"
                 R"("cumExecValue":"0",)"
                 R"("avgPrice":"0",)"
                 R"("timeInForce":"gtc",)"
                 R"("orderStatus":"new",)"
                 R"("posSide":"long",)"
                 R"("holdMode":"hedge_mode",)"
                 R"("reduceOnly":"NO",)"
                 R"("feeDetail":[{)"
                 R"("feeCoin":"",)"
                 R"("fee":"")"
                 R"(})"
                 R"(],)"
                 R"("createdTime":"1758179135732",)"
                 R"("updatedTime":"1758179135741",)"
                 R"("stpMode":"none",)"
                 R"("tpTriggerBy":null,)"
                 R"("slTriggerBy":null,)"
                 R"("takeProfit":null,)"
                 R"("stopLoss":null,)"
                 R"("tpOrderType":null,)"
                 R"("slOrderType":null,)"
                 R"("tpLimitPrice":null,)"
                 R"("slLimitPrice":null)"
                 R"(})"
                 R"(],)"
                 R"("cursor":"1352466809719251013")"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 0);
    REQUIRE(obj.msg == "success"sv);
    REQUIRE(std::size(obj.data.list) == 1);
  };
  core::json::BufferStack buffers{8192, 2};
  value_type obj{message, buffers};
  helper(obj);
}
