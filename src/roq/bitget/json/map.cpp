/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// bitget::json => roq

// bitget::json::Action => roq::MarginMode

template <>
template <>
constexpr Helper<bitget::json::Action>::operator std::optional<roq::UpdateType>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::Action::type_t;
    case UNDEFINED_INTERNAL:
      return roq::UpdateType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::UpdateType::UNDEFINED;
    case SNAPSHOT:
      return roq::UpdateType::SNAPSHOT;
    case UPDATE:
      return roq::UpdateType::INCREMENTAL;
  }
  return {};
}

static_assert(Helper{bitget::json::Action{bitget::json::Action::UNDEFINED_INTERNAL}} == roq::UpdateType::UNDEFINED);
static_assert(Helper{bitget::json::Action{bitget::json::Action::SNAPSHOT}} == roq::UpdateType::SNAPSHOT);
static_assert(Helper{bitget::json::Action{bitget::json::Action::UPDATE}} == roq::UpdateType::INCREMENTAL);

template <>
template <>
std::optional<roq::UpdateType> Map<bitget::json::Action>::helper() const {
  return Helper{args_};
}

// bitget::json::AssetMode => roq::MarginMode

template <>
template <>
constexpr Helper<bitget::json::AssetMode>::operator std::optional<roq::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::AssetMode::type_t;
    case UNDEFINED_INTERNAL:
      return roq::MarginMode::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::MarginMode::UNDEFINED;
    case SINGLE:
      return roq::MarginMode::ISOLATED;
    case UNION:
      return roq::MarginMode::CROSS;
    case MULTI_ASSETS:
      return roq::MarginMode::CROSS;  // ???
  }
  return {};
}

static_assert(Helper{bitget::json::AssetMode{bitget::json::AssetMode::UNDEFINED_INTERNAL}} == roq::MarginMode::UNDEFINED);
static_assert(Helper{bitget::json::AssetMode{bitget::json::AssetMode::SINGLE}} == roq::MarginMode::ISOLATED);
static_assert(Helper{bitget::json::AssetMode{bitget::json::AssetMode::UNION}} == roq::MarginMode::CROSS);
static_assert(Helper{bitget::json::AssetMode{bitget::json::AssetMode::MULTI_ASSETS}} == roq::MarginMode::CROSS);

template <>
template <>
std::optional<roq::MarginMode> Map<bitget::json::AssetMode>::helper() const {
  return Helper{args_};
}

// {bitget::json::Category, bitget::json::FuturesType} => roq::SecurityType

template <>
template <>
constexpr Helper<bitget::json::Category, bitget::json::FuturesType>::operator std::optional<roq::SecurityType>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::Category::type_t;
    case UNDEFINED_INTERNAL:
      return roq::SecurityType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::SecurityType::UNDEFINED;
    case SPOT:
      return roq::SecurityType::SPOT;
    case MARGIN:
      return roq::SecurityType::SPOT;
    case USDT_FUTURES:
    case USDC_FUTURES:
    case COIN_FUTURES:
      switch (std::get<1>(args_)) {
        using enum bitget::json::FuturesType::type_t;
        case UNDEFINED_INTERNAL:
          return roq::SecurityType::UNDEFINED;
        case UNKNOWN_INTERNAL:
          return roq::SecurityType::UNDEFINED;
        case PERPETUAL:
          return roq::SecurityType::SWAP;
        case DELIVERY:
          return roq::SecurityType::FUTURES;
      }
      break;
  }
  return {};
}

static_assert(
    Helper{bitget::json::Category{bitget::json::Category::UNDEFINED_INTERNAL}, bitget::json::FuturesType{bitget::json::FuturesType::UNDEFINED_INTERNAL}} ==
    roq::SecurityType::UNDEFINED);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::SPOT}, bitget::json::FuturesType{bitget::json::FuturesType::UNDEFINED_INTERNAL}} ==
    roq::SecurityType::SPOT);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::MARGIN}, bitget::json::FuturesType{bitget::json::FuturesType::UNDEFINED_INTERNAL}} ==
    roq::SecurityType::SPOT);

