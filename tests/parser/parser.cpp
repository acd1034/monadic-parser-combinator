#include <algorithm>
#include <charconv>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <mpc/parser.hpp>
#include "../stdfundamental.hpp"
using namespace std::string_view_literals;

struct FailsToParse : Catch::Matchers::MatcherGenericBase {
private:
  std::string_view sv_{};

public:
  explicit FailsToParse(std::string_view sv) : sv_(std::move(sv)) {}
  bool match(const auto& parser) const {
    auto result = mpc::eval_StateT % parser % mpc::String(sv_.begin(), sv_.end());
    return result.index() == 0;
  }
  std::string describe() const override {
    std::ostringstream ss;
    ss << "fails to parse " << '"' << sv_ << '"';
    return ss.str();
  }
};

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
  CHECK_THAT(mpc::satisfy % mpc::isdigit, FailsToParse("a"));
  CHECK_SUCCEED(mpc::char1 % 'a', "a", 'a');
  CHECK_THAT(mpc::char1 % 'a', FailsToParse("b"));
  CHECK_SUCCEED(mpc::string % "abc"sv, "abc", "abc"sv);
  CHECK_THAT(mpc::string % "abc"sv, FailsToParse("abd"));
  {
    const auto alpha = mpc::satisfy % mpc::isalpha;
    CHECK_SUCCEED(mpc::many % alpha, "abc0", "abc"sv);
    CHECK_SUCCEED(mpc::many % alpha, "0", ""sv);
  }
  {
    const auto alnum = mpc::satisfy % mpc::isalnum;
    CHECK_SUCCEED(mpc::many1 % alnum, "abc012 ", "abc012"sv);
    CHECK_THAT(mpc::many1 % alnum, FailsToParse(" "));
  }
  CHECK_SUCCEED(mpc::alnum, "a", 'a');
  CHECK_THAT(mpc::alnum, FailsToParse(","));
  CHECK_SUCCEED(mpc::alpha, "b", 'b');
  CHECK_THAT(mpc::alpha, FailsToParse("1"));
  CHECK_SUCCEED(mpc::digit, "1", '1');
  CHECK_THAT(mpc::digit, FailsToParse("b"));
  CHECK_SUCCEED(mpc::any_char, "`", '`');
  CHECK_THAT(mpc::any_char, FailsToParse(""));
  {
    const auto alpha_digit = mpc::sequence % std::list{mpc::alpha, mpc::digit};
    CHECK_SUCCEED(alpha_digit, "a1", "a1"sv);
    CHECK_THAT(alpha_digit, FailsToParse("1a"));
  }
  {
    const auto alpha_digits = mpc::liftA2(mpc::cons, mpc::alpha, mpc::many % mpc::digit);
    CHECK_SUCCEED(alpha_digits, "a1234", "a1234"sv);
    CHECK_SUCCEED(alpha_digits, "ab1234", "a"sv);
    CHECK_THAT(alpha_digits, FailsToParse("1234"));
  }
  {
    const auto alphas_digits =
      mpc::liftA2(mpc::append, mpc::many % mpc::alpha, mpc::many % mpc::digit);
    CHECK_SUCCEED(alphas_digits, "abc123", "abc123"sv);
    CHECK_SUCCEED(alphas_digits, "123abc", "123"sv);
  }
  {
    const auto alpha_sep_by1_comma = mpc::sep_by1(mpc::alpha, mpc::char1 % ',');
    CHECK_SUCCEED(alpha_sep_by1_comma, "a,b,c", "abc"sv);
    CHECK_SUCCEED(alpha_sep_by1_comma, "ab", "a"sv);
    CHECK_THAT(alpha_sep_by1_comma, FailsToParse(","));
    CHECK_THAT(alpha_sep_by1_comma, FailsToParse(""));
  }
  {
    const auto alpha_sep_by_comma = mpc::sep_by(mpc::alpha, mpc::char1 % ',');
    CHECK_SUCCEED(alpha_sep_by_comma, "a,b,c", "abc"sv);
    CHECK_SUCCEED(alpha_sep_by_comma, "ab", "a"sv);
    CHECK_SUCCEED(alpha_sep_by_comma, "", ""sv);
    CHECK_SUCCEED(alpha_sep_by_comma, ",", ""sv);
  }
  {
    const auto braced_digits =
      mpc::between(mpc::char1 % '{', mpc::many % mpc::digit, mpc::char1 % '}');
    CHECK_SUCCEED(braced_digits, "{123}", "123"sv);
    CHECK_THAT(braced_digits, FailsToParse("123}"));
    CHECK_THAT(braced_digits, FailsToParse("{123"));
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

  CHECK_THAT(mpc::any_char, FailsToParse(""));
  CHECK_SUCCEED(mpc::any_char, "abc", 'a');
  CHECK_SUCCEED(test1, "abc", "ab"sv);
  CHECK_SUCCEED(test2, "abc", "abc"sv);
  CHECK_THAT(test2, FailsToParse("12"));
  CHECK_SUCCEED(test2, "123", "123"sv);
  CHECK_SUCCEED(mpc::char1 % 'a', "abc", 'a');
  CHECK_THAT(mpc::char1 % 'a', FailsToParse("123"));
  CHECK_THAT(mpc::digit, FailsToParse("abc"));
  CHECK_SUCCEED(mpc::digit, "123", '1');
  CHECK_SUCCEED(mpc::alpha, "abc", 'a');
  CHECK_THAT(mpc::alpha, FailsToParse("123"));
  CHECK_THAT(test3, FailsToParse("abc"));
  CHECK_THAT(test3, FailsToParse("123"));
  CHECK_SUCCEED(test3, "a23", "a23"sv);
  CHECK_SUCCEED(test3, "a234", "a23"sv);
  CHECK_SUCCEED(test4, "a", 'a');
  CHECK_SUCCEED(test4, "1", '1');
  CHECK_THAT(test4, FailsToParse("!"));
  CHECK_SUCCEED(test5, "a123", "a123"sv);
  CHECK_THAT(test5, FailsToParse("ab123"));
  // CHECK_SUCCEED(test6, "a123", "a123"sv);
  // CHECK_THAT(test6, FailsToParse("ab123"));
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
  CHECK_THAT(ident, FailsToParse("1"));

  const auto ids = mpc::sep_by1(ident, mpc::char1 % ',');
  CHECK_SUCCEED(ids, "Alice,Bolice,Chris", std::list{"Alice"sv, "Bolice"sv, "Chris"sv});
  CHECK_SUCCEED(ids, "Alice", std::list{"Alice"sv});
  CHECK_SUCCEED(ids, "Alice ", std::list{"Alice"sv});
  CHECK_SUCCEED(ids, "Alice,", std::list{"Alice"sv});
  CHECK_THAT(ids, FailsToParse(","));

  const auto id_list = mpc::between(mpc::char1 % '[', ids, mpc::char1 % ']');
  CHECK_SUCCEED(id_list, "[Alice,Bolice,Chris]", std::list{"Alice"sv, "Bolice"sv, "Chris"sv});
  CHECK_SUCCEED(id_list, "[Alice]", std::list{"Alice"sv});
  CHECK_THAT(id_list, FailsToParse("[Alice ]"));
  CHECK_THAT(id_list, FailsToParse("[Alice,]"));
}

