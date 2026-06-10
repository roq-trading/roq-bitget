/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/bitget/gateway/rest.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/bitget/protocol/json/map.hpp"
#include "roq/bitget/protocol/json/message.hpp"
#include "roq/bitget/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const NAME = "rest"sv;

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 2;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
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

Rest::Rest(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .instruments = create_metrics(shared.settings, name_, "instruments"sv),
          .instruments_ack = create_metrics(shared.settings, name_, "instruments_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      shared_{shared}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
}

void Rest::operator()(Event<Start> const &) {
  (*connection_).start();
}

void Rest::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void Rest::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
}

void Rest::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.instruments, metrics::Type::PROFILE)
      .write(profile_.instruments_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void Rest::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = {},
      .supports = SUPPORTS,
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
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

// web::rest::Client::Handler

void Rest::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    download_.begin();
  }
}

void Rest::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
}

void Rest::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(State state) {
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case INSTRUMENTS:
      (*this)(ConnectionStatus::DOWNLOADING, "instruments"sv);
      get_instruments();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

// instruments

void Rest::get_instruments() {
  profile_.instruments([&]() {
    auto query = fmt::format("?category={}"sv, shared_.api.category);
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.market_data.instruments,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("instruments"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_instruments_ack(event, sequence);
    });
  });
}

void Rest::get_instruments_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = State::INSTRUMENTS;
  profile_.instruments_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        protocol::json::InstrumentsAck instruments_ack{body, decode_buffer_};
        if (instruments_ack.code == 0) {
          Trace event{trace_info, instruments_ack};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, protocol::json::guess_error(instruments_ack.code), instruments_ack.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<protocol::json::InstrumentsAck> const &event) {
  auto &[trace_info, instruments_ack] = event;
  log::info<4>("instruments_ack={}"sv, instruments_ack);
  std::vector<Symbol> symbols;
  symbols.reserve(std::size(instruments_ack.data));
  size_t counter = 0;
  for (size_t i = 0; i < std::size(instruments_ack.data); ++i) {
    auto &item = instruments_ack.data[i];
    log::info<2>("item={}"sv, item);
    auto discard = shared_.discard_symbol(item.symbol);
    auto settlement_currency = [&]() -> std::string_view {
      switch (item.category) {
        using enum protocol::json::Category::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          break;
        case SPOT:
          return item.quote_coin;  // ???
        case MARGIN:
          return item.quote_coin;  // ???
        case USDT_FUTURES:
          return item.base_coin;  // inverse ???
        case USDC_FUTURES:
          return item.quote_coin;  // linear ???
        case COIN_FUTURES:
          return item.quote_coin;  // linear ???
      }
      return {};
    }();
    auto tick_size = std::pow(10.0, -static_cast<double>(item.price_precision));
    auto reference_data = ReferenceData{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .description = {},
        .security_type = map(item.category, item.type),
        .external_security_id = {},
        .cfi_code = {},
        .base_currency = item.base_coin,
        .quote_currency = item.quote_coin,
        .settlement_currency = settlement_currency,
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = tick_size,
        .tick_size_steps = {},
        .multiplier = item.quantity_multiplier,  // XXX ???
        .min_notional = NaN,
        .min_trade_vol = item.min_order_qty,
        .max_trade_vol = item.max_order_qty,
        .trade_vol_step_size = NaN,
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = {},
        .time_zone = {},
        .issue_date = {},
        .settlement_date = {},
        .expiry_datetime = {},
        .expiry_datetime_utc = {},
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = instruments_ack.request_time,
        .discard = discard,
    };
    create_trace_and_dispatch(handler_, trace_info, reference_data, true);
    if (discard) {
      log::info<1>(R"(Drop symbol="{}")"sv, item.symbol);
      continue;
    }
    auto market_status = MarketStatus{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .trading_status = map(item.status),
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = instruments_ack.request_time,
    };
    create_trace_and_dispatch(handler_, trace_info, market_status, true);
    if (shared_.all_symbols.emplace(item.symbol).second) {  // only include new
      symbols.emplace_back(item.symbol);
    }
    ++counter;
  }
  if (!std::empty(symbols)) {
    auto instruments_update = SymbolsUpdate{
        .symbols = symbols,
    };
    handler_(instruments_update);
  }
  if (counter > 0) [[unlikely]] {
    log::info("Symbols {} / {}"sv, counter, std::size(instruments_ack.data));
  }
}

// helpers

void Rest::process_response(web::rest::Response const &response, auto error_handler, auto success_handler) {
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
