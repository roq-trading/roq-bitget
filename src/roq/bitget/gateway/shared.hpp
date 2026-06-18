/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/symbols.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/bitget/gateway/api.hpp"
#include "roq/bitget/gateway/settings.hpp"

namespace roq {
namespace bitget {
namespace gateway {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  auto discard_symbol(std::string_view const &name) const { return dispatcher.discard_symbol(name); }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher(std::forward<Args>(args)...);
  }

 public:
  std::vector<MBPUpdate> bids, asks, final_bids, final_asks;
  std::vector<Trade> trades;
  std::vector<Fill> fills;

  server::Dispatcher &dispatcher;

 public:
  Settings const &settings;
  API const api;
  core::limit::RateLimiter rate_limiter;
  core::Symbols symbols;
  utils::unordered_set<std::string> all_symbols;
};

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