static_assert(
    Helper{bitget::json::Category{bitget::json::Category::USDT_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::UNDEFINED_INTERNAL}} ==
    roq::SecurityType::UNDEFINED);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::USDT_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::UNKNOWN_INTERNAL}} ==
    roq::SecurityType::UNDEFINED);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::USDT_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::PERPETUAL}} ==
    roq::SecurityType::SWAP);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::USDT_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::DELIVERY}} ==
    roq::SecurityType::FUTURES);

static_assert(
    Helper{bitget::json::Category{bitget::json::Category::USDC_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::UNDEFINED_INTERNAL}} ==
    roq::SecurityType::UNDEFINED);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::USDC_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::UNKNOWN_INTERNAL}} ==
    roq::SecurityType::UNDEFINED);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::USDC_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::PERPETUAL}} ==
    roq::SecurityType::SWAP);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::USDC_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::DELIVERY}} ==
    roq::SecurityType::FUTURES);

static_assert(
    Helper{bitget::json::Category{bitget::json::Category::COIN_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::UNDEFINED_INTERNAL}} ==
    roq::SecurityType::UNDEFINED);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::COIN_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::UNKNOWN_INTERNAL}} ==
    roq::SecurityType::UNDEFINED);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::COIN_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::PERPETUAL}} ==
    roq::SecurityType::SWAP);
static_assert(
    Helper{bitget::json::Category{bitget::json::Category::COIN_FUTURES}, bitget::json::FuturesType{bitget::json::FuturesType::DELIVERY}} ==
    roq::SecurityType::FUTURES);

template <>
template <>
std::optional<roq::SecurityType> Map<bitget::json::Category, bitget::json::FuturesType>::helper() const {
  return Helper{args_};
}

// bitget::json::MarginMode => roq::MarginMode

template <>
template <>
constexpr Helper<bitget::json::MarginMode>::operator std::optional<roq::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::MarginMode::type_t;
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

static_assert(Helper{bitget::json::MarginMode{bitget::json::MarginMode::UNDEFINED_INTERNAL}} == roq::MarginMode::UNDEFINED);
static_assert(Helper{bitget::json::MarginMode{bitget::json::MarginMode::CROSSED}} == roq::MarginMode::CROSS);
static_assert(Helper{bitget::json::MarginMode{bitget::json::MarginMode::ISOLATED}} == roq::MarginMode::ISOLATED);

template <>
template <>
std::optional<roq::MarginMode> Map<bitget::json::MarginMode>::helper() const {
  return Helper{args_};
}

// bitget::json::OrderStatus => roq::OrderStatus

template <>
template <>
constexpr Helper<bitget::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::OrderStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case LIVE:
      return roq::OrderStatus::WORKING;
    case NEW:
      return roq::OrderStatus::WORKING;
    case PARTIALLY_FILLED:
      return roq::OrderStatus::WORKING;
    case FILLED:
      return roq::OrderStatus::COMPLETED;
    case CANCELLED:
      return roq::OrderStatus::CANCELED;
  }
  return {};
}

