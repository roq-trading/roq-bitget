/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/protocol/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// bitget::json => roq

// bitget::protocol::json::Action => roq::MarginMode

template <>
template <>
constexpr Helper<bitget::protocol::json::Action>::operator std::optional<roq::UpdateType>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::Action::type_t;
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

static_assert(Helper{bitget::protocol::json::Action{bitget::protocol::json::Action::UNDEFINED_INTERNAL}} == roq::UpdateType::UNDEFINED);
static_assert(Helper{bitget::protocol::json::Action{bitget::protocol::json::Action::SNAPSHOT}} == roq::UpdateType::SNAPSHOT);
static_assert(Helper{bitget::protocol::json::Action{bitget::protocol::json::Action::UPDATE}} == roq::UpdateType::INCREMENTAL);

template <>
template <>
std::optional<roq::UpdateType> Map<bitget::protocol::json::Action>::helper() const {
  return Helper{args_};
}

// bitget::protocol::json::AssetMode => roq::MarginMode

template <>
template <>
constexpr Helper<bitget::protocol::json::AssetMode>::operator std::optional<roq::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::AssetMode::type_t;
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

static_assert(Helper{bitget::protocol::json::AssetMode{bitget::protocol::json::AssetMode::UNDEFINED_INTERNAL}} == roq::MarginMode::UNDEFINED);
static_assert(Helper{bitget::protocol::json::AssetMode{bitget::protocol::json::AssetMode::SINGLE}} == roq::MarginMode::ISOLATED);
static_assert(Helper{bitget::protocol::json::AssetMode{bitget::protocol::json::AssetMode::UNION}} == roq::MarginMode::CROSS);
static_assert(Helper{bitget::protocol::json::AssetMode{bitget::protocol::json::AssetMode::MULTI_ASSETS}} == roq::MarginMode::CROSS);

template <>
template <>
std::optional<roq::MarginMode> Map<bitget::protocol::json::AssetMode>::helper() const {
  return Helper{args_};
}

// {bitget::protocol::json::Category, bitget::protocol::json::FuturesType} => roq::SecurityType

template <>
template <>
constexpr Helper<bitget::protocol::json::Category, bitget::protocol::json::FuturesType>::operator std::optional<roq::SecurityType>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::Category::type_t;
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
        using enum bitget::protocol::json::FuturesType::type_t;
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
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::UNDEFINED_INTERNAL},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::UNDEFINED_INTERNAL}} == roq::SecurityType::UNDEFINED);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::SPOT},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::UNDEFINED_INTERNAL}} == roq::SecurityType::SPOT);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::MARGIN},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::UNDEFINED_INTERNAL}} == roq::SecurityType::SPOT);

static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::USDT_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::UNDEFINED_INTERNAL}} == roq::SecurityType::UNDEFINED);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::USDT_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::UNKNOWN_INTERNAL}} == roq::SecurityType::UNDEFINED);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::USDT_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::PERPETUAL}} == roq::SecurityType::SWAP);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::USDT_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::DELIVERY}} == roq::SecurityType::FUTURES);

static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::USDC_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::UNDEFINED_INTERNAL}} == roq::SecurityType::UNDEFINED);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::USDC_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::UNKNOWN_INTERNAL}} == roq::SecurityType::UNDEFINED);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::USDC_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::PERPETUAL}} == roq::SecurityType::SWAP);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::USDC_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::DELIVERY}} == roq::SecurityType::FUTURES);

static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::COIN_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::UNDEFINED_INTERNAL}} == roq::SecurityType::UNDEFINED);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::COIN_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::UNKNOWN_INTERNAL}} == roq::SecurityType::UNDEFINED);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::COIN_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::PERPETUAL}} == roq::SecurityType::SWAP);
static_assert(
    Helper{
        bitget::protocol::json::Category{bitget::protocol::json::Category::COIN_FUTURES},
        bitget::protocol::json::FuturesType{bitget::protocol::json::FuturesType::DELIVERY}} == roq::SecurityType::FUTURES);

template <>
template <>
std::optional<roq::SecurityType> Map<bitget::protocol::json::Category, bitget::protocol::json::FuturesType>::helper() const {
  return Helper{args_};
}

