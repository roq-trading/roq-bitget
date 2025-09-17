/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget_futures/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// bitget_futures::json => roq

// bitget_futures::json::AssetMode => roq::MarginMode

template <>
template <>
constexpr Helper<bitget_futures::json::AssetMode>::operator std::optional<roq::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum bitget_futures::json::AssetMode::type_t;
    case UNDEFINED_INTERNAL:
      return roq::MarginMode::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::MarginMode::UNDEFINED;
    case SINGLE:
      return roq::MarginMode::ISOLATED;
    case UNION:
      return roq::MarginMode::CROSS;
  }
  return {};
}

static_assert(Helper{bitget_futures::json::AssetMode{bitget_futures::json::AssetMode::UNDEFINED_INTERNAL}} == roq::MarginMode::UNDEFINED);
static_assert(Helper{bitget_futures::json::AssetMode{bitget_futures::json::AssetMode::SINGLE}} == roq::MarginMode::ISOLATED);
static_assert(Helper{bitget_futures::json::AssetMode{bitget_futures::json::AssetMode::UNION}} == roq::MarginMode::CROSS);

template <>
template <>
std::optional<roq::MarginMode> Map<bitget_futures::json::AssetMode>::helper() const {
  return Helper{args_};
}

// bitget_futures::json::Force => roq::TimeInForce

template <>
template <>
constexpr Helper<bitget_futures::json::Force>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum bitget_futures::json::Force::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case GTC:
      return roq::TimeInForce::GTC;
    case POST_ONLY:
      return roq::TimeInForce::GTC;
    case FOK:
      return roq::TimeInForce::FOK;
    case IOC:
      return roq::TimeInForce::IOC;
  }
  return {};
}

static_assert(Helper{bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{bitget_futures::json::Force{bitget_futures::json::Force::GTC}} == roq::TimeInForce::GTC);
static_assert(Helper{bitget_futures::json::Force{bitget_futures::json::Force::POST_ONLY}} == roq::TimeInForce::GTC);
static_assert(Helper{bitget_futures::json::Force{bitget_futures::json::Force::FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{bitget_futures::json::Force{bitget_futures::json::Force::IOC}} == roq::TimeInForce::IOC);

template <>
template <>
std::optional<roq::TimeInForce> Map<bitget_futures::json::Force>::helper() const {
  return Helper{args_};
}

// bitget_futures::json::MarginMode => roq::MarginMode

template <>
template <>
constexpr Helper<bitget_futures::json::MarginMode>::operator std::optional<roq::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum bitget_futures::json::MarginMode::type_t;
    case UNDEFINED_INTERNAL:
      return roq::MarginMode::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::MarginMode::UNDEFINED;
    case CROSSED:
      return roq::MarginMode::CROSS;
    case ISOLATED:
      return roq::MarginMode::ISOLATED;
  }
  return {};
}

static_assert(Helper{bitget_futures::json::MarginMode{bitget_futures::json::MarginMode::UNDEFINED_INTERNAL}} == roq::MarginMode::UNDEFINED);
static_assert(Helper{bitget_futures::json::MarginMode{bitget_futures::json::MarginMode::CROSSED}} == roq::MarginMode::CROSS);
static_assert(Helper{bitget_futures::json::MarginMode{bitget_futures::json::MarginMode::ISOLATED}} == roq::MarginMode::ISOLATED);

template <>
template <>
std::optional<roq::MarginMode> Map<bitget_futures::json::MarginMode>::helper() const {
  return Helper{args_};
}

// bitget_futures::json::OrderStatus => roq::OrderStatus

template <>
template <>
constexpr Helper<bitget_futures::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum bitget_futures::json::OrderStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case LIVE:
      return roq::OrderStatus::WORKING;
    case PARTIALLY_FILLED:
      return roq::OrderStatus::WORKING;
    case FILLED:
      return roq::OrderStatus::COMPLETED;
    case CANCELED:
      return roq::OrderStatus::CANCELED;
  }
  return {};
}

