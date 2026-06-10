/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/protocol/json/account_assets_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::AccountAssetsAck;

TEST_CASE("simple", "[json_account_assets_ack]") {
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
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 0);
    REQUIRE(obj.msg == "success"sv);
    REQUIRE(std::size(obj.data.assets) == 3);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
