/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/order_entry.hpp"

#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/bitget/json/encoder.hpp"
#include "roq/bitget/json/map.hpp"
#include "roq/bitget/json/message.hpp"
#include "roq/bitget/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {

// === CONSTANTS ===

namespace {
auto const NAME = "om"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::FUNDS,
    SupportType::POSITION,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 2;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account.name);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.rest.uri;
  auto config = web::rest::Client::Config{
      // connection
      .interface = {},
      .proxy = settings.rest.proxy,
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = {},
      .disconnect_on_idle_timeout = {},
      .connection = web::http::Connection::KEEP_ALIVE,
      // request
      .allow_pipelining = true,
      .request_timeout = settings.rest.request_timeout,
      // response
      .suspend_on_retry_after = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .ping_frequency = settings.rest.ping_freq,
      .ping_path = settings.rest.ping_path,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::rest::Client::create(handler, context, config);
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

OrderEntry::OrderEntry(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .account_info = create_metrics(shared.settings, name_, "account_info"sv),
          .account_info_ack = create_metrics(shared.settings, name_, "account_info_ack"sv),
          .account_assets = create_metrics(shared.settings, name_, "account_assets"sv),
          .account_assets_ack = create_metrics(shared.settings, name_, "account_assets_ack"sv),
          .position_info = create_metrics(shared.settings, name_, "position_info"sv),
          .position_info_ack = create_metrics(shared.settings, name_, "position_info_ack"sv),
          .open_orders = create_metrics(shared.settings, name_, "open_orders"sv),
          .open_orders_ack = create_metrics(shared.settings, name_, "open_orders_ack"sv),
          .fill_history = create_metrics(shared.settings, name_, "fill_history"sv),
          .fill_history_ack = create_metrics(shared.settings, name_, "fill_history_ack"sv),
          .place_order = create_metrics(shared.settings, name_, "place_order"sv),
          .place_order_ack = create_metrics(shared.settings, name_, "place_order_ack"sv),
          .modify_order = create_metrics(shared.settings, name_, "modify_order"sv),
          .modify_order_ack = create_metrics(shared.settings, name_, "modify_order_ack"sv),
          .cancel_order = create_metrics(shared.settings, name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(shared.settings, name_, "cancel_order_ack"sv),
          .cancel_all_orders = create_metrics(shared.settings, name_, "cancel_all_orders"sv),
          .cancel_all_orders_ack = create_metrics(shared.settings, name_, "cancel_all_orders_ack"sv),
          .countdown_cancel_all = create_metrics(shared.settings, name_, "countdown_cancel_all"sv),
          .countdown_cancel_all_ack = create_metrics(shared.settings, name_, "countdown_cancel_all_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      account_{account}, shared_{shared}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
}

void OrderEntry::operator()(Event<Start> const &) {
  (*connection_).start();
}

void OrderEntry::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void OrderEntry::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (!ready()) {
    return;
  }
  if (shared_.settings.rest.cancel_on_disconnect && next_heartbeat_ < now) {
    next_heartbeat_ = now + (shared_.settings.rest.ping_freq / 3);
    countdown_cancel_all();
  }
}

void OrderEntry::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.account_info, metrics::Type::PROFILE)
      .write(profile_.account_info_ack, metrics::Type::PROFILE)
      .write(profile_.account_assets, metrics::Type::PROFILE)
      .write(profile_.account_assets_ack, metrics::Type::PROFILE)
      .write(profile_.position_info, metrics::Type::PROFILE)
      .write(profile_.position_info_ack, metrics::Type::PROFILE)
      .write(profile_.open_orders, metrics::Type::PROFILE)
      .write(profile_.open_orders_ack, metrics::Type::PROFILE)
      .write(profile_.fill_history, metrics::Type::PROFILE)
      .write(profile_.fill_history_ack, metrics::Type::PROFILE)
      .write(profile_.place_order, metrics::Type::PROFILE)
      .write(profile_.place_order_ack, metrics::Type::PROFILE)
      .write(profile_.modify_order, metrics::Type::PROFILE)
      .write(profile_.modify_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_order, metrics::Type::PROFILE)
      .write(profile_.cancel_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders_ack, metrics::Type::PROFILE)
      .write(profile_.countdown_cancel_all, metrics::Type::PROFILE)
      .write(profile_.countdown_cancel_all_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

uint16_t OrderEntry::operator()(Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  place_order(event, order, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  modify_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, std::string_view const &previous_request_id) {
  cancel_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  cancel_all_orders(event, request_id);
  return stream_id_;
}

void OrderEntry::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
  next_heartbeat_ = {};
}

void OrderEntry::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.name,
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    using enum OrderEntryState;
    case UNDEFINED:
      assert(false);
      break;
    case ACCOUNT_INFO:
      get_account_info();
      return 1;
    case ACCOUNT_ASSETS:  // skip
      get_account_assets();
      return 1;
    case POSITION_INFO:  // skip
      get_position_info();
      return 1;
    case OPEN_ORDERS:
      get_open_orders();
      return 1;
    case FILL_HISTORY:
      if (shared_.settings.rest.download_fills_begin.count()) {
        get_fill_history();
        return 1;
      } else {
        return 0;
      }
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

// account_info

void OrderEntry::get_account_info() {
  profile_.account_info([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.account_info;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto headers = account_.create_headers(method, path, query, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("account_info"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_account_info_ack(event, sequence);
    });
  });
}

void OrderEntry::get_account_info_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = OrderEntryState::ACCOUNT_INFO;
  profile_.account_info_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::AccountInfo account_info{body, decode_buffer_};
        if (account_info.code == 0) {
          Trace event{trace_info, account_info};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(account_info.code), account_info.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::AccountInfo> const &event) {
  auto &[trace_info, account_info] = event;
  log::info<4>("account_info={}"sv, account_info);
  log::warn(
      R"(DEBUG account="{}", account_mode={}, asset_mode={}, hold_mode={}, stp_mode={})"sv,
      account_.name,
      account_info.data.account_mode,
      account_info.data.asset_mode,
      account_info.data.hold_mode,
      account_info.data.stp_mode);
  for (auto &item : account_info.data.symbol_config_list) {
    log::warn(R"(DEBUG account="{}", symbol="{}", leverage={}, margin_mode={})"sv, account_.name, item.symbol, item.leverage, item.margin_mode);
  }
  for (auto &item : account_info.data.coin_config_list) {
    log::warn(R"(DEBUG account="{}", coin="{}", leverage={})"sv, account_.name, item.coin, item.leverage);
  }
}

// account_assets

void OrderEntry::get_account_assets() {
  profile_.account_assets([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.account_assets;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto headers = account_.create_headers(method, path, query, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("account_assets"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_assets;
      Trace event{trace_assets, response};
      get_account_assets_ack(event, sequence);
    });
  });
}

void OrderEntry::get_account_assets_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = OrderEntryState::ACCOUNT_ASSETS;
  profile_.account_assets_ack([&]() {
    auto &[trace_assets, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::AccountAssets account_assets{body, decode_buffer_};
        if (account_assets.code == 0) {
          Trace event{trace_assets, account_assets};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(account_assets.code), account_assets.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

// note! download because it seems we don't always get snapshot from drop-copy
void OrderEntry::operator()(Trace<json::AccountAssets> const &event) {
  auto &[trace_assets, account_assets] = event;
  log::info<4>("account_assets={}"sv, account_assets);
  for (auto &item : account_assets.data.assets) {
    log::warn("DEBUG item={}"sv, item);
    auto funds_update = FundsUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .currency = item.coin,
        .margin_mode = {},
        .balance = item.balance,  // ???
        .hold = item.locked,
        .borrowed = item.debt,
        .external_account = {},
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = account_assets.request_time,
    };
    log::warn("DEBUG funds_update={}"sv, funds_update);
    create_trace_and_dispatch(handler_, trace_assets, funds_update, true);
  }
}

// position_info

void OrderEntry::get_position_info() {
  profile_.position_info([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.position_info;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto headers = account_.create_headers(method, path, query, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("position_info"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_position_info_ack(event, sequence);
    });
  });
}

void OrderEntry::get_position_info_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = OrderEntryState::POSITION_INFO;
  profile_.position_info_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::PositionInfo position_info{body, decode_buffer_};
        if (position_info.code == 0) {
          Trace event{trace_info, position_info};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(position_info.code), position_info.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

// note! download because it seems we don't always get snapshot from drop-copy
void OrderEntry::operator()(Trace<json::PositionInfo> const &event) {
  auto &[trace_info, position_info] = event;
  log::info<4>("position_info={}"sv, position_info);
  for (auto &item : position_info.data.list) {
    log::warn("DEBUG item={}"sv, item);
    auto long_quantity = [&]() -> double {
      if (item.pos_side == json::PosSide::LONG) {
        return item.total;
      }
      return 0.0;
    }();
    auto short_quantity = [&]() -> double {
      if (item.pos_side == json::PosSide::SHORT) {
        return item.total;
      }
      return 0.0;
    }();
    auto position_update = PositionUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .margin_mode = map(item.margin_mode),
        .external_account = {},
        .long_quantity = long_quantity,
        .short_quantity = short_quantity,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = item.created_time,  // ???
        .exchange_sequence = {},
        .sending_time_utc = position_info.request_time,
    };
    log::warn("DEBUG position_update={}"sv, position_update);
    create_trace_and_dispatch(handler_, trace_info, position_update, true);
  }
}

// open_orders

void OrderEntry::get_open_orders() {
  profile_.open_orders([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.open_orders;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto headers = account_.create_headers(method, path, query, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("open_orders"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_open_orders_ack(event, sequence);
    });
  });
}

void OrderEntry::get_open_orders_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = OrderEntryState::OPEN_ORDERS;
  profile_.open_orders_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::OpenOrders open_orders{body, decode_buffer_};
        if (open_orders.code == 0) {
          Trace event{trace_info, open_orders};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(open_orders.code), open_orders.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::OpenOrders> const &event) {
  auto &[trace_info, open_orders] = event;
  log::info<4>("open_orders={}"sv, open_orders);
  for (auto &item : open_orders.data.list) {
    log::warn("DEBUG item={}"sv, item);
    auto remaining_quantity = item.qty - item.cum_exec_qty;
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .side = map(item.side),
        .position_effect = map(item.pos_side, item.side),
        .margin_mode = {},  // XXX FIXME TODO from asset_info[symbol]
        .max_show_quantity = NaN,
        .order_type = map(item.order_type),
        .time_in_force = map(item.time_in_force),
        .execution_instructions = {},  // XXX map from time_in_force + reduce_only
        .create_time_utc = item.created_time,
        .update_time_utc = item.updated_time,
        .external_account = {},
        .external_order_id = item.order_id,
        .client_order_id = item.client_oid,
        .order_status = map(item.order_status),
        .quantity = item.qty,
        .price = item.price,
        .stop_price = NaN,
        .remaining_quantity = remaining_quantity,
        .traded_quantity = item.cum_exec_qty,
        .average_traded_price = item.avg_price,
        .last_traded_quantity = NaN,
        .last_traded_price = NaN,
        .last_liquidity = {},
        .routing_id = {},
        .max_request_version = {},
        .max_response_version = {},
        .max_accepted_version = {},
        .update_type = UpdateType::SNAPSHOT,
        .sending_time_utc = open_orders.request_time,
    };
    log::warn("DEBUG order_update={}"sv, order_update);
    Trace event_2{trace_info, order_update};
    (*this)(event_2, item.client_oid);
  }
}

// fill_history
void OrderEntry::get_fill_history() {
  assert(shared_.settings.rest.download_fills_begin.count() > 0);
  profile_.fill_history([&]() {
    auto now = clock::get_realtime();
    auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - shared_.settings.rest.download_fills_begin);
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.fill_history;
    auto query = fmt::format("?category={}&startTime={}"sv, shared_.api.category, start_time.count());
    log::warn("DEBUG query={}"sv, query);
    auto headers = account_.create_headers(method, path, query, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("fill_history"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_fill_history_ack(event, sequence);
    });
  });
}

void OrderEntry::get_fill_history_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = OrderEntryState::FILL_HISTORY;
  profile_.fill_history_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::FillHistory fill_history{body, decode_buffer_};
        if (fill_history.code == 0) {
          Trace event{trace_info, fill_history};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(fill_history.code), fill_history.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::FillHistory> const &event) {
  auto &[trace_info, fill_history] = event;
  log::info<4>("fill_history={}"sv, fill_history);
  std::string_view symbol, order_id, client_oid;
  json::Side side = {};
  json::TradeSide trade_side = {};
  std::chrono::nanoseconds created_time = {};
  std::chrono::nanoseconds updated_time = {};
  auto dispatch = [&]() {
    if (!std::empty(shared_.fills)) {
      auto trade_update = TradeUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = shared_.settings.exchange,
          .symbol = symbol,
          .side = map(side),
          .position_effect = map(trade_side),
          .margin_mode = {},  // XXX FIXME TODO from asset_info[symbol]
          .quantity_type = {},
          .create_time_utc = created_time,
          .update_time_utc = updated_time,
          .external_account = {},
          .external_order_id = order_id,
          .client_order_id = client_oid,
          .fills = shared_.fills,
          .routing_id = {},
          .update_type = UpdateType::SNAPSHOT,
          .sending_time_utc = fill_history.request_time,
          .user = {},
          .strategy_id = {},
      };
      create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE, client_oid);
      log::warn("DEBUG trade_update={}"sv, trade_update);
      shared_.fills.clear();
    }
  };
  shared_.fills.clear();
  for (auto &item : fill_history.data.list) {
    log::warn("DEBUG item={}"sv, item);
    if (item.symbol != symbol || item.order_id != order_id || item.client_oid != client_oid || item.side != side || item.trade_side != trade_side) {
      dispatch();
      symbol = item.symbol;
      order_id = item.order_id;
      client_oid = item.client_oid;
      side = item.side;
      trade_side = item.trade_side, created_time = {};
      updated_time = {};
    }
    std::string_view fee_coin;
    double fee = 0.0;
    bool please_report = false;
    for (auto &item_2 : item.fee_detail) {
      if (!std::isnan(item_2.fee)) {
        fee += item_2.fee;
      }
      if (!std::empty(item_2.fee_coin)) {
        if (std::empty(fee_coin)) {
          fee_coin = item_2.fee_coin;
        } else if (item_2.fee_coin != fee_coin) {
          log::warn(R"(fee_coin="{}"!="{}")"sv, item_2.fee_coin, fee_coin);
        }
      }
    }
    if (please_report) {
      log::warn("*** PLEASE REPORT *** fill={}"sv, item);
    }
    auto fill = Fill{
        .exchange_time_utc = item.created_time,
        .external_trade_id = item.exec_id,
        .quantity = item.exec_qty,
        .price = item.exec_price,
        .liquidity = map(item.trade_scope),
        .commission_amount = fee,
        .commission_currency = fee_coin,
        .base_amount = NaN,   // XXX FIXME TODO
        .quote_amount = NaN,  // XXX FIXME TODO
        .profit_loss_amount = NaN,
    };
    shared_.fills.emplace_back(std::move(fill));
    if (created_time < item.created_time) {
      created_time = item.created_time;
    }
    if (updated_time < item.updated_time) {
      updated_time = item.updated_time;
    }
  }
  dispatch();
}

// place_order

void OrderEntry::place_order(Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  profile_.place_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, create_order] = event;
    auto method = web::http::Method::POST;
    auto path = shared_.api.order_management.place_order;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto body = json::Encoder::place_order(encode_buffer_, create_order, order, request_id, shared_.api.category);
    log::warn(R"(DEBUG body="{}")"sv, body);
    auto headers = account_.create_headers(method, path, query, body);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = {},
    };
    auto callback = [this, user_id = message_info.source, order_id = create_order.order_id]([[maybe_unused]] auto &request_id, auto &response) {
      uint32_t version = 1;
      TraceInfo trace_info;
      Trace event{trace_info, response};
      place_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::place_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.place_order_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      json::PlaceOrderAck place_order_ack{body, decode_buffer_};
      if (place_order_ack.code == 0) {
        Trace event_2{event, place_order_ack};
        (*this)(event_2, user_id, order_id, version);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(place_order_ack.code), place_order_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::PlaceOrderAck> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, place_order_ack] = event;
  log::info<2>("place_order_ack={}"sv, place_order_ack);
  log::warn("DEBUG place_order_ack={}"sv, place_order_ack);
}