static_assert(Helper{bitget::json::OrderStatus{bitget::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{bitget::json::OrderStatus{bitget::json::OrderStatus::LIVE}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitget::json::OrderStatus{bitget::json::OrderStatus::NEW}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitget::json::OrderStatus{bitget::json::OrderStatus::PARTIALLY_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitget::json::OrderStatus{bitget::json::OrderStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{bitget::json::OrderStatus{bitget::json::OrderStatus::CANCELLED}} == roq::OrderStatus::CANCELED);

template <>
template <>
std::optional<roq::OrderStatus> Map<bitget::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// bitget::json::OrderType => roq::OrderType

template <>
template <>
constexpr Helper<bitget::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::OrderType::type_t;
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

static_assert(Helper{bitget::json::OrderType{bitget::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{bitget::json::OrderType{bitget::json::OrderType::MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{bitget::json::OrderType{bitget::json::OrderType::LIMIT}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<bitget::json::OrderType>::helper() const {
  return Helper{args_};
}

// {bitget::json::PosSide, bitget::json::Side} => roq::PositionEffect

template <>
template <>
constexpr Helper<bitget::json::PosSide, bitget::json::Side>::operator std::optional<roq::PositionEffect>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::PosSide::type_t;
    case UNDEFINED_INTERNAL:
      return roq::PositionEffect::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::PositionEffect::UNDEFINED;
    case LONG:
      switch (std::get<1>(args_)) {
        using enum bitget::json::Side::type_t;
        case UNDEFINED_INTERNAL:
          return roq::PositionEffect::UNDEFINED;
        case UNKNOWN_INTERNAL:
          return roq::PositionEffect::UNDEFINED;
        case BUY:
          return roq::PositionEffect::OPEN;
        case SELL:
          return roq::PositionEffect::CLOSE;
      }
      break;
    case SHORT:
      switch (std::get<1>(args_)) {
        using enum bitget::json::Side::type_t;
        case UNDEFINED_INTERNAL:
          return roq::PositionEffect::UNDEFINED;
        case UNKNOWN_INTERNAL:
          return roq::PositionEffect::UNDEFINED;
        case BUY:
          return roq::PositionEffect::CLOSE;
        case SELL:
          return roq::PositionEffect::OPEN;
      }
      break;
    case NET:
      return roq::PositionEffect::UNDEFINED;
  }
  return {};
}

static_assert(
    Helper{bitget::json::PosSide{bitget::json::PosSide::UNDEFINED_INTERNAL}, bitget::json::Side{bitget::json::Side::UNDEFINED_INTERNAL}} ==
    roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::PosSide{bitget::json::PosSide::LONG}, bitget::json::Side{bitget::json::Side::BUY}} == roq::PositionEffect::OPEN);
static_assert(Helper{bitget::json::PosSide{bitget::json::PosSide::LONG}, bitget::json::Side{bitget::json::Side::SELL}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::PosSide{bitget::json::PosSide::SHORT}, bitget::json::Side{bitget::json::Side::BUY}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::PosSide{bitget::json::PosSide::SHORT}, bitget::json::Side{bitget::json::Side::SELL}} == roq::PositionEffect::OPEN);
static_assert(Helper{bitget::json::PosSide{bitget::json::PosSide::NET}, bitget::json::Side{bitget::json::Side::BUY}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::PosSide{bitget::json::PosSide::NET}, bitget::json::Side{bitget::json::Side::SELL}} == roq::PositionEffect::UNDEFINED);

template <>
template <>
std::optional<roq::PositionEffect> Map<bitget::json::PosSide, bitget::json::Side>::helper() const {
  return Helper{args_};
}

// bitget::json::Side => roq::Side

template <>
template <>
constexpr Helper<bitget::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::Side::type_t;
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

static_assert(Helper{bitget::json::Side{bitget::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{bitget::json::Side{bitget::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{bitget::json::Side{bitget::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<bitget::json::Side>::helper() const {
  return Helper{args_};
}

// bitget::json::TimeInForce => roq::TimeInForce

template <>
template <>
constexpr Helper<bitget::json::TimeInForce>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::TimeInForce::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case IOC:
      return roq::TimeInForce::IOC;
    case FOK:
      return roq::TimeInForce::FOK;
    case GTC:
      return roq::TimeInForce::GTC;
    case POST_ONLY:
      return roq::TimeInForce::GTC;
  }
  return {};
}

static_assert(Helper{bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{bitget::json::TimeInForce{bitget::json::TimeInForce::IOC}} == roq::TimeInForce::IOC);
static_assert(Helper{bitget::json::TimeInForce{bitget::json::TimeInForce::FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{bitget::json::TimeInForce{bitget::json::TimeInForce::GTC}} == roq::TimeInForce::GTC);
static_assert(Helper{bitget::json::TimeInForce{bitget::json::TimeInForce::POST_ONLY}} == roq::TimeInForce::GTC);

template <>
template <>
std::optional<roq::TimeInForce> Map<bitget::json::TimeInForce>::helper() const {
  return Helper{args_};
}

// bitget::json::TradeScope => roq::Liquidity

template <>
template <>
constexpr Helper<bitget::json::TradeScope>::operator std::optional<roq::Liquidity>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::TradeScope::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Liquidity::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Liquidity::UNDEFINED;
    case TAKER:
      return roq::Liquidity::TAKER;
    case MAKER:
      return roq::Liquidity::MAKER;
  }
  return {};
}

static_assert(Helper{bitget::json::TradeScope{bitget::json::TradeScope::UNDEFINED_INTERNAL}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{bitget::json::TradeScope{bitget::json::TradeScope::TAKER}} == roq::Liquidity::TAKER);
static_assert(Helper{bitget::json::TradeScope{bitget::json::TradeScope::MAKER}} == roq::Liquidity::MAKER);

template <>
template <>
std::optional<roq::Liquidity> Map<bitget::json::TradeScope>::helper() const {
  return Helper{args_};
}

// bitget::json::TradeSide => roq::PositionEffect

template <>
template <>
constexpr Helper<bitget::json::TradeSide>::operator std::optional<roq::PositionEffect>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::TradeSide::type_t;
    case UNDEFINED_INTERNAL:
      return roq::PositionEffect::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::PositionEffect::UNDEFINED;
    case CLOSE:
      return roq::PositionEffect::CLOSE;
    case OPEN:
      return roq::PositionEffect::OPEN;
    case REDUCE_CLOSE_LONG:
      return roq::PositionEffect::CLOSE;
    case REDUCE_CLOSE_SHORT:
      return roq::PositionEffect::CLOSE;
    case BURST_CLOSE_LONG:
      return roq::PositionEffect::CLOSE;
    case BURST_CLOSE_SHORT:
      return roq::PositionEffect::CLOSE;
    case OFFSET_CLOSE_LONG:
      return roq::PositionEffect::CLOSE;
    case OFFSET_CLOSE_SHORT:
      return roq::PositionEffect::CLOSE;
    case DELIVERY_CLOSE_LONG:
      return roq::PositionEffect::CLOSE;
    case DELIVERY_CLOSE_SHORT:
      return roq::PositionEffect::CLOSE;
    case DTE_SYS_ADL_CLOSE_LONG:
      return roq::PositionEffect::CLOSE;
    case DTE_SYS_ADL_CLOSE_SHORT:
      return roq::PositionEffect::CLOSE;
    case BUY_SINGLE:
      return roq::PositionEffect::UNDEFINED;
    case SELL_SINGLE:
      return roq::PositionEffect::UNDEFINED;
    case REDUCE_BUY_SINGLE:
      return roq::PositionEffect::UNDEFINED;
    case REDUCE_SELL_SINGLE:
      return roq::PositionEffect::UNDEFINED;
    case BURST_BUY_SINGLE:
      return roq::PositionEffect::UNDEFINED;
    case BURST_SELL_SINGLE:
      return roq::PositionEffect::UNDEFINED;
    case DELIVERY_SELL_SINGLE:
      return roq::PositionEffect::UNDEFINED;
    case DELIVERY_BUY_SINGLE:
      return roq::PositionEffect::UNDEFINED;
    case DTE_SYS_ADL_BUY_IN_SINGLE_SIDE_MODE:
      return roq::PositionEffect::UNDEFINED;
    case DTE_SYS_ADL_SELL_IN_SINGLE_SIDE_MODE:
      return roq::PositionEffect::UNDEFINED;
    case OPEN_SHORT:
      return roq::PositionEffect::OPEN;
    case OPEN_LONG:
      return roq::PositionEffect::OPEN;
    case CLOSE_SHORT:
      return roq::PositionEffect::CLOSE;
    case CLOSE_LONG:
      return roq::PositionEffect::CLOSE;
  }
  return {};
}

static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::UNDEFINED_INTERNAL}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::CLOSE}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::OPEN}} == roq::PositionEffect::OPEN);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::REDUCE_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::REDUCE_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::BURST_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::BURST_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::OFFSET_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::OFFSET_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::DELIVERY_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::DELIVERY_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::DTE_SYS_ADL_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::DTE_SYS_ADL_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::BUY_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::SELL_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::REDUCE_BUY_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::REDUCE_SELL_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::BURST_BUY_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::BURST_SELL_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::DELIVERY_SELL_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::DELIVERY_BUY_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::DTE_SYS_ADL_BUY_IN_SINGLE_SIDE_MODE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::DTE_SYS_ADL_SELL_IN_SINGLE_SIDE_MODE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::OPEN_SHORT}} == roq::PositionEffect::OPEN);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::OPEN_SHORT}} == roq::PositionEffect::OPEN);
static_assert(Helper{bitget::json::TradeSide{bitget::json::TradeSide::CLOSE_LONG}} == roq::PositionEffect::CLOSE);

template <>
template <>
std::optional<roq::PositionEffect> Map<bitget::json::TradeSide>::helper() const {
  return Helper{args_};
}

// bitget::json::TradingStatus => roq::TradingStatus

template <>
template <>
constexpr Helper<bitget::json::TradingStatus>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::json::TradingStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TradingStatus::UNDEFINED;
    case LISTED:
      return roq::TradingStatus::UNDEFINED;
    case ONLINE:
      return roq::TradingStatus::OPEN;
    case LIMIT_OPEN:
      return roq::TradingStatus::PRE_OPEN;
    case OFFLINE:
      return roq::TradingStatus::CLOSE;
    case RESTRICTED_API:
      return roq::TradingStatus::HALT;
  }
  return {};
}