// bitget::protocol::json::MarginMode => roq::MarginMode

template <>
template <>
constexpr Helper<bitget::protocol::json::MarginMode>::operator std::optional<roq::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::MarginMode::type_t;
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

static_assert(Helper{bitget::protocol::json::MarginMode{bitget::protocol::json::MarginMode::UNDEFINED_INTERNAL}} == roq::MarginMode::UNDEFINED);
static_assert(Helper{bitget::protocol::json::MarginMode{bitget::protocol::json::MarginMode::CROSSED}} == roq::MarginMode::CROSS);
static_assert(Helper{bitget::protocol::json::MarginMode{bitget::protocol::json::MarginMode::ISOLATED}} == roq::MarginMode::ISOLATED);

template <>
template <>
std::optional<roq::MarginMode> Map<bitget::protocol::json::MarginMode>::helper() const {
  return Helper{args_};
}

// bitget::protocol::json::OrderStatus => roq::OrderStatus

template <>
template <>
constexpr Helper<bitget::protocol::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::OrderStatus::type_t;
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

static_assert(Helper{bitget::protocol::json::OrderStatus{bitget::protocol::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{bitget::protocol::json::OrderStatus{bitget::protocol::json::OrderStatus::LIVE}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitget::protocol::json::OrderStatus{bitget::protocol::json::OrderStatus::NEW}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitget::protocol::json::OrderStatus{bitget::protocol::json::OrderStatus::PARTIALLY_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{bitget::protocol::json::OrderStatus{bitget::protocol::json::OrderStatus::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{bitget::protocol::json::OrderStatus{bitget::protocol::json::OrderStatus::CANCELLED}} == roq::OrderStatus::CANCELED);

template <>
template <>
std::optional<roq::OrderStatus> Map<bitget::protocol::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// bitget::protocol::json::OrderType => roq::OrderType

template <>
template <>
constexpr Helper<bitget::protocol::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::OrderType::type_t;
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

static_assert(Helper{bitget::protocol::json::OrderType{bitget::protocol::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{bitget::protocol::json::OrderType{bitget::protocol::json::OrderType::MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{bitget::protocol::json::OrderType{bitget::protocol::json::OrderType::LIMIT}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<bitget::protocol::json::OrderType>::helper() const {
  return Helper{args_};
}

// {bitget::protocol::json::PosSide, bitget::protocol::json::Side} => roq::PositionEffect

template <>
template <>
constexpr Helper<bitget::protocol::json::PosSide, bitget::protocol::json::Side>::operator std::optional<roq::PositionEffect>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::PosSide::type_t;
    case UNDEFINED_INTERNAL:
      return roq::PositionEffect::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::PositionEffect::UNDEFINED;
    case LONG:
      switch (std::get<1>(args_)) {
        using enum bitget::protocol::json::Side::type_t;
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
        using enum bitget::protocol::json::Side::type_t;
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
    Helper{
        bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::UNDEFINED_INTERNAL},
        bitget::protocol::json::Side{bitget::protocol::json::Side::UNDEFINED_INTERNAL}} == roq::PositionEffect::UNDEFINED);
static_assert(
    Helper{bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::LONG}, bitget::protocol::json::Side{bitget::protocol::json::Side::BUY}} ==
    roq::PositionEffect::OPEN);
static_assert(
    Helper{bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::LONG}, bitget::protocol::json::Side{bitget::protocol::json::Side::SELL}} ==
    roq::PositionEffect::CLOSE);
static_assert(
    Helper{bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::SHORT}, bitget::protocol::json::Side{bitget::protocol::json::Side::BUY}} ==
    roq::PositionEffect::CLOSE);
static_assert(
    Helper{bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::SHORT}, bitget::protocol::json::Side{bitget::protocol::json::Side::SELL}} ==
    roq::PositionEffect::OPEN);
static_assert(
    Helper{bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::NET}, bitget::protocol::json::Side{bitget::protocol::json::Side::BUY}} ==
    roq::PositionEffect::UNDEFINED);
static_assert(
    Helper{bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::NET}, bitget::protocol::json::Side{bitget::protocol::json::Side::SELL}} ==
    roq::PositionEffect::UNDEFINED);

template <>
template <>
std::optional<roq::PositionEffect> Map<bitget::protocol::json::PosSide, bitget::protocol::json::Side>::helper() const {
  return Helper{args_};
}

// bitget::protocol::json::Side => roq::Side

template <>
template <>
constexpr Helper<bitget::protocol::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::Side::type_t;
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

static_assert(Helper{bitget::protocol::json::Side{bitget::protocol::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{bitget::protocol::json::Side{bitget::protocol::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{bitget::protocol::json::Side{bitget::protocol::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<bitget::protocol::json::Side>::helper() const {
  return Helper{args_};
}

// bitget::protocol::json::TimeInForce => roq::TimeInForce

template <>
template <>
constexpr Helper<bitget::protocol::json::TimeInForce>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::TimeInForce::type_t;
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

static_assert(Helper{bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::IOC}} == roq::TimeInForce::IOC);
static_assert(Helper{bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::GTC}} == roq::TimeInForce::GTC);
static_assert(Helper{bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::POST_ONLY}} == roq::TimeInForce::GTC);

template <>
template <>
std::optional<roq::TimeInForce> Map<bitget::protocol::json::TimeInForce>::helper() const {
  return Helper{args_};
}

// bitget::protocol::json::TradeScope => roq::Liquidity

template <>
template <>
constexpr Helper<bitget::protocol::json::TradeScope>::operator std::optional<roq::Liquidity>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::TradeScope::type_t;
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

static_assert(Helper{bitget::protocol::json::TradeScope{bitget::protocol::json::TradeScope::UNDEFINED_INTERNAL}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeScope{bitget::protocol::json::TradeScope::TAKER}} == roq::Liquidity::TAKER);
static_assert(Helper{bitget::protocol::json::TradeScope{bitget::protocol::json::TradeScope::MAKER}} == roq::Liquidity::MAKER);

template <>
template <>
std::optional<roq::Liquidity> Map<bitget::protocol::json::TradeScope>::helper() const {
  return Helper{args_};
}

// bitget::protocol::json::TradeSide => roq::PositionEffect

template <>
template <>
constexpr Helper<bitget::protocol::json::TradeSide>::operator std::optional<roq::PositionEffect>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::TradeSide::type_t;
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

static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::UNDEFINED_INTERNAL}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::CLOSE}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::OPEN}} == roq::PositionEffect::OPEN);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::REDUCE_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::REDUCE_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::BURST_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::BURST_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::OFFSET_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::OFFSET_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::DELIVERY_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::DELIVERY_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::DTE_SYS_ADL_CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::DTE_SYS_ADL_CLOSE_SHORT}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::BUY_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::SELL_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::REDUCE_BUY_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::REDUCE_SELL_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::BURST_BUY_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::BURST_SELL_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::DELIVERY_SELL_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::DELIVERY_BUY_SINGLE}} == roq::PositionEffect::UNDEFINED);
static_assert(
    Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::DTE_SYS_ADL_BUY_IN_SINGLE_SIDE_MODE}} == roq::PositionEffect::UNDEFINED);
