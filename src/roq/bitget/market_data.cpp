/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/market_data.hpp"

#include <algorithm>

#include "roq/logging.hpp"

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
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
auto const NAME = "md"sv;

auto const SUPPORTS = Mask{
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

auto const PING = "ping"sv;

size_t const MAX_DECODE_BUFFER_DEPTH = 3;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.ws.public_uri;
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

MarketData::MarketData(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, size_t index)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, index_{index}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .subscribe = create_metrics(shared.settings, name_, "subscribe"sv),
          .ticker = create_metrics(shared.settings, name_, "ticker"sv),
          .public_trade = create_metrics(shared.settings, name_, "public_trade"sv),
          .books = create_metrics(shared.settings, name_, "books"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      shared_{shared} {
}

void MarketData::operator()(Event<Start> const &) {
  (*connection_).start();
}

void MarketData::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void MarketData::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (ready()) {
    if (next_ping_ < now) {
      next_ping_ = now + shared_.settings.ws.ping_freq;
      (*connection_).send_text(PING);
    }
  }
}

void MarketData::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.subscribe, metrics::Type::PROFILE)
      .write(profile_.ticker, metrics::Type::PROFILE)
      .write(profile_.public_trade, metrics::Type::PROFILE)
      .write(profile_.books, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void MarketData::subscribe(size_t start_from) {
  if (ready()) {
    subscribe(shared_.symbols.get_slice(index_, start_from));
  }
}

void MarketData::operator()(web::socket::Client::Connected const &) {
}

void MarketData::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MarketData::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void MarketData::operator()(web::socket::Client::Close const &) {
}

void MarketData::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void MarketData::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
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

void MarketData::subscribe(std::span<Symbol const> const &symbols) {
  if (std::empty(symbols)) {
    return;
  }
  subscribe("ticker"sv, symbols);
  subscribe("publicTrade"sv, symbols);
  subscribe("books"sv, symbols);
}

void MarketData::subscribe(std::string_view const &topic, std::span<Symbol const> const &symbols) {
  assert(!std::empty(symbols));
  auto prefix = fmt::format(R"({{"instType":"{}","topic":"{}","symbol":")", shared_.api.inst_type, topic);
  auto separator = fmt::format(R"("}},{})", prefix);
  auto message = fmt::format(
      R"({{)"
      R"("op":"subscribe",)"
      R"("args":[{}{}"}}])"
      R"(}})"sv,
      prefix,
      fmt::join(symbols, separator));
  (*connection_).send_text(message);
}

void MarketData::parse(std::string_view const &message) {
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

void MarketData::operator()(Trace<json::Error> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    log::fatal("error={}"sv, error);
  });
}

void MarketData::operator()(Trace<json::Subscribe> const &event) {
  profile_.subscribe([&]() {
    auto &[trace_info, subscribe] = event;
    log::warn("DEBUG subscribe={}"sv, subscribe);
  });
}

void MarketData::operator()(Trace<json::Ticker> const &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker] = event;
    for (auto &item : ticker.data) {
      auto top_of_book = TopOfBook{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = ticker.arg.symbol,
          .layer{
              .bid_price = item.bid1_price,
              .bid_quantity = item.bid1_size,
              .ask_price = item.ask1_price,
              .ask_quantity = item.ask1_size,
          },
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = ticker.ts,
          .exchange_sequence = {},
          .sending_time_utc = ticker.ts,
      };
      create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
      // XXX mark_price ???
      std::array<Statistics, 7> statistics{{
          {
              .type = StatisticsType::OPEN_PRICE,
              .value = item.open_price24h,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
          {
              .type = StatisticsType::HIGHEST_TRADED_PRICE,
              .value = item.high_price24h,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
          {
              .type = StatisticsType::LOWEST_TRADED_PRICE,
              .value = item.low_price24h,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
          {
              .type = StatisticsType::INDEX_VALUE,
              .value = item.index_price,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
          {
              .type = StatisticsType::TRADE_VOLUME,
              .value = item.volume24h,  // note! not sure...
              .begin_time_utc = {},
              .end_time_utc = {},
          },
          {
              .type = StatisticsType::FUNDING_RATE,
              .value = item.funding_rate,
              .begin_time_utc = {},
              .end_time_utc = utils::safe_cast(item.next_funding_time),  // ???
          },
          {
              .type = StatisticsType::OPEN_INTEREST,
              .value = item.open_interest,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
      }};
      auto statistics_update = StatisticsUpdate{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = ticker.arg.symbol,
          .statistics = statistics,
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = {},  // ???
          .exchange_sequence = {},
          .sending_time_utc = ticker.ts,
      };
      create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    }
  });
}

// XXX TODO should we rather split by trade_item.ts => exchange_time_utc ?
void MarketData::operator()(Trace<json::PublicTrade> const &event) {
  profile_.public_trade([&]() {
    auto &[trace_info, public_trade] = event;
    if (public_trade.action != json::Action::UPDATE) {  // note! drop snapshot
      return;
    }
    auto &trades = shared_.trades;
    trades.clear();
    decltype(json::PublicTradeDataItem::timestamp) timestamp = {};
    for (auto &item : public_trade.data) {
      auto item_2 = Trade{
          .side = map(item.side),
          .price = item.price,
          .quantity = item.size,
          .trade_id = item.trade_id,
          .taker_order_id = {},
          .maker_order_id = {},
      };
      trades.emplace_back(std::move(item_2));
      utils::update_max(timestamp, item.timestamp);
    }
    if (!std::empty(trades)) {
      auto trade_summary = TradeSummary{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = public_trade.arg.symbol,
          .trades = trades,
          .exchange_time_utc = timestamp,
          .exchange_sequence = {},
          .sending_time_utc = public_trade.ts,
      };
      create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
    }
  });
}

void MarketData::operator()(Trace<json::Books> const &event) {
  profile_.books([&]() {
    auto &[trace_info, books] = event;
    auto &bids = shared_.bids;
    auto &asks = shared_.asks;
    bids.clear();
    asks.clear();
    if (std::size(books.data) != 1) [[unlikely]] {
      log::fatal("Unexpected: books={}"sv, books);
    }
    auto &data = books.data[0];
    for (auto &item : data.bids) {
      auto mbp_update = MBPUpdate{
          .price = item.price,
          .quantity = item.size,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
      bids.emplace_back(std::move(mbp_update));
    }
    for (auto &item : data.asks) {
      auto mbp_update = MBPUpdate{
          .price = item.price,
          .quantity = item.size,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
      asks.emplace_back(std::move(mbp_update));
    }
    if (!(std::empty(bids) && std::empty(asks))) {
      auto update_type = [&]() -> UpdateType {
        switch (books.action) {
          using enum json::Action::type_t;
          case UNDEFINED_INTERNAL:
          case UNKNOWN_INTERNAL:
            break;
          case SNAPSHOT:
            return UpdateType::SNAPSHOT;
          case UPDATE:
            return UpdateType::INCREMENTAL;
        }
        log::fatal("Unexpected"sv);
      }();
      auto market_by_price_update = MarketByPriceUpdate{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = books.arg.symbol,
          .bids = bids,
          .asks = asks,
          .update_type = update_type,
          .exchange_time_utc = data.ts,
          .exchange_sequence = data.seq,
          .sending_time_utc = books.ts,
          .price_precision = {},
          .quantity_precision = {},
          .max_depth = {},
          .checksum = {},
      };
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
    }
  });
}

void MarketData::operator()(Trace<json::Login> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::Account> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::Position> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::Order> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::Fill> const &) {
  log::fatal("Unexpected"sv);
}

}  // namespace bitget
}  // namespace roq