static_assert(Helper{bitget::json::TradingStatus{bitget::json::TradingStatus::UNDEFINED_INTERNAL}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitget::json::TradingStatus{bitget::json::TradingStatus::LISTED}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitget::json::TradingStatus{bitget::json::TradingStatus::ONLINE}} == roq::TradingStatus::OPEN);
static_assert(Helper{bitget::json::TradingStatus{bitget::json::TradingStatus::LIMIT_OPEN}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{bitget::json::TradingStatus{bitget::json::TradingStatus::OFFLINE}} == roq::TradingStatus::CLOSE);
static_assert(Helper{bitget::json::TradingStatus{bitget::json::TradingStatus::RESTRICTED_API}} == roq::TradingStatus::HALT);

template <>
template <>
std::optional<roq::TradingStatus> Map<bitget::json::TradingStatus>::helper() const {
  return Helper{args_};
}

// roq => bitget::json

// roq::MarginMode => bitget::json::MarginMode

template <>
template <>
constexpr Helper<roq::MarginMode>::operator std::optional<bitget::json::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum roq::MarginMode;
    case UNDEFINED:
      return bitget::json::MarginMode::UNDEFINED_INTERNAL;
    case CROSS:
      return bitget::json::MarginMode::CROSSED;
    case ISOLATED:
      return bitget::json::MarginMode::ISOLATED;
    case PORTFOLIO:
      return bitget::json::MarginMode::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::MarginMode::UNDEFINED} == bitget::json::MarginMode{bitget::json::MarginMode::UNDEFINED_INTERNAL});
