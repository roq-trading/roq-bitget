/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/gateway/controller.hpp"

#include "roq/logging.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/bitget/gateway/api.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace gateway {

// === CONSTANTS ===

namespace {
uint8_t const API_SPOT = 0x1;
uint8_t const API_MARGIN = 0x2;
uint8_t const API_COIN_FUTURES = 0x3;
uint8_t const API_USDT_FUTURES = 0x4;
uint8_t const API_USDC_FUTURES = 0x5;
}  // namespace

// === HELPERS ===

namespace {
template <typename R>
R create_accounts(auto &settings, auto &config) {
  using value_type = std::remove_cvref_t<R>;
  value_type result;
  for (auto &[_, iter] : config.accounts) {
    auto obj = std::make_unique<Account>(settings, config, iter.name);
    result.try_emplace(iter.name, std::move(obj));
  }
  return result;
}

template <typename R>
R create_requests(auto &config, auto &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    auto obj = std::make_unique<Request>(shared);
    result.try_emplace(account.name, std::move(obj));
  }
  return result;
}

Request &get_request(auto &requests, auto &account) {
  auto iter = requests.find(account);
  if (iter == std::end(requests)) {
    log::fatal("Unexpected"sv);
  }
  return *(*iter).second;
}

auto create_rest(auto &gateway, auto &context, auto &stream_id, auto &shared) {
  return std::make_unique<Rest>(gateway, context, ++stream_id, shared);
}

template <typename R>
R create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared, auto &requests) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  if (!shared.settings.misc.number_of_order_entry_connections) {
    log::fatal("Unexpected: --number_of_order_entry_connections={}"sv, shared.settings.misc.number_of_order_entry_connections);
  }
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto &request = get_request(requests, account.name);
    auto obj = std::make_unique<OrderEntry>(gateway, context, ++stream_id, account, shared, request);
    result.try_emplace(account.name, std::move(obj));
  }
  return result;
}

