/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/public_trade.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

TEST_CASE("simple", "[json_public_trade]") {
  auto message = R"({)"
                 R"("action":"update",)"
                 R"("arg":{)"
                 R"("instType":"usdt-futures",)"
                 R"("topic":"publicTrade",)"
                 R"("symbol":"BTCUSDT")"
                 R"(},)"
                 R"("data":[{)"
                 R"("i":"1352182127218896905",)"
                 R"("L":"1352182127218896906",)"
                 R"("p":"116292",)"
                 R"("v":"0.0084",)"
                 R"("S":"buy",)"
                 R"("T":"1758111262135")"
                 R"(})"
                 R"(],)"
                 R"("ts":1758111262141)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::PublicTrade obj{message, buffer};
}