static_assert(Helper{roq::MarginMode::CROSS} == bitget::json::MarginMode{bitget::json::MarginMode::CROSSED});
static_assert(Helper{roq::MarginMode::ISOLATED} == bitget::json::MarginMode{bitget::json::MarginMode::ISOLATED});
static_assert(Helper{roq::MarginMode::PORTFOLIO} == bitget::json::MarginMode{bitget::json::MarginMode::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<bitget::json::MarginMode> Map<roq::MarginMode>::helper() const {
  return Helper{args_};
}

// roq::OrderType => bitget::json::OrderType

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<bitget::json::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return bitget::json::OrderType::UNDEFINED_INTERNAL;
    case MARKET:
      return bitget::json::OrderType::MARKET;
    case LIMIT:
      return bitget::json::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == bitget::json::OrderType{bitget::json::OrderType::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderType::MARKET} == bitget::json::OrderType{bitget::json::OrderType::MARKET});
static_assert(Helper{roq::OrderType::LIMIT} == bitget::json::OrderType{bitget::json::OrderType::LIMIT});

template <>
template <>
std::optional<bitget::json::OrderType> Map<roq::OrderType>::helper() const {
  return Helper{args_};
}

// {roq::PositionEffect, roq::Side} => bitget::json::PosSide

template <>
template <>
constexpr Helper<roq::PositionEffect, roq::Side>::operator std::optional<bitget::json::PosSide>() const {
  switch (std::get<0>(args_)) {
    using enum roq::PositionEffect;
    case UNDEFINED:
      return bitget::json::PosSide::UNDEFINED_INTERNAL;
    case OPEN:
      switch (std::get<1>(args_)) {
        using enum roq::Side;
        case UNDEFINED:
          return bitget::json::PosSide::UNDEFINED_INTERNAL;
        case BUY:
          return bitget::json::PosSide::LONG;
        case SELL:
          return bitget::json::PosSide::SHORT;
      }
      break;
    case CLOSE:
      switch (std::get<1>(args_)) {
        using enum roq::Side;
        case UNDEFINED:
          return bitget::json::PosSide::UNDEFINED_INTERNAL;
        case BUY:
          return bitget::json::PosSide::SHORT;
        case SELL:
          return bitget::json::PosSide::LONG;
      }
      break;
  }
  return {};
}

static_assert(Helper{roq::PositionEffect::UNDEFINED, roq::Side::UNDEFINED} == bitget::json::PosSide{bitget::json::PosSide::UNDEFINED_INTERNAL});
static_assert(Helper{roq::PositionEffect::OPEN, roq::Side::BUY} == bitget::json::PosSide{bitget::json::PosSide::LONG});
static_assert(Helper{roq::PositionEffect::OPEN, roq::Side::SELL} == bitget::json::PosSide{bitget::json::PosSide::SHORT});
static_assert(Helper{roq::PositionEffect::CLOSE, roq::Side::BUY} == bitget::json::PosSide{bitget::json::PosSide::SHORT});
static_assert(Helper{roq::PositionEffect::CLOSE, roq::Side::SELL} == bitget::json::PosSide{bitget::json::PosSide::LONG});

template <>
template <>
std::optional<bitget::json::PosSide> Map<roq::PositionEffect, roq::Side>::helper() const {
  return Helper{args_};
}

// roq::PositionEffect => bitget::json::TradeSide

template <>
template <>
constexpr Helper<roq::PositionEffect>::operator std::optional<bitget::json::TradeSide>() const {
  switch (std::get<0>(args_)) {
    using enum roq::PositionEffect;
    case UNDEFINED:
      return bitget::json::TradeSide::UNDEFINED_INTERNAL;
    case OPEN:
      return bitget::json::TradeSide::OPEN;
    case CLOSE:
      return bitget::json::TradeSide::CLOSE;
  }
  return {};
}

static_assert(Helper{roq::PositionEffect::UNDEFINED} == bitget::json::TradeSide{bitget::json::TradeSide::UNDEFINED_INTERNAL});
static_assert(Helper{roq::PositionEffect::OPEN} == bitget::json::TradeSide{bitget::json::TradeSide::OPEN});
static_assert(Helper{roq::PositionEffect::CLOSE} == bitget::json::TradeSide{bitget::json::TradeSide::CLOSE});

template <>
template <>
std::optional<bitget::json::TradeSide> Map<roq::PositionEffect>::helper() const {
  return Helper{args_};
}

// roq::Side => bitget::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<bitget::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return bitget::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return bitget::json::Side::BUY;
    case SELL:
      return bitget::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == bitget::json::Side{bitget::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == bitget::json::Side{bitget::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == bitget::json::Side{bitget::json::Side::SELL});

template <>
template <>
std::optional<bitget::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// roq::TimeInForce => bitget::json::Force

// POST_ONLY ???
template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<bitget::json::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFD:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTC:
      return bitget::json::TimeInForce::GTC;
    case OPG:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case IOC:
      return bitget::json::TimeInForce::IOC;
    case FOK:
      return bitget::json::TimeInForce::FOK;
    case GTX:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTD:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_THE_CLOSE:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_THROUGH_CROSSING:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_CROSSING:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_FOR_TIME:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFA:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFM:
      return bitget::json::TimeInForce::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFD} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTC} == bitget::json::TimeInForce{bitget::json::TimeInForce::GTC});
static_assert(Helper{roq::TimeInForce::OPG} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::IOC} == bitget::json::TimeInForce{bitget::json::TimeInForce::IOC});
static_assert(Helper{roq::TimeInForce::FOK} == bitget::json::TimeInForce{bitget::json::TimeInForce::FOK});
static_assert(Helper{roq::TimeInForce::GTX} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTD} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_FOR_TIME} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFA} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFM} == bitget::json::TimeInForce{bitget::json::TimeInForce::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<bitget::json::TimeInForce> Map<roq::TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
