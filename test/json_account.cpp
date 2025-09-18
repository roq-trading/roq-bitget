/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/account.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_account]") {
  auto message = R"({)"
                 R"("action":"snapshot",)"
                 R"("arg":{)"
                 R"("instType":"UTA",)"
                 R"("topic":"account")"
                 R"(},)"
                 R"("data":[{)"
                 R"("totalEquity":"681.49",)"
                 R"("effEquity":"624.2",)"
                 R"("mmr":"0",)"
                 R"("imr":"0",)"
                 R"("mgnRatio":"0",)"
                 R"("positionMgnRatio":"0",)"
                 R"("unrealisedPnL":"0",)"
                 R"("coin":[{)"
                 R"("coin":"EUR",)"
                 R"("balance":"39.3748947",)"
                 R"("locked":"0",)"
                 R"("equity":"39.3748947",)"
                 R"("usdValue":"46.58884031",)"
                 R"("available":"39.3748947",)"
                 R"("borrow":"0",)"
                 R"("debts":"0")"
                 R"(},{)"
                 R"("coin":"BTC",)"
                 R"("balance":"0.0046",)"
                 R"("locked":"0",)"
                 R"("equity":"0.0046",)"
                 R"("usdValue":"534.91028783",)"
                 R"("available":"0.0046",)"
                 R"("borrow":"0",)"
                 R"("debts":"0")"
                 R"(},{)"
                 R"("coin":"USDT",)"
                 R"("balance":"99.97860095",)"
                 R"("locked":"0",)"
                 R"("equity":"99.97860095",)"
                 R"("usdValue":"99.99549139",)"
                 R"("available":"99.97860095",)"
                 R"("borrow":"0",)"
                 R"("debts":"0")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(],)"
                 R"("ts":1758113442410)"
                 R"(})";
  core::json::BufferStack buffer{8192, 2};
  json::Account obj{message, buffer};
  auto &data = obj.data;
  REQUIRE(std::size(data) == 1);
}
