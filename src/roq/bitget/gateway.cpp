/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/gateway.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

#include "roq/logging.hpp"

#include "roq/clock.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/bitget/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {

// === HELPERS ===

namespace {
template <typename R>
R create_accounts(auto &config) {
  using value_type = std::remove_cvref_t<R>;
  value_type result;
  for (auto &[_, iter] : config.accounts) {
    result.try_emplace(static_cast<std::string_view>(iter.name), std::make_unique<Account>(config, iter.name));
  }
  return result;
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  if (!shared.settings.misc.number_of_order_entry_connections) {
    log::fatal("Unexpected: --number_of_order_entry_connections={}"sv, shared.settings.misc.number_of_order_entry_connections);
  }
  for (auto &[name, account] : accounts) {
    std::vector<std::unique_ptr<OrderEntry>> order_entry;
    for (size_t i = 0; i < shared.settings.misc.number_of_order_entry_connections; ++i) {
      auto master = i == 0;
      order_entry.emplace_back(std::make_unique<OrderEntry>(gateway, context, ++stream_id, *account, shared, master));
    }
    result.try_emplace(static_cast<std::string_view>(name), std::move(order_entry));
  }
  return result;
}

template <typename R>
R create_drop_copy(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared) {
  R result;
  for (auto &[name, account] : accounts) {
    result.try_emplace(static_cast<std::string_view>(name), std::make_unique<DropCopy>(gateway, context, ++stream_id, *account, shared));
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Gateway::Gateway(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_(dispatcher), master_account_(config.get_master_account()), accounts_(create_accounts<decltype(accounts_)>(config)), context_(context),
      shared_(dispatcher, settings), rest_(*this, context_, ++stream_id_, shared_),
      order_entry_(create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_)),
      drop_copy_(create_drop_copy<decltype(drop_copy_)>(*this, context_, stream_id_, accounts_, shared_)) {
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  assert(std::empty(market_data_));
  dispatch(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  dispatch(event);
}

void Gateway::operator()(Event<Control> const &event) {
  auto &[message_info, control] = event;
  switch (control.action) {
    using enum Action;
    case UNDEFINED:
      assert(false);
      break;
    case ENABLE:
      dispatcher_(State::ENABLED);
      break;
    case DISABLE:
      dispatcher_(State::DISABLED);
      break;
  }
}

void Gateway::operator()(Event<Connected> const &) {
}

void Gateway::operator()(Event<Disconnected> const &) {
}

void Gateway::operator()(Trace<StreamStatus> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ReferenceData> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketStatus> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TopOfBook> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last, shared_.final_bids, shared_.final_asks, []([[maybe_unused]] auto &market_by_price) {});
}

void Gateway::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TradeUpdate> const &event, bool is_last, uint8_t user_id, std::string_view const &request_id) {
  dispatcher_(event, is_last, user_id, request_id);
}

void Gateway::operator()(Trace<FundsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<PositionUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Rest::SymbolsUpdate &symbols_update) {
  auto [size, start_from] = shared_.symbols(symbols_update.symbols);
  ensure_symbol_slices(size);
  for (auto &iter : market_data_) {
    (*iter).subscribe(start_from);
  }
}

void Gateway::ensure_symbol_slices(size_t size) {
  while (std::size(market_data_) < size) {
    log::debug("Create market-data (user-stream)"sv);
    auto market_data = std::make_unique<MarketData>(*this, context_, ++stream_id_, shared_, std::size(market_data_));
    MessageInfo message_info;
    Start start;
    create_event_and_dispatch(*market_data, message_info, start);
    market_data_.emplace_back(std::move(market_data));
  }
}

uint16_t Gateway::operator()(Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  if (shared_.settings.ws_api) {
    return get_drop_copy(event.value.account)(event, order, request_id);
  } else {
    return get_order_entry(event.value.account)(event, order, request_id);
  }
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  if (shared_.settings.ws_api) {
    return get_drop_copy(event.value.account)(event, order, request_id, previous_request_id);
  } else {
    return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
  }
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  if (shared_.settings.ws_api) {
    return get_drop_copy(event.value.account)(event, order, request_id, previous_request_id);
  } else {
    return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
  }
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, request_id);
}

uint16_t Gateway::operator()(Event<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(Event<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Gateway::operator()(metrics::Writer &writer) const {
  dispatch_helper(*this, writer);
}

template <typename... Args>
void Gateway::dispatch(Args &&...args) {
  dispatch_helper(*this, std::forward<Args>(args)...);
}

template <typename... Args>
void Gateway::dispatch_helper(auto &self, Args &&...args) {
  auto helper = [&](auto &target) { target(args...); };
  helper(self.rest_);
  for (auto &[_, order_entry] : self.order_entry_) {
    helper(order_entry);
  }
  for (auto &[_, drop_copy] : self.drop_copy_) {
    if (static_cast<bool>(drop_copy)) {
      helper(*drop_copy);
    }
  }
  for (auto &iter : self.market_data_) {
    helper(*iter);
  }
}

OrderEntry &Gateway::get_order_entry(std::string_view const &account) {
  auto iter = order_entry_.find(account);
  if (iter != std::end(order_entry_)) {
    return (*iter).second.get_next();
  }
  throw RuntimeError(R"(Unknown account="{}")"sv, account);
}

DropCopy &Gateway::get_drop_copy(std::string_view const &account) {
  auto iter = drop_copy_.find(account);
  if (iter != std::end(drop_copy_)) {
    return *(*iter).second;
  }
  throw RuntimeError(R"(Unknown account="{}")"sv, account);
}

// OrderEntryRR

Gateway::OrderEntryRR::OrderEntryRR(std::vector<std::unique_ptr<OrderEntry>> &&order_entry) : order_entry_{std::move(order_entry)} {
  for (auto &item : order_entry_) {
    if (item == nullptr) {
      log::fatal("HERE"sv);
    }
  }
}

template <typename... Args>
void Gateway::OrderEntryRR::operator()(Args &&...args) {
  for (auto &item : order_entry_) {
    (*item)(args...);
  }
}

template <typename... Args>
void Gateway::OrderEntryRR::operator()(Args &&...args) const {
  for (auto &item : order_entry_) {
    (*item)(args...);
  }
}

OrderEntry &Gateway::OrderEntryRR::get_next() {
  auto length = std::size(order_entry_);
  for (size_t offset = 0; offset < length; ++offset) {
    auto index = (index_ + offset) % length;
    auto &order_entry = *(order_entry_[index]);
    if (!order_entry.ready()) {
      continue;
    }
    index_ = (index + 1) % length;
    return order_entry;
  }
  throw server::oms::NotReady{"get_next"sv};
}

}  // namespace bitget
}  // namespace roq
