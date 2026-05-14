/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace bitget {

enum class RestState : uint8_t {
  UNDEFINED = 0,
  INSTRUMENTS,
  DONE,
};

}  // namespace bitget
}  // namespace roq
