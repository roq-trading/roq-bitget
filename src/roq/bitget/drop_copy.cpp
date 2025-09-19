/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/drop_copy.hpp"

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/bitget/json/map.hpp"
#include "roq/bitget/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::FUNDS,
    SupportType::POSITION,
};

auto const PING = "ping"sv;

size_t const MAX_DECODE_BUFFER_DEPTH = 2;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account.name);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.ws.private_uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared} {
}

bool DropCopy::ready() const {
  return (*connection_).ready();
}

void DropCopy::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopy::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (ready()) {
    if (next_ping_ < now) {
      next_ping_ = now + shared_.settings.ws.ping_freq;
      (*connection_).send_text(PING);
    }
  }
}

void DropCopy::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void DropCopy::operator()(web::socket::Client::Connected const &) {
  assert(logon_timeout_.count() == 0);
  auto now = clock::get_system();
  logon_timeout_ = now + shared_.settings.ws.request_timeout;
}

void DropCopy::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  logon_timeout_ = {};
  next_ping_ = {};
}

void DropCopy::operator()(web::socket::Client::Ready const &) {
  login();
}

void DropCopy::operator()(web::socket::Client::Close const &) {
}

void DropCopy::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopy::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.name,
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
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

void DropCopy::login() {
  auto message = account_.create_ws_login();
  log::debug("message={}"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::subscribe() {
  subscribe("account"sv);
  subscribe("position"sv);
  subscribe("order"sv);
  subscribe("fill"sv);
}

void DropCopy::subscribe(std::string_view const &topic) {
  log::info(R"(Subscribe topic="{}")"sv, topic);
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":[{{)"
      R"("instType":"UTA",)"
      R"("topic":"{}")"
      R"(}})"
      R"(])"
      R"(}})"sv,
      topic);
  log::debug("message={}"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::Parser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void DropCopy::operator()(Trace<json::Error> const &event) {
  auto &[trace_info, error] = event;
  log::fatal("event={{book_ticker={}, trace_info={}}}"sv, error, trace_info);
}

void DropCopy::operator()(Trace<json::Subscribe> const &event) {
  auto &[trace_info, subscribe] = event;
  log::warn("DEBUG subscribe={}"sv, subscribe);
}

void DropCopy::operator()(Trace<json::Ticker> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::PublicTrade> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Books> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Login> const &event) {
  auto &[trace_info, login] = event;
  log::info<2>("login={}"sv, login);
  if (login.code != 0) {
    log::fatal("Unexpected: login={}"sv, login);
  }
  subscribe();
  ready_ = true;
  (*this)(ConnectionStatus::READY);
}

// note! snapshot + incremental
void DropCopy::operator()(Trace<json::Account> const &event) {
  auto &[trace_info, account] = event;
  log::info<2>("account={}"sv, account);
  log::warn("DEBUG account={}"sv, account);
  for (auto &item : account.data) {
    for (auto &item_2 : item.coin) {
      auto funds_update = FundsUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .currency = item_2.coin,
          .margin_mode = {},
          .balance = item_2.balance,
          .hold = item_2.locked,
          .borrowed = item_2.debts,
          .external_account = {},
          .update_type = map(account.action),
          .exchange_time_utc = {},
          .exchange_sequence = {},
          .sending_time_utc = account.ts,
      };
      log::warn("DEBUG funds_update={}"sv, funds_update);
      create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    }
  }
}

// note! snapshot + incremental
void DropCopy::operator()(Trace<json::Position> const &event) {
  auto &[trace_info, position] = event;
  log::info<2>("position={}"sv, position);
  log::warn("DEBUG position={}"sv, position);
  for (auto &item : position.data) {
    auto long_quantity = [&]() -> double {
      if (item.pos_side == json::PosSide::LONG) {
        return item.size;
      }
      return 0.0;
    }();
    auto short_quantity = [&]() -> double {
      if (item.pos_side == json::PosSide::SHORT) {
        return item.size;
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
        .update_type = map(position.action),
        .exchange_time_utc = item.updated_time,
        .exchange_sequence = {},
        .sending_time_utc = position.ts,
    };
    log::warn("DEBUG position_update={}"sv, position_update);
    create_trace_and_dispatch(handler_, trace_info, position_update, true);
  }
}

// note! incremental
void DropCopy::operator()(Trace<json::Order> const &event) {
  auto &[trace_info, order] = event;
  log::info<2>("order={}"sv, order);
  log::warn("DEBUG order={}"sv, order);
  for (auto &item : order.data) {
    auto remaining_quantity = item.qty - item.cum_exec_qty;
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .side = map(item.side),
        .position_effect = map(item.trade_side),  // XXX alt. hold_side + side
        .margin_mode = map(item.margin_mode),
        .max_show_quantity = NaN,
        .order_type = map(item.order_type),
        .time_in_force = map(item.time_in_force),
        .execution_instructions = {},  // XXX order_type + reduce_only
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
        .update_type = map(order.action),
        .sending_time_utc = order.ts,
    };
    log::warn("DEBUG order_update={}"sv, order_update);
    if (shared_.update_order(item.client_oid, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {
          // no fills here
        })) {
    } else {
      log::warn<1>(R"(*** EXTERNAL ORDER *** (order_id="{}", client_oid="{}"))"sv, item.order_id, item.client_oid);
    }
  }
}

// note! incremental
void DropCopy::operator()(Trace<json::Fill> const &event) {
  auto &[trace_info, fill] = event;
  log::info<2>("fill={}"sv, fill);
  log::warn("DEBUG fill={}"sv, fill);
  std::string_view symbol, order_id, client_oid;
  json::Side side = {};
  json::TradeSide trade_side = {};
  std::chrono::nanoseconds exec_time = {};
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
          .create_time_utc = exec_time,
          .update_time_utc = updated_time,
          .external_account = {},
          .external_order_id = order_id,
          .client_order_id = client_oid,
          .fills = shared_.fills,
          .routing_id = {},
          .update_type = map(fill.action),
          .sending_time_utc = fill.ts,
          .user = {},
          .strategy_id = {},
      };
      create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE, client_oid);
      log::warn("DEBUG trade_update={}"sv, trade_update);
      shared_.fills.clear();
    }
  };
  shared_.fills.clear();
  for (auto &item : fill.data) {
    if (item.symbol != symbol || item.order_id != order_id || item.client_oid != client_oid || item.side != side || item.trade_side != trade_side) {
      dispatch();
      symbol = item.symbol;
      order_id = item.order_id;
      client_oid = item.client_oid;
      side = item.side;
      trade_side = item.trade_side;
      exec_time = {};
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
        .exchange_time_utc = item.exec_time,
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
    if (exec_time < item.exec_time) {
      exec_time = item.exec_time;
    }
    if (updated_time < item.updated_time) {
      updated_time = item.updated_time;
    }
  }
  dispatch();
}

}  // namespace bitget
}  // namespace roq
