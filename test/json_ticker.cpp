/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget_futures/json/ticker.hpp"

using namespace roq;
using namespace roq::bitget_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_ticker_snapshot", "[json_ticker]") {
  auto message = R"({)"
                 R"("action":"snapshot",)"
                 R"("arg":{)"
                 R"("instType":"COIN-FUTURES",)"
                 R"("channel":"ticker",)"
                 R"("instId":"BTCUSD")"
                 R"(},)"
                 R"("data":[{)"
                 R"("instId":"BTCUSD",)"
                 R"("lastPr":"109822.5",)"
                 R"("bidPr":"109815.8",)"
                 R"("askPr":"109815.9",)"
                 R"("bidSz":"2.469",)"
                 R"("askSz":"3.321",)"
                 R"("open24h":"112171.9",)"
                 R"("high24h":"113271.5",)"
                 R"("low24h":"108660",)"
                 R"("change24h":"-0.03045",)"
                 R"("fundingRate":"0.0001",)"
                 R"("nextFundingTime":"1756195200000",)"
                 R"("markPrice":"109822.7",)"
                 R"("indexPrice":"109867.6857822380165048",)"
                 R"("holdingAmount":"21425.2578",)"
                 R"("baseVolume":"4075.2916",)"
                 R"("quoteVolume":"451366093.334",)"
                 R"("openUtc":"110104.2",)"
                 R"("symbolType":"1",)"
                 R"("symbol":"BTCUSD",)"
                 R"("deliveryPrice":"0",)"
                 R"("ts":"1756180111636")"
                 R"(})"
                 R"(],)"
                 R"("ts":1756180111642)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::Ticker obj{message, buffer};
  auto &data = obj.data;
  REQUIRE(std::size(data) == 1);
}
