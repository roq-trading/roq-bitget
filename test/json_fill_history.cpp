/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/fill_history.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

// TODO empty

TEST_CASE("simple", "[json_fill_history]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758167065271,)"
                 R"("data":{)"
                 R"("list":[{)"
                 R"("execId":"1351958722167156758",)"
                 R"("orderId":"1351958722159730706",)"
                 R"("clientOid":"1351958722159730709",)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("orderType":"limit",)"
                 R"("side":"buy",)"
                 R"("tradeSide":"close_short",)"
                 R"("execPrice":"116535.3",)"
                 R"("execQty":"0.002",)"
                 R"("execValue":"233.0706",)"
                 R"("tradeScope":"taker",)"
                 R"("feeDetail":[{)"
                 R"("feeCoin":"USDT",)"
                 R"("fee":"-0.08157471")"
                 R"(})"
                 R"(],)"
                 R"("createdTime":"1758057998220",)"
                 R"("updatedTime":"1758057998227",)"
                 R"("execPnl":"0.1418",)"
                 R"("execLinkId":"1351958722167156758")"
                 R"(},{)"
                 R"("execId":"1351957615097393153",)"
                 R"("orderId":"1351957615085772804",)"
                 R"("clientOid":"1351957615085772809",)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("orderType":"market",)"
                 R"("side":"sell",)"
                 R"("tradeSide":"open_short",)"
                 R"("execPrice":"116606.2",)"
                 R"("execQty":"0.002",)"
                 R"("execValue":"233.2124",)"
                 R"("tradeScope":"taker",)"
                 R"("feeDetail":[{)"
                 R"("feeCoin":"USDT",)"
                 R"("fee":"-0.08162434")"
                 R"(})"
                 R"(],)"
                 R"("createdTime":"1758057734274",)"
                 R"("updatedTime":"1758057734285",)"
                 R"("execPnl":"0",)"
                 R"("execLinkId":"1351957615097393153")"
                 R"(})"
                 R"(],)"
                 R"("cursor":"1351957615097393153")"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 2};
  json::FillHistory obj{message, buffer};
  REQUIRE(std::size(obj.data.list) == 2);
}
