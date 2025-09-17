/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitget_futures/json/books.hpp"
#include "roq/bitget_futures/json/parser.hpp"

using namespace roq;
using namespace roq::bitget_futures;

using namespace std::literals;

TEST_CASE("json_books_update", "[json_books]") {
  auto message = R"({)"
                 R"("action":"update",)"
                 R"("arg":{)"
                 R"("instType":"mc",)"
                 R"("channel":"books",)"
                 R"("instId":"SUIUSD")"
                 R"(},)"
                 R"("data":[{)"
                 R"("asks":[)"
                 R"(["3.3671","59.1"],)"
                 R"(["3.3672","59.5"],)"
                 R"(["3.3673","87.7"],)"
                 R"(["3.3675","22.3"],)"
                 R"(["3.3677","345.8"],)"
                 R"(["3.3698","199.7"],)"
                 R"(["3.3699","1583.6"],)"
                 R"(["3.3715","267.2"],)"
                 R"(["3.3930","6166.8"],)"
                 R"(["3.3947","6196.8"],)"
                 R"(["3.3964","6466.9"],)"
                 R"(["3.3981","6526.9"],)"
                 R"(["3.3998","6797.0"],)"
                 R"(["3.4015","6406.9"],)"
                 R"(["3.4032","6226.9"],)"
                 R"(["3.4049","7397.2"],)"
                 R"(["3.4066","6917.1"],)"
                 R"(["3.4083","6587.0"],)"
                 R"(["3.4100","6707.0"],)"
                 R"(["3.4117","6617.0"],)"
                 R"(["3.4134","7277.2"],)"
                 R"(["3.4151","6737.0"],)"
                 R"(["3.4168","7367.2"],)"
                 R"(["3.4185","7007.1"],)"
                 R"(["3.4202","6977.1"],)"
                 R"(["3.4219","7307.2"],)"
                 R"(["3.4236","6767.0"],)"
                 R"(["3.4253","7127.1"],)"
                 R"(["3.4270","6647.0"],)"
                 R"(["3.4287","7037.1"],)"
                 R"(["3.4304","6496.9"],)"
                 R"(["3.4321","7097.1"],)"
                 R"(["3.4338","6947.1"],)"
                 R"(["3.4355","6436.9"],)"
                 R"(["3.4372","6136.8"],)"
                 R"(["3.4389","6346.9"],)"
                 R"(["3.4406","6827.0"],)"
                 R"(["3.4423","7217.2"],)"
                 R"(["3.4440","6286.9"],)"
                 R"(["3.4457","6557.0"],)"
                 R"(["3.4474","6857.0"],)"
                 R"(["3.4491","6316.9"],)"
                 R"(["3.4508","6256.9"],)"
                 R"(["3.4525","6376.9"],)"
                 R"(["3.4542","7187.1"],)"
                 R"(["3.4559","6677.0"],)"
                 R"(["3.4576","7337.2"],)"
                 R"(["3.4593","7067.1"],)"
                 R"(["3.4610","6887.1"],)"
                 R"(["3.4627","6106.8"],)"
                 R"(["3.4644","7247.2"],)"
                 R"(["3.4661","7157.1"])"
                 R"(],)"
                 R"("bids":[)"
                 R"(["3.3653","17.0"],)"
                 R"(["3.3648","122.8"],)"
                 R"(["3.3639","261.0"],)"
                 R"(["3.3637","883.2"],)"
                 R"(["3.3390","7550.6"],)"
                 R"(["3.3373","6761.0"],)"
                 R"(["3.3356","7583.5"],)"
                 R"(["3.3339","7419.0"],)"
                 R"(["3.3322","6793.9"],)"
                 R"(["3.3305","6695.2"],)"
                 R"(["3.3288","7386.1"],)"
                 R"(["3.3271","6464.9"],)"
                 R"(["3.3254","7057.1"],)"
                 R"(["3.3237","7451.9"],)"
                 R"(["3.3220","7649.3"],)"
                 R"(["3.3203","7287.4"],)"
                 R"(["3.3186","7024.2"],)"
                 R"(["3.3169","6399.1"],)"
                 R"(["3.3152","7122.9"],)"
                 R"(["3.3135","6991.3"],)"
                 R"(["3.3118","6432.0"],)"
                 R"(["3.3101","6958.4"],)"
                 R"(["3.3084","7221.6"],)"
                 R"(["3.3067","6859.7"],)"
                 R"(["3.3050","6563.6"],)"
                 R"(["3.3033","7682.2"],)"
                 R"(["3.3016","6662.3"],)"
                 R"(["3.2999","7320.3"],)"
                 R"(["3.2982","6728.1"],)"
                 R"(["3.2965","7748.0"],)"
                 R"(["3.2948","7517.7"],)"
                 R"(["3.2931","7188.7"],)"
                 R"(["3.2914","6497.8"],)"
                 R"(["3.2897","6925.5"],)"
                 R"(["3.2880","6629.4"],)"
                 R"(["3.2863","6892.6"],)"
                 R"(["3.2846","7353.2"],)"
                 R"(["3.2829","7155.8"],)"
                 R"(["3.2812","6826.8"],)"
                 R"(["3.2795","7715.1"],)"
                 R"(["3.2778","6530.7"],)"
                 R"(["3.2761","6596.5"],)"
                 R"(["3.2744","7616.4"],)"
                 R"(["3.2727","7254.5"],)"
                 R"(["3.2710","7484.8"],)"
                 R"(["3.2693","7090.0"],)"
                 R"(["2.9000","0"])"
                 R"(],)"
                 R"("checksum":-618973030,)"
                 R"("seq":1344048307307175949,)"
                 R"("ts":"1756172008349")"
                 R"(})"
                 R"(],)"
                 R"("ts":1756172008351)"
                 R"(})";
  core::json::BufferStack buffer_stack{8192, 3};
  struct MyHandler : public json::Parser::Handler {
    void operator()(Trace<json::Error> const &) { FAIL(); }
    void operator()(Trace<json::Ticker> const &) { FAIL(); }
    void operator()(Trace<json::Trade> const &) { FAIL(); }
    void operator()(Trace<json::Books> const &event) {
      ++count;
      auto &[trace_info, books] = event;
      CHECK(books.action == json::Action::UPDATE);
      CHECK(books.arg.inst_id == "SUIUSD"sv);
      REQUIRE(std::size(books.data) == 1);
      auto &data = books.data[0];
      auto &asks = data.asks;
      REQUIRE(std::size(asks) == 52);
      CHECK(asks[0].price == Catch::Approx{3.3671});
      CHECK(asks[0].size == Catch::Approx{59.1});
      CHECK(asks[51].price == Catch::Approx{3.4661});
      CHECK(asks[51].size == Catch::Approx{7157.1});
      auto &bids = data.bids;
      REQUIRE(std::size(bids) == 47);
      CHECK(bids[0].price == Catch::Approx{3.3653});
      CHECK(bids[0].size == Catch::Approx{17.0});
      CHECK(bids[46].price == Catch::Approx{2.9000});
      CHECK(bids[46].size == Catch::Approx{0.0});
      CHECK(data.checksum == -618973030);
      CHECK(data.seq == 1344048307307175949);
      CHECK(data.ts == 1756172008349ms);
      CHECK(books.ts == 1756172008351ms);
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
