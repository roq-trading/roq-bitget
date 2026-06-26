/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/compat.hpp"

#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/bitget/flags/flags.hpp"
#include "roq/bitget/flags/misc.hpp"
#include "roq/bitget/flags/rest.hpp"
#include "roq/bitget/flags/ws.hpp"

namespace roq {
namespace bitget {
namespace flags {

struct ROQ_PUBLIC Settings final : public server::flags::Settings, public flags::Flags {
  explicit Settings(args::Parser const &);

  flags::Misc misc;
  flags::REST rest;
  flags::WS ws;

 private:
  explicit Settings(args::Parser const &, flags::Flags const &);
};

}  // namespace flags
}  // namespace bitget
}  // namespace roq

template <>
struct fmt::formatter<roq::bitget::flags::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::bitget::flags::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(misc={}, )"
        R"(rest={}, )"
        R"(ws={}, )"
        R"(server={})"
        R"(}})"sv,
        value.misc,
        value.rest,
        value.ws,
        static_cast<roq::server::Settings const &>(value));
  }
};
