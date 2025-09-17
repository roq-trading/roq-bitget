/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget_futures/json/message.hpp"

using namespace roq;
using namespace roq::bitget_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

/*
// note! doesn't work due to escaping
TEST_CASE("json_message_error_not_working", "[json_message]") {
  auto message = R"({)"
                 R"("event":"error",)"
                 R"("code":30002,)"
                 R"("msg":"Unrecognized request:{\"ping\":1559291007437691}")"
                 R"(})";
  core::json::BufferStack buffer{8192,1};
  [[maybe_unused]] json::Message obj{message, buffer};
}
*/
TEST_CASE("json_message_error_modified", "[json_message]") {
  auto message = R"({)"
                 R"("event":"error",)"
                 R"("code":30001,)"
                 R"("msg":"instType:SP, channel:ticker, instId:XRPETH_SPBL doesn't exist")"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Message obj{message, buffer};
}

TEST_CASE("json_message_subscribe", "[json_message]") {
  auto message = R"({)"
                 R"("event":"subscribe",)"
                 R"("arg":{)"
                 R"("instType":"sp",)"
                 R"("channel":"ticker",)"
                 R"("instId":"MASKUSDT")"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Message obj{message, buffer};
}

TEST_CASE("json_message_ticker_snapshot", "[json_message]") {
  auto message = R"({)"
                 R"("action":"snapshot",)"
                 R"("arg":{)"
                 R"("instType":"sp",)"
                 R"("channel":"ticker",)"
                 R"("instId":"MASKUSDT"},)"
                 R"("data":[{)"
                 R"("instId":"MASKUSDT",)"
                 R"("last":"13.4758",)"
                 R"("open24h":"13.9270",)"
                 R"("high24h":"14.0461",)"
                 R"("low24h":"13.1470",)"
                 R"("bestBid":"13.468100",)"
                 R"("bestAsk":"13.486100",)"
                 R"("baseVolume":"33134.9935",)"
                 R"("quoteVolume":"450169.1986",)"
                 R"("ts":1641217090075)"
                 R"(})"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Message obj{message, buffer};
}
