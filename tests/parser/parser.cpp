#define CATCH_CONFIG_MAIN
#include <algorithm>
#include <catch2/catch.hpp>
#include <mpc/parser.hpp>
#include "../stdfundamental.hpp"
using namespace std::string_view_literals;

// TODO: parser を is_Parser<T> で制約
void CHECK_FAIL(auto&& parser, std::string_view sv) {
  auto result = mpc::eval_StateT % MPC_FORWARD(parser) % mpc::String(sv.begin(), sv.end());
  CHECK(result.index() == 0);
}

// TODO: parser を is_Parser<T> で制約
void CHECK_SUCCEED(auto&& parser, std::string_view sv, const auto& expected) {
  auto result = mpc::eval_StateT % MPC_FORWARD(parser) % mpc::String(sv.begin(), sv.end());
  CHECK(result.index() == 1);
  CHECK(*mpc::snd(result) == expected);
}

inline constexpr auto string_equal = [](const mpc::String& x, const std::string_view& y) {
  return std::equal(x.begin(), x.end(), y.begin(), y.end());
};

// TODO: parser を is_Parser<T> で制約
void CHECK_SUCCEED(auto&& parser, std::string_view sv, const std::string_view& expected) {
  auto result = mpc::eval_StateT % MPC_FORWARD(parser) % mpc::String(sv.begin(), sv.end());
  CHECK(result.index() == 1);
  CHECK(string_equal(*mpc::snd(result), expected));
}

