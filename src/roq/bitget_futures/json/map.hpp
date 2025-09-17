/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/map.hpp"

#include "roq/margin_mode.hpp"
#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/position_effect.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"

#include "roq/bitget_futures/json/asset_mode.hpp"
#include "roq/bitget_futures/json/force.hpp"
#include "roq/bitget_futures/json/margin_mode.hpp"
#include "roq/bitget_futures/json/order_status.hpp"
#include "roq/bitget_futures/json/order_type.hpp"
#include "roq/bitget_futures/json/side.hpp"
#include "roq/bitget_futures/json/trade_side.hpp"

namespace roq {

// bitget_futures::json => roq

template <>
template <>
std::optional<MarginMode> Map<bitget_futures::json::AssetMode>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<bitget_futures::json::Force>::helper() const;

template <>
template <>
std::optional<MarginMode> Map<bitget_futures::json::MarginMode>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<bitget_futures::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<bitget_futures::json::OrderType>::helper() const;

template <>
template <>
std::optional<Side> Map<bitget_futures::json::Side>::helper() const;

// roq => bitget_futures::json => roq

template <>
template <>
std::optional<bitget_futures::json::MarginMode> Map<MarginMode>::helper() const;

template <>
template <>
std::optional<bitget_futures::json::OrderType> Map<OrderType>::helper() const;

template <>
template <>
std::optional<bitget_futures::json::TradeSide> Map<PositionEffect>::helper() const;

template <>
template <>
std::optional<bitget_futures::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<bitget_futures::json::Force> Map<TimeInForce>::helper() const;

}  // namespace roq