static_assert(Helper{bitget_futures::json::OrderStatus{bitget_futures::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{bitget_futures::json::OrderStatus{bitget_futures::json::OrderStatus::LIVE}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitget_futures::json::OrderStatus{bitget_futures::json::OrderStatus::PARTIALLY_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitget_futures::json::OrderStatus{bitget_futures::json::OrderStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{bitget_futures::json::OrderStatus{bitget_futures::json::OrderStatus::CANCELED}} == roq::OrderStatus::CANCELED);

template <>
template <>
std::optional<roq::OrderStatus> Map<bitget_futures::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// bitget_futures::json::OrderType => roq::OrderType

template <>
template <>
constexpr Helper<bitget_futures::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum bitget_futures::json::OrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case MARKET:
      return roq::OrderType::MARKET;
    case LIMIT:
      return roq::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{bitget_futures::json::OrderType{bitget_futures::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{bitget_futures::json::OrderType{bitget_futures::json::OrderType::MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{bitget_futures::json::OrderType{bitget_futures::json::OrderType::LIMIT}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<bitget_futures::json::OrderType>::helper() const {
  return Helper{args_};
}

// bitget_futures::json::Side => roq::Side

template <>
template <>
constexpr Helper<bitget_futures::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum bitget_futures::json::Side::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Side::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{bitget_futures::json::Side{bitget_futures::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{bitget_futures::json::Side{bitget_futures::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{bitget_futures::json::Side{bitget_futures::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<bitget_futures::json::Side>::helper() const {
  return Helper{args_};
}

// roq => bitget_futures::json

// roq::MarginMode => bitget_futures::json::MarginMode

template <>
template <>
constexpr Helper<roq::MarginMode>::operator std::optional<bitget_futures::json::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum roq::MarginMode;
    case UNDEFINED:
      return bitget_futures::json::MarginMode::UNDEFINED_INTERNAL;
    case CROSS:
      return bitget_futures::json::MarginMode::CROSSED;
    case ISOLATED:
      return bitget_futures::json::MarginMode::ISOLATED;
    case PORTFOLIO:
      return bitget_futures::json::MarginMode::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::MarginMode::UNDEFINED} == bitget_futures::json::MarginMode{bitget_futures::json::MarginMode::UNDEFINED_INTERNAL});
static_assert(Helper{roq::MarginMode::CROSS} == bitget_futures::json::MarginMode{bitget_futures::json::MarginMode::CROSSED});
static_assert(Helper{roq::MarginMode::ISOLATED} == bitget_futures::json::MarginMode{bitget_futures::json::MarginMode::ISOLATED});
static_assert(Helper{roq::MarginMode::PORTFOLIO} == bitget_futures::json::MarginMode{bitget_futures::json::MarginMode::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<bitget_futures::json::MarginMode> Map<roq::MarginMode>::helper() const {
  return Helper{args_};
}

// roq::OrderType => bitget_futures::json::OrderType

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<bitget_futures::json::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return bitget_futures::json::OrderType::UNDEFINED_INTERNAL;
    case MARKET:
      return bitget_futures::json::OrderType::MARKET;
    case LIMIT:
      return bitget_futures::json::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == bitget_futures::json::OrderType{bitget_futures::json::OrderType::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderType::MARKET} == bitget_futures::json::OrderType{bitget_futures::json::OrderType::MARKET});
static_assert(Helper{roq::OrderType::LIMIT} == bitget_futures::json::OrderType{bitget_futures::json::OrderType::LIMIT});

template <>
template <>
std::optional<bitget_futures::json::OrderType> Map<roq::OrderType>::helper() const {
  return Helper{args_};
}

// roq::PositionEffect => bitget_futures::json::TradeSide

template <>
template <>
constexpr Helper<roq::PositionEffect>::operator std::optional<bitget_futures::json::TradeSide>() const {
  switch (std::get<0>(args_)) {
    using enum roq::PositionEffect;
    case UNDEFINED:
      return bitget_futures::json::TradeSide::UNDEFINED_INTERNAL;
    case OPEN:
      return bitget_futures::json::TradeSide::OPEN;
    case CLOSE:
      return bitget_futures::json::TradeSide::CLOSE;
  }
  return {};
}

static_assert(Helper{roq::PositionEffect::UNDEFINED} == bitget_futures::json::TradeSide{bitget_futures::json::TradeSide::UNDEFINED_INTERNAL});
static_assert(Helper{roq::PositionEffect::OPEN} == bitget_futures::json::TradeSide{bitget_futures::json::TradeSide::OPEN});
static_assert(Helper{roq::PositionEffect::CLOSE} == bitget_futures::json::TradeSide{bitget_futures::json::TradeSide::CLOSE});

template <>
template <>
std::optional<bitget_futures::json::TradeSide> Map<roq::PositionEffect>::helper() const {
  return Helper{args_};
}

// roq::Side => bitget_futures::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<bitget_futures::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return bitget_futures::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return bitget_futures::json::Side::BUY;
    case SELL:
      return bitget_futures::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == bitget_futures::json::Side{bitget_futures::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == bitget_futures::json::Side{bitget_futures::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == bitget_futures::json::Side{bitget_futures::json::Side::SELL});

template <>
template <>
std::optional<bitget_futures::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// roq::TimeInForce => bitget_futures::json::Force

// POST_ONLY ???
template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<bitget_futures::json::Force>() const {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case GFD:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case GTC:
      return bitget_futures::json::Force::GTC;
    case OPG:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case IOC:
      return bitget_futures::json::Force::IOC;
    case FOK:
      return bitget_futures::json::Force::FOK;
    case GTX:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case GTD:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case AT_THE_CLOSE:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case GOOD_THROUGH_CROSSING:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case AT_CROSSING:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case GOOD_FOR_TIME:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case GFA:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
    case GFM:
      return bitget_futures::json::Force::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFD} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTC} == bitget_futures::json::Force{bitget_futures::json::Force::GTC});
static_assert(Helper{roq::TimeInForce::OPG} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::IOC} == bitget_futures::json::Force{bitget_futures::json::Force::IOC});
static_assert(Helper{roq::TimeInForce::FOK} == bitget_futures::json::Force{bitget_futures::json::Force::FOK});
static_assert(Helper{roq::TimeInForce::GTX} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTD} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_FOR_TIME} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFA} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFM} == bitget_futures::json::Force{bitget_futures::json::Force::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<bitget_futures::json::Force> Map<roq::TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
