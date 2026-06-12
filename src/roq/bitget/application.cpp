/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/application.hpp"

#include "roq/bitget/flags/settings.hpp"

#include "roq/bitget/gateway/config.hpp"
#include "roq/bitget/gateway/controller.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  flags::Settings settings{args};
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<gateway::Controller>(settings, config, *context).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace bitget
}  // namespace roq