inline const auto spaces = mpc::many % mpc::space;
inline constexpr auto token = //
  mpc::partial([](auto&& parser) { return mpc::discard2nd(MPC_FORWARD(parser), spaces); });
inline constexpr auto char_token = mpc::compose(token, mpc::char1);

inline constexpr auto readint = //
  mpc::partial([](mpc::similar_to<mpc::String> auto&& s) {
    std::string str(s.begin(), s.end());
    std::int64_t num{};
    if (auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), num);
        ec == std::errc{}) {
      return num;
    } else
      throw "conversion from chars to integer failed";
  });
inline constexpr auto readstr = //
  mpc::partial(
    [](mpc::similar_to<mpc::String> auto&& s) { return std::string(s.begin(), s.end()); });
inline constexpr auto numop = //
  mpc::partial([](const char c, auto&& x, auto&& y) {
    switch (c) {
    case '+':
      return MPC_FORWARD(x) + MPC_FORWARD(y);
    case '-':
      return MPC_FORWARD(x) - MPC_FORWARD(y);
    case '*':
      return MPC_FORWARD(x) * MPC_FORWARD(y);
    case '/':
      return MPC_FORWARD(x) / MPC_FORWARD(y);
    default:
      throw "Unexpected operator";
    }
  });
inline constexpr auto make_pair = //
  mpc::partial([](auto&& x, auto&& y) { return std::make_pair(MPC_FORWARD(x), MPC_FORWARD(y)); });

