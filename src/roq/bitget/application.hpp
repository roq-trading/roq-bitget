/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/service.hpp"

namespace roq {
namespace bitget {

class Application final : public roq::Service {
 public:
  using roq::Service::Service;

 protected:
  int main(args::Parser const &) override;
};

}  // namespace bitget
}  // namespace roq
