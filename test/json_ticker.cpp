/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Ticker;

TEST_CASE("simple", "[json_ticker]") {
  auto message = R"({)"
                 R"("action":"snapshot",)"
                 R"("arg":{)"
                 R"("instType":"usdt-futures",)"
                 R"("topic":"ticker",)"
                 R"("symbol":"ETHUSDT")"
                 R"(},)"
                 R"("data":[{)"
                 R"("highPrice24h":"4552.9",)"
                 R"("lowPrice24h":"4421.82",)"
                 R"("openPrice24h":"4466.32",)"
                 R"("lastPrice":"4488.01",)"
                 R"("turnover24h":"6665054599.47691892",)"
                 R"("volume24h":"1484525.858642",)"
                 R"("bid1Price":"4488",)"
                 R"("ask1Price":"4488.01",)"
                 R"("bid1Size":"235.61",)"
                 R"("ask1Size":"167.15",)"
                 R"("price24hPcnt":"-0.00252",)"
                 R"("indexPrice":"4489.9910584085476633",)"
                 R"("markPrice":"4488.22",)"
                 R"("fundingRate":"0.000081",)"
                 R"("openInterest":"1308410.83",)"
                 R"("deliveryTime":"",)"
                 R"("deliveryStartTime":"",)"
                 R"("deliveryStatus":"",)"
                 R"("nextFundingTime":"1758124800000")"
                 R"(})"
                 R"(],)"
                 R"("ts":1758111262250)"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.action == json::Action::SNAPSHOT);
    REQUIRE(std::size(obj.data) == 1);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
