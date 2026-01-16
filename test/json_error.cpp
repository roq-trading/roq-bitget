/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::bitget;

using namespace std::literals;

using value_type = json::Error;

TEST_CASE("param", "[json_error]") {
  auto message = R"({)"
                 R"("event":"error",)"
                 R"("id":"IQAC76UdfkwAAQAAAAAA",)"
                 R"("code":"41101",)"
                 R"("msg":"Param category=USDT-FUTURES error")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "IQAC76UdfkwAAQAAAAAA"sv);
    CHECK(obj.code == 41101);
    CHECK(obj.msg == "Param category=USDT-FUTURES error"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("insufficient_margin", "[json_error]") {
  auto message = R"({)"
                 R"("event":"error",)"
                 R"("id":"IwACRLKjfkwAAQAAAAAA",)"
                 R"("code":"25203",)"
                 R"("msg":"Insufficient margin")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "IwACRLKjfkwAAQAAAAAA"sv);
    CHECK(obj.code == 25203);
    CHECK(obj.msg == "Insufficient margin"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
