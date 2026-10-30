/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/protocol/json/order_details_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::OrderDetailsAck;

TEST_CASE("simple", "[json_order_details_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1793334984900,)"
                 R"("data":{)"
                 R"("orderId":"1499921110940409856",)"
                 R"("clientOid":"DBACArpRN4YAAQAAAAAA",)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("orderType":"market",)"
                 R"("side":"buy",)"
                 R"("price":"",)"
                 R"("qty":"0.001",)"
                 R"("amount":"0",)"
                 R"("cumExecQty":"0.001",)"
                 R"("cumExecValue":"78.2903",)"
                 R"("avgPrice":"78290.3",)"
                 R"("timeInForce":"ioc",)"
                 R"("orderStatus":"filled",)"
                 R"("posSide":"long",)"
                 R"("tradeSide":"buy_single",)"
                 R"("holdMode":"one_way_mode",)"
                 R"("reduceOnly":"NO",)"
                 R"("feeDetail":[{)"
                 R"("feeCoin":"USDT",)"
                 R"("fee":"0.04697418")"
                 R"(})"
                 R"(],)"
                 R"("createdTime":"1793334980711",)"
                 R"("updatedTime":"1793334980713",)"
                 R"("cancelReason":"",)"
                 R"("execType":"normal",)"
                 R"("stpMode":"none",)"
                 R"("tpTriggerBy":null,)"
                 R"("slTriggerBy":null,)"
                 R"("takeProfit":null,)"
                 R"("stopLoss":null,)"
                 R"("tpOrderType":null,)"
                 R"("slOrderType":null,)"
                 R"("tpLimitPrice":null,)"
                 R"("slLimitPrice":null,)"
                 R"("marginMode":"crossed",)"
                 R"("delegateType":"market",)"
                 R"("venue":"",)"
                 R"("rTokenTimeInForce":""})"
                 R"(})"sv;
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 0);
    REQUIRE(obj.msg == "success"sv);
  };
  core::json::BufferStack buffers{8192, 2};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("missing", "[json_order_details_ack]") {
  auto message = R"({)"
                 R"("code":"25204",)"
                 R"("msg":"Order does not exist",)"
                 R"("requestTime":1793338018660,)"
                 R"("data":null)"
                 R"(})"sv;
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 25204);
    REQUIRE(obj.msg == "Order does not exist"sv);
  };
  core::json::BufferStack buffers{8192, 2};
  value_type obj{message, buffers};
  helper(obj);
}