template <typename R>
R create_drop_copy(auto &gateway, auto &context, auto &stream_id, auto &accounts, auto &shared, auto &requests) {
  R result;
  for (auto &[_, item] : accounts) {
    auto &account = *item;
    auto &request = get_request(requests, account.name);
    std::vector<std::unique_ptr<DropCopy>> drop_copy;
    for (size_t i = 0; i < shared.settings.misc.number_of_order_entry_connections; ++i) {
      auto master = i == 0;
      auto obj = std::make_unique<DropCopy>(gateway, context, ++stream_id, account, shared, request, master);
      drop_copy.emplace_back(std::move(obj));
    }
    result.try_emplace(account.name, std::move(drop_copy));
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

std::unique_ptr<server::Handler> Controller::create(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context) {
  return std::make_unique<Controller>(dispatcher, settings, config, context);
}

uint8_t Controller::parse_api(Settings const &settings) {
  auto api = API::parse_api(settings);
  switch (api) {
    using enum gateway::API::Key;
    case SPOT:
      return API_SPOT;
    case COIN_FUTURES:
      return API_COIN_FUTURES;
    case USDT_FUTURES:
      return API_USDT_FUTURES;
    case USDC_FUTURES:
      return API_USDC_FUTURES;
    case MARGIN:
      return API_MARGIN;
  }
  log::fatal(R"(Unexpected: api="{}")"sv, settings.app.api);
}

Controller::Controller(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_(dispatcher), master_account_(config.get_master_account()), context_(context), shared_(dispatcher, settings),
      accounts_(create_accounts<decltype(accounts_)>(settings, config)), requests_{create_requests<decltype(requests_)>(config, shared_)},
      rest_(create_rest(*this, context_, stream_id_, shared_)),
      order_entry_(create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, accounts_, shared_, requests_)),
      drop_copy_(create_drop_copy<decltype(drop_copy_)>(*this, context_, stream_id_, accounts_, shared_, requests_)) {
}

// server::Handler

void Controller::operator()(Trace<Start> const &event) {
  log::info("Starting..."sv);
  assert(std::empty(market_data_));
  dispatch(event);
}

void Controller::operator()(Trace<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Controller::operator()(Trace<Timer> const &event) {
  dispatch(event);
}

void Controller::operator()(Trace<Connected> const &, [[maybe_unused]] uint8_t user_id) {
}

void Controller::operator()(Trace<Disconnected> const &, [[maybe_unused]] uint8_t user_id) {
}

void Controller::operator()(Trace<Subscribe> const &event) {
  auto &[trace_info, subscribe] = event;
  std::vector<Symbol> symbols;
  for (auto &item : subscribe.symbols) {
    if (shared_.all_symbols.emplace(item).second) {
      symbols.emplace_back(item);
    } else {
      log::warn(R"(*** DUPLICATE SUBSCRIPTION *** (symbol="{}")"sv, item);
    }
  }
  auto symbols_update = SymbolsUpdate{
      .symbols = symbols,
  };
  create_trace_and_dispatch_2(trace_info, symbols_update);
}

void Controller::operator()(Trace<Control> const &event) {
  auto &[trace_info, control] = event;
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

void Controller::operator()(Trace<server::oms::OrderActionBatchBegin> const &event) {
  dispatch_order_action(event);
}

void Controller::operator()(Trace<server::oms::OrderActionBatchCommit> const &event) {
  dispatch_order_action(event);
}

void Controller::operator()(Trace<server::oms::OrderActionBatchRollback> const &event) {
  dispatch_order_action(event);
}

uint16_t Controller::operator()(
    Trace<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  return dispatch_order_action(event, order, ref_data, request_id);
}

uint16_t Controller::operator()(
    Trace<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  return dispatch_order_action(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(
    Trace<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  return dispatch_order_action(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Controller::operator()(Trace<CancelAllOrders> const &event, std::string_view const &request_id) {
  return dispatch_order_action(event, request_id);
}

uint16_t Controller::operator()(Trace<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Controller::operator()(Trace<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Controller::operator()(metrics::Writer &writer) const {
  dispatch_helper(*this, writer);
}

// Rest::Handler

void Controller::operator()(Trace<SymbolsUpdate> const &event) {
  auto &[trace_info, symbols_update] = event;
  auto [size, start_from] = shared_.symbols(symbols_update.symbols);
  ensure_symbol_slices(trace_info, size);
  for (auto &iter : market_data_) {
    dynamic_cast<server::MarketDataStream &>(*iter).subscribe(start_from);
  }
}

// utilities

void Controller::ensure_symbol_slices(TraceInfo const &trace_info, size_t size) {
  while (std::size(market_data_) < size) {
    log::debug("Create market-data (user-stream)"sv);
    auto obj = std::make_unique<MarketData>(*this, context_, ++stream_id_, shared_, std::size(market_data_));
    Start start;
    create_trace_and_dispatch(dynamic_cast<server::Stream &>(*obj), trace_info, start);
    market_data_.emplace_back(std::move(obj));
  }
}

template <typename... Args>
void Controller::dispatch(Args &&...args) {
  dispatch_helper(*this, std::forward<Args>(args)...);
}

template <typename... Args>
void Controller::dispatch_helper(auto &self, Args &&...args) {
  auto helper = [&](auto &target) {
    if constexpr (std::is_const_v<decltype(self)>) {
      dynamic_cast<server::Stream const &>(target)(std::forward<Args>(args)...);
    } else {
      dynamic_cast<server::Stream &>(target)(std::forward<Args>(args)...);
    }
  };
  helper(*self.rest_);
  for (auto &[_, order_entry] : self.order_entry_) {
    if (static_cast<bool>(order_entry)) {
      helper(*order_entry);
    }
  }
  for (auto &[_, drop_copy] : self.drop_copy_) {
    for (auto &item : drop_copy.drop_copy) {
      helper(*item);
    }
  }
  for (auto &iter : self.market_data_) {
    helper(*iter);
  }
}

template <typename T, typename... Args>
uint16_t Controller::dispatch_order_action(Trace<T> const &event, Args &&...args) {
  auto &[trace_info, value] = event;
  if (shared_.settings.misc.disable_ws_api) {
    auto iter = order_entry_.find(value.account);
    if (iter == std::end(order_entry_)) [[unlikely]] {
      throw RuntimeError(R"(Unknown account="{}")"sv, value.account);
    }
    auto &stream = dynamic_cast<server::OrderActionStream &>(*(*iter).second);
    stream(event, args...);
    return stream.stream_id();
  } else {
    auto iter = drop_copy_.find(value.account);
    if (iter == std::end(drop_copy_)) {
      throw RuntimeError(R"(Unknown account="{}")"sv, value.account);
    }
    auto &stream = (*iter).second.get_next();
    stream(event, args...);
    return stream.stream_id();
  }
}

// DropCopyRR

Controller::DropCopyRR::DropCopyRR(std::vector<std::unique_ptr<DropCopy>> &&drop_copy) : drop_copy{std::move(drop_copy)} {
}

server::OrderActionStream &Controller::DropCopyRR::get_next() {
  auto length = std::size(drop_copy);
  for (size_t offset = 0; offset < length; ++offset) {
    auto index = (index_ + offset) % length;
    auto &obj = dynamic_cast<server::OrderActionStream &>(*(drop_copy[index]));
    if (!obj.ready()) {
      continue;
    }
    index_ = (index + 1) % length;
    return obj;
  }
  throw server::oms::NotReady{"get_next"sv};
}

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
