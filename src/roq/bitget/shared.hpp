/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/symbols.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/bitget/api.hpp"
#include "roq/bitget/settings.hpp"

namespace roq {
namespace bitget {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

 public:
  std::vector<MBPUpdate> bids, asks, final_bids, final_asks;
  std::vector<Trade> trades;
  std::vector<Fill> fills;

 private:
  server::Dispatcher &dispatcher_;

 public:
  Settings const &settings;
  API const api;
  core::limit::RateLimiter rate_limiter;
  core::Symbols symbols;
};

}  // namespace bitget
}  // namespace roq
