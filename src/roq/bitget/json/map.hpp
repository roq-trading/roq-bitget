/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/map.hpp"

#include "roq/liquidity.hpp"
#include "roq/margin_mode.hpp"
#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/position_effect.hpp"
#include "roq/security_type.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"
#include "roq/trading_status.hpp"
#include "roq/update_type.hpp"

#include "roq/bitget/json/action.hpp"
#include "roq/bitget/json/asset_mode.hpp"
#include "roq/bitget/json/category.hpp"
#include "roq/bitget/json/futures_type.hpp"
#include "roq/bitget/json/margin_mode.hpp"
#include "roq/bitget/json/order_status.hpp"
#include "roq/bitget/json/order_type.hpp"
#include "roq/bitget/json/pos_side.hpp"
#include "roq/bitget/json/side.hpp"
#include "roq/bitget/json/time_in_force.hpp"
#include "roq/bitget/json/trade_scope.hpp"
#include "roq/bitget/json/trade_side.hpp"
#include "roq/bitget/json/trading_status.hpp"

namespace roq {

// bitget::json => roq

template <>
template <>
std::optional<UpdateType> Map<bitget::json::Action>::helper() const;

template <>
template <>
std::optional<MarginMode> Map<bitget::json::AssetMode>::helper() const;

template <>
template <>
std::optional<SecurityType> Map<bitget::json::Category, bitget::json::FuturesType>::helper() const;

template <>
template <>
std::optional<MarginMode> Map<bitget::json::MarginMode>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<bitget::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<bitget::json::OrderType>::helper() const;

template <>
template <>
std::optional<PositionEffect> Map<bitget::json::PosSide, bitget::json::Side>::helper() const;

template <>
template <>
std::optional<Side> Map<bitget::json::Side>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<bitget::json::TimeInForce>::helper() const;

template <>
template <>
std::optional<Liquidity> Map<bitget::json::TradeScope>::helper() const;

template <>
template <>
std::optional<PositionEffect> Map<bitget::json::TradeSide>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<bitget::json::TradingStatus>::helper() const;

// roq => bitget::json

template <>
template <>
std::optional<bitget::json::MarginMode> Map<MarginMode>::helper() const;

template <>
template <>
std::optional<bitget::json::OrderType> Map<OrderType>::helper() const;

template <>
template <>
std::optional<bitget::json::PosSide> Map<PositionEffect, Side>::helper() const;

template <>
template <>
std::optional<bitget::json::TradeSide> Map<PositionEffect>::helper() const;

template <>
template <>
std::optional<bitget::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<bitget::json::TimeInForce> Map<TimeInForce>::helper() const;

}  // namespace roq
