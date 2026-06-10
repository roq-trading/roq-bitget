/* Copyright (c) 2017-2026, Hans Erik Thrane */

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

#include "roq/bitget/protocol/json/action.hpp"
#include "roq/bitget/protocol/json/asset_mode.hpp"
#include "roq/bitget/protocol/json/category.hpp"
#include "roq/bitget/protocol/json/futures_type.hpp"
#include "roq/bitget/protocol/json/margin_mode.hpp"
#include "roq/bitget/protocol/json/order_status.hpp"
#include "roq/bitget/protocol/json/order_type.hpp"
#include "roq/bitget/protocol/json/pos_side.hpp"
#include "roq/bitget/protocol/json/side.hpp"
#include "roq/bitget/protocol/json/time_in_force.hpp"
#include "roq/bitget/protocol/json/trade_scope.hpp"
#include "roq/bitget/protocol/json/trade_side.hpp"
#include "roq/bitget/protocol/json/trading_status.hpp"

namespace roq {

// bitget::json => roq

template <>
template <>
std::optional<UpdateType> Map<bitget::protocol::json::Action>::helper() const;

template <>
template <>
std::optional<MarginMode> Map<bitget::protocol::json::AssetMode>::helper() const;

template <>
template <>
std::optional<SecurityType> Map<bitget::protocol::json::Category, bitget::protocol::json::FuturesType>::helper() const;

template <>
template <>
std::optional<MarginMode> Map<bitget::protocol::json::MarginMode>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<bitget::protocol::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<bitget::protocol::json::OrderType>::helper() const;

template <>
template <>
std::optional<PositionEffect> Map<bitget::protocol::json::PosSide, bitget::protocol::json::Side>::helper() const;

template <>
template <>
std::optional<Side> Map<bitget::protocol::json::Side>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<bitget::protocol::json::TimeInForce>::helper() const;

template <>
template <>
std::optional<Liquidity> Map<bitget::protocol::json::TradeScope>::helper() const;

template <>
template <>
std::optional<PositionEffect> Map<bitget::protocol::json::TradeSide>::helper() const;

template <>
template <>
std::optional<TradingStatus> Map<bitget::protocol::json::TradingStatus>::helper() const;

// roq => bitget::json

template <>
template <>
std::optional<bitget::protocol::json::MarginMode> Map<MarginMode>::helper() const;

template <>
template <>
std::optional<bitget::protocol::json::OrderType> Map<OrderType>::helper() const;

template <>
template <>
std::optional<bitget::protocol::json::PosSide> Map<PositionEffect, Side>::helper() const;

template <>
template <>
std::optional<bitget::protocol::json::TradeSide> Map<PositionEffect>::helper() const;

template <>
template <>
std::optional<bitget::protocol::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<bitget::protocol::json::TimeInForce> Map<TimeInForce>::helper() const;

}  // namespace roq