// modify_order

void OrderEntry::modify_order(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, modify_order] = event;
    auto method = web::http::Method::POST;
    auto path = shared_.api.order_management.modify_order;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto body = json::Encoder::modify_order(encode_buffer_, modify_order, order, request_id);
    log::warn(R"(DEBUG body="{}")"sv, body);
    auto headers = account_.create_headers(method, path, query, body);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = {},
    };
    auto callback = [this, user_id = message_info.source, order_id = modify_order.order_id, version = modify_order.version](
                        [[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      modify_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::modify_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.modify_order_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::MODIFY_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      json::ModifyOrderAck modify_order_ack{body, decode_buffer_};
      if (modify_order_ack.code == 0) {
        Trace event_2{event, modify_order_ack};
        (*this)(event_2, user_id, order_id, version);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(modify_order_ack.code), modify_order_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::ModifyOrderAck> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, modify_order_ack] = event;
  log::info<2>("modify_order_ack={}"sv, modify_order_ack);
  log::warn("DEBUG modify_order_ack={}"sv, modify_order_ack);
}

// cancel_order

void OrderEntry::cancel_order(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_order] = event;
    auto method = web::http::Method::POST;
    auto path = shared_.api.order_management.cancel_order;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto body = json::Encoder::cancel_order(encode_buffer_, cancel_order, order, request_id);
    log::warn(R"(DEBUG body="{}")"sv, body);
    auto headers = account_.create_headers(method, path, query, body);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = {},
    };
    auto callback = [this, user_id = message_info.source, order_id = cancel_order.order_id, version = cancel_order.version](
                        [[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      cancel_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::cancel_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.cancel_order_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CANCEL_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      json::CancelOrderAck cancel_order_ack{body, decode_buffer_};
      if (cancel_order_ack.code == 0) {
        Trace event_2{event, cancel_order_ack};
        (*this)(event_2, user_id, order_id, version);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(cancel_order_ack.code), cancel_order_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::CancelOrderAck> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  auto &[trace_info, cancel_order_ack] = event;
  log::info<2>("cancel_order_ack={}"sv, cancel_order_ack);
  log::warn("DEBUG cancel_order_ack={}"sv, cancel_order_ack);
}

// cancel_all_orders

void OrderEntry::cancel_all_orders(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  profile_.cancel_all_orders([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_all_orders] = event;
    auto method = web::http::Method::POST;
    auto path = shared_.api.order_management.cancel_all_orders;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto body = json::Encoder::cancel_all_orders(encode_buffer_, cancel_all_orders, request_id, shared_.api.category);
    auto headers = account_.create_headers(method, path, query, body);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = {},
    };
    auto callback = [this, user_id = message_info.source]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      cancel_all_orders_ack(event, user_id);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::cancel_all_orders_ack(Trace<web::rest::Response> const &event, [[maybe_unused]] uint8_t user_id) {
  profile_.cancel_all_orders_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto cancel_all_orders_ack = CancelAllOrdersAck{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = shared_.settings.exchange,
          .symbol = {},
          .side = {},
          .origin = Origin::EXCHANGE,
          .request_status = RequestStatus::REJECTED,
          .error = error,
          .text = text,
          .request_id = {},  // ???
          .external_account = {},
          .number_of_affected_orders = {},
          .round_trip_latency = {},
          .user = {},
          .strategy_id = {},
      };
      TraceInfo trace_info;
      Trace event_2{trace_info, cancel_all_orders_ack};
      shared_(event_2);
    };
    auto handle_success = [&](auto &body) {
      json::CancelAllOrdersAck cancel_all_orders_ack{body, decode_buffer_};
      if (cancel_all_orders_ack.code == 0) {
        Trace event_2{event, cancel_all_orders_ack};
        (*this)(event_2, user_id);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(cancel_all_orders_ack.code), cancel_all_orders_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::CancelAllOrdersAck> const &event, uint8_t user_id) {
  auto &[trace_info, cancel_all_orders_ack] = event;
  log::info<2>("cancel_all_orders_ack={}"sv, cancel_all_orders_ack);
  log::warn("DEBUG cancel_all_orders_ack={}"sv, cancel_all_orders_ack);
}

// countdown_cancel_all

void OrderEntry::countdown_cancel_all() {
  profile_.countdown_cancel_all([&]() {
    auto method = web::http::Method::POST;
    auto path = shared_.api.order_management.countdown_cancel_all;
    auto body = json::Encoder::countdown_cancel_all(encode_buffer_, std::chrono::duration_cast<std::chrono::seconds>(shared_.settings.rest.ping_freq));
    auto headers = account_.create_headers(method, path, {}, body);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = {},
    };
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      countdown_cancel_all_ack(event);
    };
    (*connection_)("countdown_cancel_all"sv, request, callback);
  });
}

void OrderEntry::countdown_cancel_all_ack(Trace<web::rest::Response> const &event) {
  profile_.countdown_cancel_all_ack([&]() {
    auto &[trace_info, response] = event;
    auto [status, category, body] = response.result();
    if (status != web::http::Status::OK) {
      log::warn(R"(DEBUG status={}, category={}, body="{}")"sv, status, category, body);
    }
  });
}

// helpers

void OrderEntry::process_response(web::rest::Response const &response, auto error_handler, auto success_handler) {
  try {
    auto [status, category, body] = response.result();
    switch (category) {
      using enum web::http::Category;
      case UNKNOWN:
      case INFORMATIONAL_RESPONSE:
        response.expect(web::http::Status::OK);  // throws
        break;
      case SUCCESS:
        success_handler(body);
        break;
      case REDIRECTION:
        log::fatal("Unexpected: URL is being redirected"sv);
      case CLIENT_ERROR:
        switch (status) {
          using enum web::http::Status;
          case FORBIDDEN:            // 403
          case I_AM_A_TEAPOT:        // 418
          case TOO_MANY_REQUESTS: {  // 429
            auto message = fmt::format("{}"sv, status);
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::REQUEST_RATE_LIMIT_REACHED, message);
            break;
          }
          case CONFLICT:  // 409
            assert(false);
            [[fallthrough]];
          default: {
            json::Message error{body};
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(error.code), error.msg);
          }
        }
        break;
      case SERVER_ERROR: {
        auto message = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
        break;
      }
    }
  } catch (server::oms::Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(e.origin, e.status, e.error, e.what());
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}

template <typename... Args>
void OrderEntry::operator()(Trace<server::oms::Response> const &event, uint8_t user_id, uint64_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(user_id, order_id, stream_id_, trace_info, response, std::forward<Args>(args)..., []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

void OrderEntry::operator()(Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

}  // namespace bitget
}  // namespace roq