static_assert(
    Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::DTE_SYS_ADL_SELL_IN_SINGLE_SIDE_MODE}} == roq::PositionEffect::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::OPEN_SHORT}} == roq::PositionEffect::OPEN);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::CLOSE_LONG}} == roq::PositionEffect::CLOSE);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::OPEN_SHORT}} == roq::PositionEffect::OPEN);
static_assert(Helper{bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::CLOSE_LONG}} == roq::PositionEffect::CLOSE);

template <>
template <>
std::optional<roq::PositionEffect> Map<bitget::protocol::json::TradeSide>::helper() const {
  return Helper{args_};
}

// bitget::protocol::json::TradingStatus => roq::TradingStatus

template <>
template <>
constexpr Helper<bitget::protocol::json::TradingStatus>::operator std::optional<roq::TradingStatus>() const {
  switch (std::get<0>(args_)) {
    using enum bitget::protocol::json::TradingStatus::type_t;
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
    case LIMIT_CLOSE:
      return roq::TradingStatus::CLOSE;
    case OFFLINE:
      return roq::TradingStatus::CLOSE;
    case RESTRICTED_API:
      return roq::TradingStatus::HALT;
  }
  return {};
}

static_assert(Helper{bitget::protocol::json::TradingStatus{bitget::protocol::json::TradingStatus::UNDEFINED_INTERNAL}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradingStatus{bitget::protocol::json::TradingStatus::LISTED}} == roq::TradingStatus::UNDEFINED);
static_assert(Helper{bitget::protocol::json::TradingStatus{bitget::protocol::json::TradingStatus::ONLINE}} == roq::TradingStatus::OPEN);
static_assert(Helper{bitget::protocol::json::TradingStatus{bitget::protocol::json::TradingStatus::LIMIT_OPEN}} == roq::TradingStatus::PRE_OPEN);
static_assert(Helper{bitget::protocol::json::TradingStatus{bitget::protocol::json::TradingStatus::LIMIT_CLOSE}} == roq::TradingStatus::CLOSE);
static_assert(Helper{bitget::protocol::json::TradingStatus{bitget::protocol::json::TradingStatus::OFFLINE}} == roq::TradingStatus::CLOSE);
static_assert(Helper{bitget::protocol::json::TradingStatus{bitget::protocol::json::TradingStatus::RESTRICTED_API}} == roq::TradingStatus::HALT);

