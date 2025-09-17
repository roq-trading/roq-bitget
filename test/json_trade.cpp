/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/trade.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

TEST_CASE("json_trade_snapshot", "[json_trade]") {
  auto message = R"({)"
                 R"("action":"update",)"
                 R"("arg":{)"
                 R"("instType":"COIN-FUTURES",)"
                 R"("channel":"trade",)"
                 R"("instId":"BTCUSD")"
                 R"(},)"
                 R"("data":[{)"
                 R"("ts":"1756180610495",)"
                 R"("price":"109843.5",)"
                 R"("size":"0.0107",)"
                 R"("side":"buy",)"
                 R"("tradeId":"1344084387322626071")"
                 R"(})"
                 R"(],)"
                 R"("ts":1756180610502)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Trade obj{message, buffer};
}
