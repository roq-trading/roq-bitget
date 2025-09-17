/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget_futures/order_entry.hpp"

#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/bitget_futures/json/encoder.hpp"
#include "roq/bitget_futures/json/map.hpp"
#include "roq/bitget_futures/json/message.hpp"
#include "roq/bitget_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget_futures {

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
          .all_accounts = create_metrics(shared.settings, name_, "all_accounts"sv),
          .all_accounts_ack = create_metrics(shared.settings, name_, "all_accounts_ack"sv),
          .all_positions = create_metrics(shared.settings, name_, "all_positions"sv),
          .all_positions_ack = create_metrics(shared.settings, name_, "all_positions_ack"sv),
          .orders_history = create_metrics(shared.settings, name_, "orders_history"sv),
          .orders_history_ack = create_metrics(shared.settings, name_, "orders_history_ack"sv),
          .order_fill_details = create_metrics(shared.settings, name_, "order_fill_details"sv),
          .order_fill_details_ack = create_metrics(shared.settings, name_, "order_fill_details_ack"sv),
          .place_order = create_metrics(shared.settings, name_, "place_order"sv),
          .place_order_ack = create_metrics(shared.settings, name_, "place_order_ack"sv),
          .modify_order = create_metrics(shared.settings, name_, "modify_order"sv),
          .modify_order_ack = create_metrics(shared.settings, name_, "modify_order_ack"sv),
          .cancel_order = create_metrics(shared.settings, name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(shared.settings, name_, "cancel_order_ack"sv),
          .cancel_all_orders = create_metrics(shared.settings, name_, "cancel_all_orders"sv),
          .cancel_all_orders_ack = create_metrics(shared.settings, name_, "cancel_all_orders_ack"sv),
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
}

void OrderEntry::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.all_accounts, metrics::Type::PROFILE)
      .write(profile_.all_accounts_ack, metrics::Type::PROFILE)
      .write(profile_.all_positions, metrics::Type::PROFILE)
      .write(profile_.all_positions_ack, metrics::Type::PROFILE)
      .write(profile_.orders_history, metrics::Type::PROFILE)
      .write(profile_.orders_history_ack, metrics::Type::PROFILE)
      .write(profile_.order_fill_details, metrics::Type::PROFILE)
      .write(profile_.order_fill_details_ack, metrics::Type::PROFILE)
      .write(profile_.place_order, metrics::Type::PROFILE)
      .write(profile_.place_order_ack, metrics::Type::PROFILE)
      .write(profile_.modify_order, metrics::Type::PROFILE)
      .write(profile_.modify_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_order, metrics::Type::PROFILE)
      .write(profile_.cancel_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders_ack, metrics::Type::PROFILE)
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
  margin_coins_.clear();
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
    case ACCOUNTS:
      get_all_accounts();
      return 1;
    case POSITIONS:
      get_all_positions();
      return 1;
    case ORDERS:
      get_orders_history();
      return 1;
    case FILLS:
      get_order_fill_details();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

// all_accounts

void OrderEntry::get_all_accounts() {
  profile_.all_accounts([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.all_accounts;
    auto headers = account_.create_headers(method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("all_accounts"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_all_accounts_ack(event, sequence);
    });
  });
}

void OrderEntry::get_all_accounts_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = OrderEntryState::ACCOUNTS;
  profile_.all_accounts_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::AllAccounts all_accounts{body, decode_buffer_};
        if (all_accounts.code == 0) {
          Trace event{trace_info, all_accounts};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(all_accounts.code), all_accounts.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::AllAccounts> const &event) {
  auto &[trace_info, all_accounts] = event;
  log::info<4>("all_accounts={}"sv, all_accounts);
  for (auto &item : all_accounts.data) {
    log::warn("DEBUG item={}"sv, item);
    auto funds_update = FundsUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .currency = item.margin_coin,
        .margin_mode = {},
        .balance = item.available,
        .hold = item.locked,
        .borrowed = NaN,
        .external_account = {},
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    //
    auto margin_mode = map(item.asset_mode).template get<MarginMode>();
    log::warn(R"(DEBUG account="{}", coin="{}", margin_mode={})"sv, account_.name, item.margin_coin, margin_mode);
    margin_coins_[std::string{item.margin_coin}] = margin_mode;
  }
}

