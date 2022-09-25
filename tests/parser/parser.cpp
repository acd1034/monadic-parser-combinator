#define CATCH_CONFIG_MAIN
#include <algorithm>
#include <catch2/catch.hpp>
#include <mpc/parser.hpp>
#include "../stdfundamental.hpp"
using namespace std::string_view_literals;

// TODO: parser を制約
void CHECK_FAIL(/* mpc::similar_to<mpc::Parser> */ auto&& parser, std::string_view sv) {
  auto result = mpc::eval_StateT % MPC_FORWARD(parser) % mpc::String(sv.begin(), sv.end());
  CHECK(result.index() == 0);
}

// TODO: parser を制約
void CHECK_SUCCEED(/* mpc::similar_to<mpc::Parser> */ auto&& parser, std::string_view sv,
                   const auto& expected) {
  auto result = mpc::eval_StateT % MPC_FORWARD(parser) % mpc::String(sv.begin(), sv.end());
  CHECK(result.index() == 1);
  CHECK(*mpc::snd(result) == expected);
}

// TODO: parser を制約
void CHECK_SUCCEED(/* mpc::similar_to<mpc::Parser> */ auto&& parser, std::string_view sv,
                   const std::string_view& expected) {
  auto result = mpc::eval_StateT % MPC_FORWARD(parser) % mpc::String(sv.begin(), sv.end());
  CHECK(result.index() == 1);
  constexpr auto equal = [](const mpc::String& x, const std::string_view& y) {
    return std::equal(x.begin(), x.end(), y.begin(), y.end());
  };
  CHECK(equal(*mpc::snd(result), expected));
}

TEST_CASE("parser min", "[parser][min]") {
  CHECK_SUCCEED(mpc::satisfy % mpc::isdigit, "0", '0');
  CHECK_FAIL(mpc::satisfy % mpc::isdigit, "a");
  CHECK_SUCCEED(mpc::char1 % 'a', "a", 'a');
  CHECK_FAIL(mpc::char1 % 'a', "b");
  CHECK_SUCCEED(mpc::string % "abc"sv, "abc", "abc"sv);
  CHECK_FAIL(mpc::string % "abc"sv, "abd");
  {
    const auto alpha = mpc::satisfy % mpc::isalpha;
    CHECK_SUCCEED(mpc::many % alpha, "abc0", "abc"sv);
    CHECK_SUCCEED(mpc::many % alpha, "0", ""sv);
  }
  {
    const auto alnum = mpc::satisfy % mpc::isalnum;
    CHECK_SUCCEED(mpc::many1 % alnum, "abc012 ", "abc012"sv);
    CHECK_FAIL(mpc::many1 % alnum, " ");
  }
  CHECK_SUCCEED(mpc::alnum, "a", 'a');
  CHECK_FAIL(mpc::alnum, ",");
  CHECK_SUCCEED(mpc::alpha, "b", 'b');
  CHECK_FAIL(mpc::alpha, "1");
  CHECK_SUCCEED(mpc::digit, "1", '1');
  CHECK_FAIL(mpc::digit, "b");
  CHECK_SUCCEED(mpc::any_char, "`", '`');
  CHECK_FAIL(mpc::any_char, "");
}