// TODO: parser を is_Parser<T> で制約
void CHECK_SUCCEED(auto&& parser, std::string_view sv,
                   const std::list<std::string_view>& expected) {
  auto result = mpc::eval_StateT % MPC_FORWARD(parser) % mpc::String(sv.begin(), sv.end());
  CHECK(result.index() == 1);
  constexpr auto equal = [](const auto& x, const auto& y) {
    return std::equal(x.begin(), x.end(), y.begin(), y.end(), string_equal);
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
  {
    const auto alpha_digit = mpc::sequence % std::list{mpc::alpha, mpc::digit};
    CHECK_SUCCEED(alpha_digit, "a1", "a1"sv);
    CHECK_FAIL(alpha_digit, "1a");
  }
  {
    const auto alpha_digits = mpc::liftA2(mpc::cons, mpc::alpha, mpc::many % mpc::digit);
    CHECK_SUCCEED(alpha_digits, "a1234", "a1234"sv);
    CHECK_SUCCEED(alpha_digits, "ab1234", "a"sv);
    CHECK_FAIL(alpha_digits, "1234");
  }
  {
    const auto alphas_digits =
      mpc::liftA2(mpc::append, mpc::many % mpc::alpha, mpc::many % mpc::digit);
    CHECK_SUCCEED(alphas_digits, "abc123", "abc123"sv);
    CHECK_SUCCEED(alphas_digits, "123abc", "123"sv);
  }
  {
    const auto alpha_sep_by_comma = mpc::sep_by1(mpc::alpha, mpc::char1 % ',');
    CHECK_SUCCEED(alpha_sep_by_comma, "a,b,c", "abc"sv);
    CHECK_SUCCEED(alpha_sep_by_comma, "ab", "a"sv);
    CHECK_FAIL(alpha_sep_by_comma, ",");
  }
  {
    const auto braced_digits =
      mpc::between(mpc::char1 % '{', mpc::many % mpc::digit, mpc::char1 % '}');
    CHECK_SUCCEED(braced_digits, "{123}", "123"sv);
    CHECK_FAIL(braced_digits, "123}");
    CHECK_FAIL(braced_digits, "{123");
  }
}

TEST_CASE("parser parse_test", "[parser][parse_test]") {
  // Reference: https://qiita.com/7shi/items/b8c741e78a96ea2c10fe

  using namespace mpc::operators::alternatives;
  const auto test1 = mpc::sequence % std::list{mpc::any_char, mpc::any_char};
  const auto test2 = mpc::sequence % std::list{mpc::any_char, mpc::any_char, mpc::any_char};
  const auto test3 = mpc::sequence % std::list{mpc::alpha, mpc::digit, mpc::digit};
  const auto test4 = mpc::alpha or mpc::digit;
  const auto test5 = mpc::sequence % std::list{mpc::alpha, mpc::digit, mpc::digit, mpc::digit};
  // test6 = sequence $ letter : replicate 3 digit
  const auto test7 = mpc::many % mpc::alpha;
  const auto test8 = mpc::many % test4;
  const auto test9 = mpc::sequence % std::list{mpc::char1 % 'a', mpc::char1 % 'b'}
                     or mpc::sequence % std::list{mpc::char1 % 'a', mpc::char1 % 'c'};
  const auto test10 = mpc::try1(mpc::sequence % std::list{mpc::char1 % 'a', mpc::char1 % 'b'})
                      or mpc::sequence % std::list{mpc::char1 % 'a', mpc::char1 % 'c'};
  const auto test11 = mpc::string % "ab" or mpc::string % "ac";
  const auto test12 = mpc::try1(mpc::string % "ab") or mpc::string % "ac";
  const auto test13 =
    mpc::sequence % std::list{mpc::char1 % 'a', mpc::char1 % 'b' or mpc::char1 % 'c'};

  CHECK_FAIL(mpc::any_char, "");
  CHECK_SUCCEED(mpc::any_char, "abc", 'a');
  CHECK_SUCCEED(test1, "abc", "ab"sv);
  CHECK_SUCCEED(test2, "abc", "abc"sv);
  CHECK_FAIL(test2, "12");
  CHECK_SUCCEED(test2, "123", "123"sv);
  CHECK_SUCCEED(mpc::char1 % 'a', "abc", 'a');
  CHECK_FAIL(mpc::char1 % 'a', "123");
  CHECK_FAIL(mpc::digit, "abc");
  CHECK_SUCCEED(mpc::digit, "123", '1');
  CHECK_SUCCEED(mpc::alpha, "abc", 'a');
  CHECK_FAIL(mpc::alpha, "123");
  CHECK_FAIL(test3, "abc");
  CHECK_FAIL(test3, "123");
  CHECK_SUCCEED(test3, "a23", "a23"sv);
  CHECK_SUCCEED(test3, "a234", "a23"sv);
  CHECK_SUCCEED(test4, "a", 'a');
  CHECK_SUCCEED(test4, "1", '1');
  CHECK_FAIL(test4, "!");
  CHECK_SUCCEED(test5, "a123", "a123"sv);
  CHECK_FAIL(test5, "ab123");
  // CHECK_SUCCEED(test6, "a123", "a123"sv);
  // CHECK_FAIL(test6, "ab123");
  CHECK_SUCCEED(test7, "abc123", "abc"sv);
  CHECK_SUCCEED(test7, "123abc", ""sv);
  CHECK_SUCCEED(test8, "abc123", "abc123"sv);
  CHECK_SUCCEED(test8, "123abc", "123abc"sv);
  CHECK_SUCCEED(test9, "ab", "ab"sv);
  CHECK_SUCCEED(test9, "ac", "ac"sv);
  CHECK_SUCCEED(test10, "ab", "ab"sv);
  CHECK_SUCCEED(test10, "ac", "ac"sv);
  CHECK_SUCCEED(test11, "ab", "ab"sv);
  CHECK_SUCCEED(test11, "ac", "ac"sv);
  CHECK_SUCCEED(test12, "ab", "ab"sv);
  CHECK_SUCCEED(test12, "ac", "ac"sv);
  CHECK_SUCCEED(test13, "ab", "ab"sv);
  CHECK_SUCCEED(test13, "ac", "ac"sv);
}

TEST_CASE("parser ident", "[parser][ident]") {
  // Reference: https://deepsource.io/blog/monadic-parser-combinators/

  using namespace mpc::operators::alternatives;
  const auto ident1 = mpc::alpha or mpc::char1 % '_';
  const auto ident2 = mpc::alnum or mpc::char1 % '_';
  const auto ident = mpc::liftA2(mpc::cons, ident1, mpc::many % ident2);
  CHECK_SUCCEED(ident, "__cplu5plu5", "__cplu5plu5"sv);
  CHECK_FAIL(ident, "1");

  const auto id_list = mpc::sep_by1(ident, mpc::char1 % ',');
  CHECK_SUCCEED(id_list, "Alice,Bolice,Chris", std::list{"Alice"sv, "Bolice"sv, "Chris"sv});
  CHECK_SUCCEED(id_list, "Alice", std::list{"Alice"sv});
  CHECK_SUCCEED(id_list, "Alice ", std::list{"Alice"sv});
  CHECK_SUCCEED(id_list, "Alice,", std::list{"Alice"sv});
  CHECK_FAIL(id_list, ",");
}
