/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/account_assets.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_account_assets]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758163153240,)"
                 R"("data":{)"
                 R"("accountEquity":"683.00426814",)"
                 R"("usdtEquity":"682.80669942",)"
                 R"("btcEquity":"0.00585648",)"
                 R"("unrealisedPnl":"0",)"
                 R"("usdtUnrealisedPnl":"0",)"
                 R"("btcUnrealizedPnl":"0",)"
                 R"("effEquity":"625.74639071",)"
                 R"("mmr":"0",)"
                 R"("imr":"0",)"
                 R"("mgnRatio":"0",)"
                 R"("positionMgnRatio":"0",)"
                 R"("assets":[{)"
                 R"("coin":"BTC",)"
                 R"("equity":"0.0046",)"
                 R"("usdValue":"536.46822566",)"
                 R"("balance":"0.0046",)"
                 R"("available":"0.0046",)"
                 R"("debt":"0",)"
                 R"("locked":"0"},{)"
                 R"("coin":"USDT",)"
                 R"("equity":"99.97860095",)"
                 R"("usdValue":"100.00752955",)"
                 R"("balance":"99.97860095",)"
                 R"("available":"99.97860095",)"
                 R"("debt":"0",)"
                 R"("locked":"0")"
                 R"(},{)"
                 R"("coin":"EUR",)"
                 R"("equity":"39.3748947",)"
                 R"("usdValue":"46.52851292",)"
                 R"("balance":"39.3748947",)"
                 R"("available":"39.3748947",)"
                 R"("debt":"0",)"
                 R"("locked":"0")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 2};
  json::AccountAssets obj{message, buffer};
  REQUIRE(std::size(obj.data.assets) == 3);
}