template <>
template <>
std::optional<roq::TradingStatus> Map<bitget::protocol::json::TradingStatus>::helper() const {
  return Helper{args_};
}

// roq => bitget::json

// roq::MarginMode => bitget::protocol::json::MarginMode

template <>
template <>
constexpr Helper<roq::MarginMode>::operator std::optional<bitget::protocol::json::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum roq::MarginMode;
    case UNDEFINED:
      return bitget::protocol::json::MarginMode::UNDEFINED_INTERNAL;
    case CROSS:
      return bitget::protocol::json::MarginMode::CROSSED;
    case ISOLATED:
      return bitget::protocol::json::MarginMode::ISOLATED;
    case PORTFOLIO:
      return bitget::protocol::json::MarginMode::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::MarginMode::UNDEFINED} == bitget::protocol::json::MarginMode{bitget::protocol::json::MarginMode::UNDEFINED_INTERNAL});
static_assert(Helper{roq::MarginMode::CROSS} == bitget::protocol::json::MarginMode{bitget::protocol::json::MarginMode::CROSSED});
static_assert(Helper{roq::MarginMode::ISOLATED} == bitget::protocol::json::MarginMode{bitget::protocol::json::MarginMode::ISOLATED});
static_assert(Helper{roq::MarginMode::PORTFOLIO} == bitget::protocol::json::MarginMode{bitget::protocol::json::MarginMode::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<bitget::protocol::json::MarginMode> Map<roq::MarginMode>::helper() const {
  return Helper{args_};
}

// roq::OrderType => bitget::protocol::json::OrderType

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<bitget::protocol::json::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return bitget::protocol::json::OrderType::UNDEFINED_INTERNAL;
    case MARKET:
      return bitget::protocol::json::OrderType::MARKET;
    case LIMIT:
      return bitget::protocol::json::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == bitget::protocol::json::OrderType{bitget::protocol::json::OrderType::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderType::MARKET} == bitget::protocol::json::OrderType{bitget::protocol::json::OrderType::MARKET});
static_assert(Helper{roq::OrderType::LIMIT} == bitget::protocol::json::OrderType{bitget::protocol::json::OrderType::LIMIT});

template <>
template <>
std::optional<bitget::protocol::json::OrderType> Map<roq::OrderType>::helper() const {
  return Helper{args_};
}

// {roq::PositionEffect, roq::Side} => bitget::protocol::json::PosSide

template <>
template <>
constexpr Helper<roq::PositionEffect, roq::Side>::operator std::optional<bitget::protocol::json::PosSide>() const {
  switch (std::get<0>(args_)) {
    using enum roq::PositionEffect;
    case UNDEFINED:
      return bitget::protocol::json::PosSide::UNDEFINED_INTERNAL;
    case OPEN:
      switch (std::get<1>(args_)) {
        using enum roq::Side;
        case UNDEFINED:
          return bitget::protocol::json::PosSide::UNDEFINED_INTERNAL;
        case BUY:
          return bitget::protocol::json::PosSide::LONG;
        case SELL:
          return bitget::protocol::json::PosSide::SHORT;
      }
      break;
    case CLOSE:
      switch (std::get<1>(args_)) {
        using enum roq::Side;
        case UNDEFINED:
          return bitget::protocol::json::PosSide::UNDEFINED_INTERNAL;
        case BUY:
          return bitget::protocol::json::PosSide::SHORT;
        case SELL:
          return bitget::protocol::json::PosSide::LONG;
      }
      break;
  }
  return {};
}

static_assert(
    Helper{roq::PositionEffect::UNDEFINED, roq::Side::UNDEFINED} == bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::UNDEFINED_INTERNAL});
