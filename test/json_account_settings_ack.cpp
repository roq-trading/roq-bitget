/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/bitget/json/account_settings_ack.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::AccountSettingsAck;

TEST_CASE("simple", "[json_account_settings_ack]") {
  auto message = R"({)"
                 R"("code":"00000",)"
                 R"("msg":"success",)"
                 R"("requestTime":1758162763365,)"
                 R"("data":{)"
                 R"("uid":"4486145123",)"
                 R"("accountMode":"unified",)"
                 R"("assetMode":"multi_assets",)"
                 R"("holdMode":"hedge_mode",)"
                 R"("stpMode":"none",)"
                 R"("symbolConfigList":[{)"
                 R"("category":"USDT-FUTURES",)"
                 R"("symbol":"BTCUSDT",)"
                 R"("marginMode":"crossed",)"
                 R"("leverage":"10")"
                 R"(})"
                 R"(],)"
                 R"("coinConfigList":[])"
                 R"(})"
                 R"(})";
  auto helper = [&](value_type &obj) {
    REQUIRE(obj.code == 0);
    REQUIRE(obj.msg == "success"sv);
    REQUIRE(std::size(obj.data.symbol_config_list) == 1);
    REQUIRE(std::size(obj.data.coin_config_list) == 0);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
