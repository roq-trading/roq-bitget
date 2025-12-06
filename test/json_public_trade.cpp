/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

using value_type = json::PublicTrade;

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
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::UPDATE);
    REQUIRE(std::size(obj.data) == 1);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