static_assert(Helper{roq::PositionEffect::OPEN, roq::Side::BUY} == bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::LONG});
static_assert(Helper{roq::PositionEffect::OPEN, roq::Side::SELL} == bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::SHORT});
static_assert(Helper{roq::PositionEffect::CLOSE, roq::Side::BUY} == bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::SHORT});
static_assert(Helper{roq::PositionEffect::CLOSE, roq::Side::SELL} == bitget::protocol::json::PosSide{bitget::protocol::json::PosSide::LONG});

template <>
template <>
std::optional<bitget::protocol::json::PosSide> Map<roq::PositionEffect, roq::Side>::helper() const {
  return Helper{args_};
}

// roq::PositionEffect => bitget::protocol::json::TradeSide

template <>
template <>
constexpr Helper<roq::PositionEffect>::operator std::optional<bitget::protocol::json::TradeSide>() const {
  switch (std::get<0>(args_)) {
    using enum roq::PositionEffect;
    case UNDEFINED:
      return bitget::protocol::json::TradeSide::UNDEFINED_INTERNAL;
    case OPEN:
      return bitget::protocol::json::TradeSide::OPEN;
    case CLOSE:
      return bitget::protocol::json::TradeSide::CLOSE;
  }
  return {};
}

static_assert(Helper{roq::PositionEffect::UNDEFINED} == bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::UNDEFINED_INTERNAL});
static_assert(Helper{roq::PositionEffect::OPEN} == bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::OPEN});
static_assert(Helper{roq::PositionEffect::CLOSE} == bitget::protocol::json::TradeSide{bitget::protocol::json::TradeSide::CLOSE});

template <>
template <>
std::optional<bitget::protocol::json::TradeSide> Map<roq::PositionEffect>::helper() const {
  return Helper{args_};
}

// roq::Side => bitget::protocol::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<bitget::protocol::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return bitget::protocol::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return bitget::protocol::json::Side::BUY;
    case SELL:
      return bitget::protocol::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == bitget::protocol::json::Side{bitget::protocol::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == bitget::protocol::json::Side{bitget::protocol::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == bitget::protocol::json::Side{bitget::protocol::json::Side::SELL});

template <>
template <>
std::optional<bitget::protocol::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

// roq::TimeInForce => bitget::protocol::json::Force

// POST_ONLY ???
template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<bitget::protocol::json::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFD:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTC:
      return bitget::protocol::json::TimeInForce::GTC;
    case OPG:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case IOC:
      return bitget::protocol::json::TimeInForce::IOC;
    case FOK:
      return bitget::protocol::json::TimeInForce::FOK;
    case GTX:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTD:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_THE_CLOSE:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_THROUGH_CROSSING:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_CROSSING:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_FOR_TIME:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFA:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFM:
      return bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFD} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTC} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::GTC});
static_assert(Helper{roq::TimeInForce::OPG} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::IOC} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::IOC});
static_assert(Helper{roq::TimeInForce::FOK} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::FOK});
static_assert(Helper{roq::TimeInForce::GTX} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTD} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_FOR_TIME} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFA} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFM} == bitget::protocol::json::TimeInForce{bitget::protocol::json::TimeInForce::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<bitget::protocol::json::TimeInForce> Map<roq::TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
