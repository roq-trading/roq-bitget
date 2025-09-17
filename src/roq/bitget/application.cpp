/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/application.hpp"

#include "roq/bitget/config.hpp"
#include "roq/bitget/gateway.hpp"
#include "roq/bitget/settings.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {

// === CONSTANTS ===

namespace {
uint8_t const API_2 = {};
}

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Config config{settings};
  log::info<1>("config={}"sv, config);
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>(settings, config, *context, API_2).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitget
}  // namespace roq