// all_positions

void OrderEntry::get_all_positions() {
  profile_.all_positions([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.all_positions;
    auto headers = account_.create_headers(method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("all_positions"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_all_positions_ack(event, sequence);
    });
  });
}

void OrderEntry::get_all_positions_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = OrderEntryState::POSITIONS;
  profile_.all_positions_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::AllPositions all_positions{body, decode_buffer_};
        if (all_positions.code == 0) {
          Trace event{trace_info, all_positions};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(all_positions.code), all_positions.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::AllPositions> const &event) {
  auto &[trace_info, all_positions] = event;
  log::info<4>("all_positions={}"sv, all_positions);
  for (auto &item : all_positions.data) {
    log::warn("DEBUG item={}"sv, item);
    auto position_update = PositionUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.margin_coin,
        .margin_mode = map(item.margin_mode),
        .external_account = {},
        .long_quantity = std::max(item.total, 0.0),    // ???
        .short_quantity = std::max(-item.total, 0.0),  // ???
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = item.c_time,  // ???
        .exchange_sequence = {},
        .sending_time_utc = item.u_time,  // ???
    };
    create_trace_and_dispatch(handler_, trace_info, position_update, true);
  }
}

// orders_history

void OrderEntry::get_orders_history() {
  profile_.orders_history([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.orders_history;
    auto headers = account_.create_headers(method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("orders_history"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_orders_history_ack(event, sequence);
    });
  });
}

void OrderEntry::get_orders_history_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = OrderEntryState::ORDERS;
  profile_.orders_history_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::OrdersHistory orders_history{body, decode_buffer_};
        if (orders_history.code == 0) {
          Trace event{trace_info, orders_history};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(orders_history.code), orders_history.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::OrdersHistory> const &event) {
  auto &[trace_info, orders_history] = event;
  log::info<4>("orders_history={}"sv, orders_history);
  for (auto &item : orders_history.data.entrusted_list) {
    log::warn("DEBUG item={}"sv, item);
  }
}

// order_fill_details

void OrderEntry::get_order_fill_details() {
  profile_.order_fill_details([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.order_management.order_fill_details;
    auto headers = account_.create_headers(method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("order_fill_details"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_order_fill_details_ack(event, sequence);
    });
  });
}

void OrderEntry::get_order_fill_details_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = OrderEntryState::FILLS;
  profile_.order_fill_details_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::OrderFillDetails order_fill_details{body, decode_buffer_};
        if (order_fill_details.code == 0) {
          Trace event{trace_info, order_fill_details};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(order_fill_details.code), order_fill_details.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntry::operator()(Trace<json::OrderFillDetails> const &event) {
  auto &[trace_info, order_fill_details] = event;
  log::info<4>("order_fill_details={}"sv, order_fill_details);
  for (auto &item : order_fill_details.data.fill_list) {
    log::warn("DEBUG item={}"sv, item);
  }
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
    auto body = json::Encoder::place_order(encode_buffer_, create_order, order, request_id, {}, shared_.api.inst_type, shared_.settings.margin_coin);
    log::warn(R"(DEBUG body="{}")"sv, body);
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
    auto body = json::Encoder::modify_order(encode_buffer_, modify_order, order, request_id, {}, shared_.api.inst_type, shared_.settings.margin_coin);
    log::warn(R"(DEBUG body="{}")"sv, body);
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
    auto body = json::Encoder::cancel_order(encode_buffer_, cancel_order, order, request_id, {}, shared_.api.inst_type, shared_.settings.margin_coin);
    log::warn(R"(DEBUG body="{}")"sv, body);
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
    auto body = json::Encoder::cancel_all_orders(encode_buffer_, cancel_all_orders, request_id, {}, shared_.api.inst_type, shared_.settings.margin_coin);
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
        error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, message);
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

}  // namespace bitget_futures
}  // namespace roq
