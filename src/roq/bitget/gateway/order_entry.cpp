/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/gateway/order_entry.hpp"

#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/bitget/protocol/json/encoder.hpp"
#include "roq/bitget/protocol/json/map.hpp"
#include "roq/bitget/protocol/json/message.hpp"
#include "roq/bitget/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace gateway {

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

auto const SUPPORTS_WS_API = Mask{
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
          .account_settings = create_metrics(shared.settings, name_, "account_settings"sv),
          .account_settings_ack = create_metrics(shared.settings, name_, "account_settings_ack"sv),
          .account_assets = create_metrics(shared.settings, name_, "account_assets"sv),
          .account_assets_ack = create_metrics(shared.settings, name_, "account_assets_ack"sv),
          .current_positions = create_metrics(shared.settings, name_, "current_positions"sv),
          .current_positions_ack = create_metrics(shared.settings, name_, "current_positions_ack"sv),
          .unfilled_orders = create_metrics(shared.settings, name_, "unfilled_orders"sv),
          .unfilled_orders_ack = create_metrics(shared.settings, name_, "unfilled_orders_ack"sv),
          .trade_fills = create_metrics(shared.settings, name_, "trade_fills"sv),
          .trade_fills_ack = create_metrics(shared.settings, name_, "trade_fills_ack"sv),
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
      .write(profile_.account_settings, metrics::Type::PROFILE)
      .write(profile_.account_settings_ack, metrics::Type::PROFILE)
      .write(profile_.account_assets, metrics::Type::PROFILE)
      .write(profile_.account_assets_ack, metrics::Type::PROFILE)
      .write(profile_.current_positions, metrics::Type::PROFILE)
      .write(profile_.current_positions_ack, metrics::Type::PROFILE)
      .write(profile_.unfilled_orders, metrics::Type::PROFILE)
      .write(profile_.unfilled_orders_ack, metrics::Type::PROFILE)
      .write(profile_.trade_fills, metrics::Type::PROFILE)
      .write(profile_.trade_fills_ack, metrics::Type::PROFILE)
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

uint16_t OrderEntry::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  place_order(event, order, ref_data, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  modify_order(event, order, ref_data, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  cancel_order(event, order, ref_data, request_id, previous_request_id);
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
  create_trace_and_dispatch(shared_.dispatcher, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = account_.name,
      .supports = shared_.settings.ws_api ? SUPPORTS_WS_API : SUPPORTS,
      .transport = Transport::TCP,
      .protocol = Protocol::HTTP,
      .encoding = {Encoding::JSON},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
      .reason = reason,
      .interface = (*connection_).get_interface(),
      .authority = (*connection_).get_current_authority(),
      .path = (*connection_).get_current_path(),
      .proxy = (*connection_).get_proxy(),
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(shared_.dispatcher, trace_info, stream_status);
}

uint32_t OrderEntry::download(State state) {
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case ACCOUNT_SETTINGS:
      (*this)(ConnectionStatus::DOWNLOADING, "account-settings"sv);
      get_account_settings();
      return 1;
    case ACCOUNT_ASSETS:  // skip
      (*this)(ConnectionStatus::DOWNLOADING, "account-assets"sv);
      get_account_assets();
      return 1;
    case CURRENT_POSITIONS:  // skip
      (*this)(ConnectionStatus::DOWNLOADING, "current-positions"sv);
      get_current_positions();
      return 1;
    case UNFILLED_ORDERS:
      (*this)(ConnectionStatus::DOWNLOADING, "unfilled-orders"sv);
      get_unfilled_orders();
      return 1;
    case TRADE_FILLS:
      if (shared_.settings.rest.download_fills_begin.count()) {
        (*this)(ConnectionStatus::DOWNLOADING, "fills"sv);
        get_trade_fills();
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

// account-settings

void OrderEntry::get_account_settings() {
  profile_.account_settings([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.account_settings;
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
    (*connection_)("account_settings"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_account_settings_ack(event, sequence);
    });
  });
}

void OrderEntry::get_account_settings_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = State::ACCOUNT_SETTINGS;
  profile_.account_settings_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        protocol::json::AccountSettingsAck account_settings_ack{body, decode_buffer_};
        if (account_settings_ack.code == 0) {
          Trace event{trace_info, account_settings_ack};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(account_settings_ack.code), account_settings_ack.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::AccountSettingsAck> const &event) {
  auto &[trace_info, account_settings_ack] = event;
  log::info<4>("account_settings_ack={}"sv, account_settings_ack);
  log::warn(
      R"(DEBUG account="{}", account_mode={}, asset_mode={}, hold_mode={}, stp_mode={})"sv,
      account_.name,
      account_settings_ack.data.account_mode,
      account_settings_ack.data.asset_mode,
      account_settings_ack.data.hold_mode,
      account_settings_ack.data.stp_mode);
  for (auto &item : account_settings_ack.data.symbol_config_list) {
    log::warn(R"(DEBUG account="{}", symbol="{}", leverage={}, margin_mode={})"sv, account_.name, item.symbol, item.leverage, item.margin_mode);
  }
  for (auto &item : account_settings_ack.data.coin_config_list) {
    log::warn(R"(DEBUG account="{}", coin="{}", leverage={})"sv, account_.name, item.coin, item.leverage);
  }
}

// account-assets

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
  auto const state = State::ACCOUNT_ASSETS;
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
        protocol::json::AccountAssetsAck account_assets_ack{body, decode_buffer_};
        if (account_assets_ack.code == 0) {
          Trace event{trace_assets, account_assets_ack};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(account_assets_ack.code), account_assets_ack.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

// note! download because it seems we don't always get snapshot from drop-copy
void OrderEntry::operator()(Trace<protocol::json::AccountAssetsAck> const &event) {
  auto &[trace_assets, account_assets_ack] = event;
  log::info<4>("account_assets_ack={}"sv, account_assets_ack);
  for (auto &item : account_assets_ack.data.assets) {
    log::info<2>("item={}"sv, item);
    auto funds_update = FundsUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .currency = item.coin,
        .margin_mode = {},
        .balance = item.balance,  // ???
        .hold = item.locked,
        .borrowed = item.debt,
        .unrealized_pnl = NaN,
        .external_account = {},
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = account_assets_ack.request_time,
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_assets, funds_update, true);
  }
}

// current-positions

void OrderEntry::get_current_positions() {
  profile_.current_positions([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.current_positions;
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
    (*connection_)("current_positions"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_current_positions_ack(event, sequence);
    });
  });
}

void OrderEntry::get_current_positions_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = State::CURRENT_POSITIONS;
  profile_.current_positions_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        protocol::json::CurrentPositionsAck current_positions_ack{body, decode_buffer_};
        if (current_positions_ack.code == 0) {
          Trace event{trace_info, current_positions_ack};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(current_positions_ack.code), current_positions_ack.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

// note! download because it seems we don't always get snapshot from drop-copy
void OrderEntry::operator()(Trace<protocol::json::CurrentPositionsAck> const &event) {
  auto &[trace_info, current_positions_ack] = event;
  log::info<4>("current_positions_ack={}"sv, current_positions_ack);
  for (auto &item : current_positions_ack.data.list) {
    log::info<2>("item={}"sv, item);
    auto long_quantity = [&]() -> double {
      if (item.pos_side == protocol::json::PosSide::LONG) {
        return item.total;
      }
      return 0.0;
    }();
    auto short_quantity = [&]() -> double {
      if (item.pos_side == protocol::json::PosSide::SHORT) {
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
        .sending_time_utc = current_positions_ack.request_time,
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_info, position_update, true);
  }
}

// unfilled-orders

void OrderEntry::get_unfilled_orders() {
  profile_.unfilled_orders([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.unfilled_orders;
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
    (*connection_)("unfilled_orders"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_unfilled_orders_ack(event, sequence);
    });
  });
}

void OrderEntry::get_unfilled_orders_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = State::UNFILLED_ORDERS;
  profile_.unfilled_orders_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        protocol::json::UnfilledOrdersAck unfilled_orders_ack{body, decode_buffer_};
        if (unfilled_orders_ack.code == 0) {
          Trace event{trace_info, unfilled_orders_ack};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(unfilled_orders_ack.code), unfilled_orders_ack.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::UnfilledOrdersAck> const &event) {
  auto &[trace_info, unfilled_orders_ack] = event;
  log::info<4>("unfilled_orders_ack={}"sv, unfilled_orders_ack);
  for (auto &item : unfilled_orders_ack.data.list) {
    log::info<2>("item={}"sv, item);
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
        .error = {},
        .text = {},
        .quantity = item.qty,
        .price = item.price,
        .stop_price = NaN,
        .leverage = NaN,
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
        .sending_time_utc = unfilled_orders_ack.request_time,
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_info, order_update, stream_id_);
  }
}

// trade-fills

void OrderEntry::get_trade_fills() {
  assert(shared_.settings.rest.download_fills_begin.count() > 0);
  profile_.trade_fills([&]() {
    auto now = clock::get_realtime();
    auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - shared_.settings.rest.download_fills_begin);
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.trade_fills;
    auto query = fmt::format("?category={}&startTime={}"sv, shared_.api.category, start_time.count());
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
    (*connection_)("trade_fills"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_trade_fills_ack(event, sequence);
    });
  });
}

void OrderEntry::get_trade_fills_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = State::TRADE_FILLS;
  profile_.trade_fills_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        protocol::json::TradeFillsAck trade_fills_ack{body, decode_buffer_};
        if (trade_fills_ack.code == 0) {
          Trace event{trace_info, trade_fills_ack};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(trade_fills_ack.code), trade_fills_ack.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::TradeFillsAck> const &event) {
  auto &[trace_info, trade_fills_ack] = event;
  log::info<4>("trade_fills_ack={}"sv, trade_fills_ack);
  std::string_view symbol, order_id, client_oid;
  protocol::json::Side side = {};
  protocol::json::TradeSide trade_side = {};
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
          .sending_time_utc = trade_fills_ack.request_time,
          .user = {},
          .strategy_id = {},
      };
      create_trace_and_dispatch(shared_.dispatcher, trace_info, trade_update, true, SOURCE_NONE);
      shared_.fills.clear();
    }
  };
  shared_.fills.clear();
  for (auto &item : trade_fills_ack.data.list) {
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

// place-order

void OrderEntry::place_order(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  profile_.place_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, create_order] = event;
    auto method = web::http::Method::POST;
    auto path = shared_.api.order_management.place_order;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto body = protocol::json::Encoder::place_order(encode_buffer_, create_order, order, ref_data, request_id, shared_.api.category);
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
    auto &[trace_info, response] = event;
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
          .external_order_id = {},
          .client_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      create_trace_and_dispatch(shared_.dispatcher, trace_info, response, stream_id_, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::PlaceOrderAck place_order_ack{body, decode_buffer_};
      if (place_order_ack.code == 0) {
        Trace event_2{event, place_order_ack};
        (*this)(event_2, user_id, order_id, version);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(place_order_ack.code), place_order_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(
    Trace<protocol::json::PlaceOrderAck> const &event,
    [[maybe_unused]] uint8_t user_id,
    [[maybe_unused]] uint64_t order_id,
    [[maybe_unused]] uint32_t version) {
  auto &[trace_info, place_order_ack] = event;
  log::info<2>("place_order_ack={}"sv, place_order_ack);
}

// modify-order

void OrderEntry::modify_order(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
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
    auto body = protocol::json::Encoder::modify_order(encode_buffer_, modify_order, order, ref_data, request_id);
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
    auto &[trace_info, response] = event;
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
          .external_order_id = {},
          .client_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      create_trace_and_dispatch(shared_.dispatcher, trace_info, response, stream_id_, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::ModifyOrderAck modify_order_ack{body, decode_buffer_};
      if (modify_order_ack.code == 0) {
        Trace event_2{event, modify_order_ack};
        (*this)(event_2, user_id, order_id, version);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(modify_order_ack.code), modify_order_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(
    Trace<protocol::json::ModifyOrderAck> const &event,
    [[maybe_unused]] uint8_t user_id,
    [[maybe_unused]] uint64_t order_id,
    [[maybe_unused]] uint32_t version) {
  auto &[trace_info, modify_order_ack] = event;
  log::info<2>("modify_order_ack={}"sv, modify_order_ack);
}

// cancel-order

void OrderEntry::cancel_order(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
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
    auto body = protocol::json::Encoder::cancel_order(encode_buffer_, cancel_order, order, ref_data, request_id);
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
    auto &[trace_info, response] = event;
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
          .external_order_id = {},
          .client_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      create_trace_and_dispatch(shared_.dispatcher, trace_info, response, stream_id_, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::CancelOrderAck cancel_order_ack{body, decode_buffer_};
      if (cancel_order_ack.code == 0) {
        Trace event_2{event, cancel_order_ack};
        (*this)(event_2, user_id, order_id, version);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(cancel_order_ack.code), cancel_order_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(
    Trace<protocol::json::CancelOrderAck> const &event,
    [[maybe_unused]] uint8_t user_id,
    [[maybe_unused]] uint64_t order_id,
    [[maybe_unused]] uint32_t version) {
  auto &[trace_info, cancel_order_ack] = event;
  log::info<2>("cancel_order_ack={}"sv, cancel_order_ack);
}

// cancel-all-orders

void OrderEntry::cancel_all_orders(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  profile_.cancel_all_orders([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_all_orders] = event;
    auto method = web::http::Method::POST;
    auto path = shared_.api.order_management.cancel_all_orders;
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto body = protocol::json::Encoder::cancel_all_orders(encode_buffer_, cancel_all_orders, request_id, shared_.api.category);
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
      create_trace_and_dispatch(shared_.dispatcher, trace_info, cancel_all_orders_ack);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::CancelAllOrdersAck cancel_all_orders_ack{body, decode_buffer_};
      if (cancel_all_orders_ack.code == 0) {
        Trace event_2{event, cancel_all_orders_ack};
        (*this)(event_2, user_id);
      } else {
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(cancel_all_orders_ack.code), cancel_all_orders_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<protocol::json::CancelAllOrdersAck> const &event, [[maybe_unused]] uint8_t user_id) {
  auto &[trace_info, cancel_all_orders_ack] = event;
  log::info<2>("cancel_all_orders_ack={}"sv, cancel_all_orders_ack);
}

// countdown-cancel-all

void OrderEntry::countdown_cancel_all() {
  profile_.countdown_cancel_all([&]() {
    auto method = web::http::Method::POST;
    auto path = shared_.api.order_management.countdown_cancel_all;
    auto body =
        protocol::json::Encoder::countdown_cancel_all(encode_buffer_, std::chrono::duration_cast<std::chrono::seconds>(shared_.settings.rest.ping_freq));
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
            protocol::json::Message error{body};
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(error.code), error.msg);
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

}  // namespace gateway
}  // namespace bitget
}  // namespace roq
