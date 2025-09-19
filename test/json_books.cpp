/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget/json/books.hpp"
#include "roq/bitget/json/parser.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

TEST_CASE("simple", "[json_books]") {
  auto message = R"({)"
                 R"("action":"update",)"
                 R"("arg":{)"
                 R"("instType":"usdt-futures",)"
                 R"("topic":"books",)"
                 R"("symbol":"ETHUSDT")"
                 R"(},)"
                 R"("data":[{)"
                 R"("a":[)"
                 R"(["4488.46","1.52"],)"
                 R"(["4488.48","0"],)"
                 R"(["4488.50","7.64"],)"
                 R"(["4488.83","0"],)"
                 R"(["4488.85","1.74"],)"
                 R"(["4488.89","5.99"],)"
                 R"(["4491.08","81.37"],)"
                 R"(["4493.33","88.85"],)"
                 R"(["4495.58","94.60"],)"
                 R"(["4497.83","90.57"],)"
                 R"(["4500.08","84.82"],)"
                 R"(["4502.33","91.72"])"
                 R"(],)"
                 R"("b":[)"
                 R"(["4487.66","34.16"],)"
                 R"(["4487.58","65.61"],)"
                 R"(["4486.57","2.34"],)"
                 R"(["4486.56","2.89"],)"
                 R"(["4483.89","92.30"],)"
                 R"(["4481.64","81.37"],)"
                 R"(["4479.39","84.82"],)"
                 R"(["4477.14","91.72"],)"
                 R"(["4474.89","92.87"])"
                 R"(],)"
                 R"("checksum":1990051938,)"
                 R"("pseq":1352182127478939650,)"
                 R"("seq":1352182127898370048,)"
                 R"("ts":"1758111262297")"
                 R"(})"
                 R"(],)"
                 R"("ts":1758111262300)"
                 R"(})";
  core::json::BufferStack buffer_stack{8192, 3};
  struct MyHandler : public json::Parser::Handler {
    void operator()(Trace<json::Error> const &) { FAIL(); }
    void operator()(Trace<json::Subscribe> const &) { FAIL(); }
    void operator()(Trace<json::Ticker> const &) { FAIL(); }
    void operator()(Trace<json::PublicTrade> const &) { FAIL(); }
    void operator()(Trace<json::Books> const &event) {
      ++count;
      auto &[trace_info, books] = event;
      CHECK(books.action == json::Action::UPDATE);
      CHECK(books.arg.symbol == "ETHUSDT"sv);
      REQUIRE(std::size(books.data) == 1);
      auto &data = books.data[0];
      auto &asks = data.asks;
      REQUIRE(std::size(asks) == 12);
      CHECK(asks[0].price == Catch::Approx{4488.46});
      CHECK(asks[0].size == Catch::Approx{1.52});
      CHECK(asks[11].price == Catch::Approx{4502.33});
      CHECK(asks[11].size == Catch::Approx{91.72});
      auto &bids = data.bids;
      REQUIRE(std::size(bids) == 9);
      CHECK(bids[0].price == Catch::Approx{4487.66});
      CHECK(bids[0].size == Catch::Approx{34.16});
      CHECK(bids[8].price == Catch::Approx{4474.89});
      CHECK(bids[8].size == Catch::Approx{92.87});
      CHECK(data.checksum == 1990051938);
      CHECK(data.pseq == 1352182127478939650);
      CHECK(data.seq == 1352182127898370048);
      CHECK(data.ts == 1758111262297ms);
      CHECK(books.ts == 1758111262300ms);
    }
    void operator()(Trace<json::Login> const &) { FAIL(); }
    void operator()(Trace<json::Account> const &) { FAIL(); }
    void operator()(Trace<json::Position> const &) { FAIL(); }
    void operator()(Trace<json::Order> const &) { FAIL(); }
    void operator()(Trace<json::Fill> const &) { FAIL(); }
    size_t count = 0;
  } handler;
  TraceInfo trace_info;
  json::Parser::dispatch(handler, message, buffer_stack, trace_info);
  CHECK(handler.count == 1);
}
