/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/message.hpp"

using namespace roq;
using namespace roq::bitget;

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
TEST_CASE("json_message_error_modified", "[json_message]") {
  auto message = R"({)"
                 R"("event":"error",)"
                 R"("code":30001,)"
                 R"("msg":"instType:SP, channel:ticker, instId:XRPETH_SPBL doesn't exist")"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Message obj{message, buffer};
}
*/

TEST_CASE("json_message_subscribe", "[json_message]") {
  auto message = R"({)"
                 R"("event":"subscribe",)"
                 R"("arg":{)"
                 R"("instType":"usdt-futures",)"
                 R"("topic":"ticker",)"
                 R"("symbol":"BTCUSDT")"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Message obj{message, buffer};
}
