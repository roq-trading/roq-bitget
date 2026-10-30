/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/utils/container.hpp"

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

  core::limit::Request account;
  core::limit::Request balances;
  core::limit::Request positions;
  core::limit::Request open_orders;
  core::limit::Request lost_orders;
  core::limit::Request fills;

  utils::unordered_map<std::string, std::string> details_lost_orders;

 private:
  Shared &shared_;
};

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
