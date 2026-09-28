/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/tools/throttle.hpp"

#include "roq/utils/compare.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/hash/fnv.hpp"

#include "roq/utils/charconv/from_chars.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace tools {

// === CONSTANTS ===

namespace {
auto const DEFAULT_BACKOFF = 1min;
auto const BLOCKED_BACKOFF = 5min;  // note! very serious
}  // namespace

// === HELPERS ===

namespace {
// note! std::tolower is not constexpr gcc16 + clang23
constexpr auto lower(auto value) {
  return utils::detail::ascii_to_lower(value);
}

enum class Header {
  UNKNOWN,
  X_MBX_USED_REMAIN_LIMIT,
};

constexpr auto parse_header(std::string_view const &text) {
  std::string value;
  value.reserve(std::size(text));
  std::transform(std::begin(text), std::end(text), std::back_inserter(value), [](auto c) { return lower(c); });
  auto key = utils::hash::FNV::compute(value);
  switch (key) {
    case utils::hash::FNV::compute("x-mbx-used-remain-limit"sv):
      return Header::X_MBX_USED_REMAIN_LIMIT;
  }
  return Header::UNKNOWN;
}

static_assert(parse_header("x-mbx-used-remain-limit"sv) == Header::X_MBX_USED_REMAIN_LIMIT);
}  // namespace

// === IMPLEMENTATION ===

Throttle::Throttle(server::Settings const &settings) : enabled_{settings.experimental.enable_rate_limit} {
}

// web::rest::Interceptor

void Throttle::operator()(Trace<web::rest::MessageBegin> const &) {
}

void Throttle::operator()(Trace<web::rest::MessageHeader> const &event) {
  auto &[trace_info, header] = event;
  auto update_value = [&](auto &result) {
    using value_type = std::remove_cvref_t<decltype(result)>;
    auto value = utils::charconv::from_chars<value_type>(header.value);
    return utils::update(result, value);
  };
  auto key = parse_header(header.name);
  switch (key) {
    using enum Header;
    [[likely]] case UNKNOWN:
      return;
    case X_MBX_USED_REMAIN_LIMIT:
      update_value(params_.used_remain_limit);
      break;
  }
}

void Throttle::operator()(Trace<web::rest::MessageEnd> const &event) {
  auto &[trace_info, message_end] = event;
  if (!enabled_) {
    return;
  }
  switch (message_end.status) {
    using enum web::http::Status;
    [[unlikely]] case FORBIDDEN: {  // 403
      auto now = clock::get_system();
      suspend_until_ = std::max(suspend_until_, now + BLOCKED_BACKOFF);
      break;
    }
    [[unlikely]] case TOO_MANY_REQUESTS: {  // 429
      if (suspend_until_.count() == 0) {
        auto now = clock::get_system();
        suspend_until_ = now + DEFAULT_BACKOFF;
      }
      break;
    }
    default:
      break;
  }
}

// web::socket::Interceptor

}  // namespace tools
}  // namespace bitget
}  // namespace roq
