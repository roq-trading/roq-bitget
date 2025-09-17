/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget_futures/rest.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/bitget_futures/json/message.hpp"
#include "roq/bitget_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace bitget_futures {

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

std::string_view extract_symbol(std::string_view const &symbol) {
  auto separator = symbol.find_first_of('_');
  return symbol.substr(0, separator);
}
}  // namespace

// === IMPLEMENTATION ===

Rest::Rest(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .currencies = create_metrics(shared.settings, name_, "currencies"sv),
          .currencies_ack = create_metrics(shared.settings, name_, "currencies_ack"sv),
          .contracts = create_metrics(shared.settings, name_, "contracts"sv),
          .contracts_ack = create_metrics(shared.settings, name_, "contracts_ack"sv),
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
      .write(profile_.currencies, metrics::Type::PROFILE)
      .write(profile_.currencies_ack, metrics::Type::PROFILE)
      .write(profile_.contracts, metrics::Type::PROFILE)
      .write(profile_.contracts_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
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

void Rest::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
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

uint32_t Rest::download(RestState state) {
  switch (state) {
    using enum RestState;
    case UNDEFINED:
      assert(false);
      break;
    case CURRENCIES:
      get_currencies();
      return 1;
    case CONTRACTS:
      get_contracts();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

// currencies

void Rest::get_currencies() {
  profile_.currencies([&]() {
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.market_data.currencies,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("currencies"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_currencies_ack(event, sequence);
    });
  });
}

void Rest::get_currencies_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = RestState::CURRENCIES;
  profile_.currencies_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::Currencies currencies{body, decode_buffer_};
        if (currencies.code == 0) {
          Trace event{trace_info, currencies};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(currencies.code), currencies.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<json::Currencies> const &event) {
  auto &[trace_info, currencies] = event;
  log::info<4>("currencies={}"sv, currencies);
}

// contracts

void Rest::get_contracts() {
  profile_.contracts([&]() {
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.market_data.contracts,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    (*connection_)("market-contracts"sv, request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_contracts_ack(event, sequence);
    });
  });
}

void Rest::get_contracts_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const state = RestState::CONTRACTS;
  profile_.contracts_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(state);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
      } else {
        json::Contracts contracts{body, decode_buffer_};
        if (contracts.code == 0) {
          Trace event{trace_info, contracts};
          (*this)(event);
          download_.check(state);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(contracts.code), contracts.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<json::Contracts> const &event) {
  auto &[trace_info, contracts] = event;
  log::info<4>("contracts={}"sv, contracts);
  std::vector<Symbol> symbols;
  symbols.reserve(std::size(contracts.data));
  size_t counter = 0;
  for (size_t i = 0; i < std::size(contracts.data); ++i) {
    auto &item = contracts.data[i];
    log::info<2>("item={}"sv, item);
    auto symbol = item.symbol;
    if (std::empty(symbol)) {
      symbol = extract_symbol(item.symbol);  // note! legacy had some extension
    }
    if (shared_.discard_symbol(symbol)) {
      continue;
    }
    if (all_symbols_.emplace(symbol).second) {  // only include new
      symbols.emplace_back(symbol);
    }
    ++counter;
    auto security_type = [&]() -> SecurityType {
      switch (item.symbol_type) {
        using enum json::SymbolType::type_t;
        case UNDEFINED_INTERNAL:
          break;
        case UNKNOWN_INTERNAL:
          assert(false);
          break;
        case PERPETUAL:
          return SecurityType::SWAP;
        case DELIVERY:
          return SecurityType::FUTURES;
      }
      return {};
    }();
    // XXX TODO how to detect linear vs inverse futures?
    auto tick_size = std::pow(10.0, -static_cast<double>(item.price_place));
    auto reference_data = ReferenceData{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = symbol,
        .description = {},
        .security_type = security_type,
        .cfi_code = {},
        .base_currency = item.base_coin,
        .quote_currency = item.quote_coin,
        .settlement_currency = {},
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = tick_size,
        .tick_size_steps = {},
        .multiplier = item.size_multiplier,  // XXX TODO or inverse of this ???
        .min_notional = NaN,
        .min_trade_vol = item.min_trade_num,
        .max_trade_vol = NaN,
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
        .sending_time_utc = contracts.request_time,
        .discard = {},
    };
    create_trace_and_dispatch(handler_, trace_info, reference_data, true);
    // XXX FIXME TODO symbol_status
  }
  if (!std::empty(symbols)) {
    auto contracts_update = SymbolsUpdate{
        .symbols = symbols,
    };
    handler_(contracts_update);
  }
  if (counter > 0) [[unlikely]] {
    log::info("Symbols {} / {}"sv, counter, std::size(contracts.data));
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
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}

}  // namespace bitget_futures
}  // namespace roq