TEST_CASE("parser calc", "[parser][calc]") {
  using namespace mpc::operators::alternatives;
  const auto num = token % (mpc::many1 % mpc::digit);
  const auto ident1 = mpc::alpha or mpc::char1 % '_';
  const auto ident2 = mpc::alnum or mpc::char1 % '_';
  const auto ident = token % mpc::liftA2(mpc::cons, ident1, mpc::many % ident2);

  // expr   = term ("+" term | "-" term)*
  // term   = factor ("*" factor | "/" factor)*
  // factor = "(" expr ")" | number
  const auto factor = mpc::fmap(readint, num);
  const auto termop = mpc::fmap(numop, char_token % '*' or char_token % '/');
  const auto term = mpc::chainl1(factor, termop);
  const auto exprop = mpc::fmap(numop, char_token % '+' or char_token % '-');
  const auto expr = mpc::chainl1(term, exprop);

  CHECK_SUCCEED(expr, "10", 10);
  CHECK_SUCCEED(expr, "1 + 2 + 3 + 4", 10);
  CHECK_SUCCEED(expr, "1 + 2 - 3 + 4", 4);
  // clang-format off
  CHECK_SUCCEED(expr, "1*2*3 + 3*4*5 + 5*6*7", 1*2*3 + 3*4*5 + 5*6*7);
  CHECK_SUCCEED(expr, "1*2*3 + 3*4*5 - 5*6*7", 1*2*3 + 3*4*5 - 5*6*7);
  CHECK_SUCCEED(expr, "1*2/3 + 3/4*5 + 5*6/7", 1*2/3 + 3/4*5 + 5*6/7);
  CHECK_SUCCEED(expr, "1*2/3 + 3/4*5 - 5*6/7", 1*2/3 + 3/4*5 - 5*6/7);
  // clang-format on

  // stmts   = stmt?
  // stmt    = assign ("," assign)*
  // assign  = ident "=" expr
  const auto assign =
    mpc::liftA2(make_pair, mpc::fmap(readstr, ident), mpc::discard1st(char_token % '=', expr));
  const auto stmts = mpc::sep_by(assign, char_token % ',');
  // clang-format off
  using assign_result = std::pair<std::string, std::int64_t>;
  CHECK_SUCCEED(assign,       "num = 1*2*3 + 3*4*5 + 5*6*7",
                assign_result{"num", 1*2*3 + 3*4*5 + 5*6*7});
  CHECK_SUCCEED(assign,       "num = 1*2*3 + 3*4*5 - 5*6*7",
                assign_result{"num", 1*2*3 + 3*4*5 - 5*6*7});
  CHECK_SUCCEED(assign,       "num = 1*2/3 + 3/4*5 + 5*6/7",
                assign_result{"num", 1*2/3 + 3/4*5 + 5*6/7});
  CHECK_SUCCEED(assign,       "num = 1*2/3 + 3/4*5 - 5*6/7",
                assign_result{"num", 1*2/3 + 3/4*5 - 5*6/7});
  using stmts_result = std::list<assign_result>;
  CHECK_SUCCEED(stmts,        "num1 = 1*2*3 ,   num2 = 3*4*5 ,   num3 = 5*6*7",
                stmts_result{{"num1", 1*2*3}, {"num2", 3*4*5}, {"num3", 5*6*7}});
  CHECK_SUCCEED(stmts,        "num1 = 1*2*3 ,   num2 = 3*4*5 ,   num3 = 5*6*7",
                stmts_result{{"num1", 1*2*3}, {"num2", 3*4*5}, {"num3", 5*6*7}});
  CHECK_SUCCEED(stmts,        "num1 = 1*2/3 ,   num2 = 3/4*5 ,   num3 = 5*6/7",
                stmts_result{{"num1", 1*2/3}, {"num2", 3/4*5}, {"num3", 5*6/7}});
  CHECK_SUCCEED(stmts,        "num1 = 1*2/3 ,   num2 = 3/4*5 ,   num3 = 5*6/7",
                stmts_result{{"num1", 1*2/3}, {"num2", 3/4*5}, {"num3", 5*6/7}});
  // clang-format on
}
