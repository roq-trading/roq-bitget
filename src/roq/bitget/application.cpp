/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/application.hpp"

#include "roq/bitget/config.hpp"
#include "roq/bitget/gateway.hpp"
#include "roq/bitget/settings.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {

// === CONSTANTS ===

namespace {
uint8_t const API_SPOT = 0x0;
uint8_t const API_MARGIN = 0x4;  // XXX FIXME TODO
uint8_t const API_USDT_FUTURES = 0x2;
uint8_t const API_USDC_FUTURES = 0x3;
uint8_t const API_COIN_FUTURES = 0x1;  // note!
}  // namespace

// === HELPERS ===

namespace {
auto parse_api(auto &settings) {
  auto api = API::parse_api(settings);
  switch (api) {
    using enum API::Key;
    case SPOT:
      return API_SPOT;
    case MARGIN:
      return API_MARGIN;
    case USDT_FUTURES:
      return API_USDT_FUTURES;
    case USDC_FUTURES:
      return API_USDC_FUTURES;
    case COIN_FUTURES:
      return API_COIN_FUTURES;
  }
  log::fatal(R"(Unexpected: api="{}")"sv, settings.app.api);
}
}  // namespace

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  auto api = parse_api(settings);
  Config config{settings};
  log::info<1>("config={}"sv, config);
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>(settings, config, *context, api).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitget
}  // namespace roq
