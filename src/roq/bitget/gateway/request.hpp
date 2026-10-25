/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/core/limit/request.hpp"

#include "roq/bitget/gateway/shared.hpp"

namespace roq {
namespace bitget {
namespace gateway {

struct Request final : public core::limit::Request::Handler {
  explicit Request(Shared &);

  // core::Request::Handler

  bool ready() const override;
  bool can_request(TraceInfo const &) override;

  // ...

  core::limit::Request account_settings;
  core::limit::Request account_assets;
  core::limit::Request current_positions;
  core::limit::Request unfilled_orders;
  core::limit::Request trade_fills;

 private:
  Shared &shared_;
};

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
