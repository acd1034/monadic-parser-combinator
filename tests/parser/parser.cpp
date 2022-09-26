#include <algorithm>
#include <charconv>
#include <sstream> // std::ostringstream
#include <utility> // std::exchange
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <mpc/parser.hpp>
#include <mpc/utility/overloaded.hpp>
#include "../stdfundamental.hpp"
using namespace std::string_view_literals;

template <class T>
concept streamable = requires(std::ostringstream& ss, T t) {
  ss << t;
};

inline constexpr auto to_stream = [](std::ostringstream& ss, const auto& t) {
  if constexpr (streamable<decltype(t)>)
    ss << '"' << t << '"';
  else
    ss << "{?}";
};

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
    ss << "fails to parse ";
    to_stream(ss, sv_);
    return ss.str();
  }
};

inline constexpr auto equal_to =
  mpc::overloaded(std::equal_to<>{}, [](const mpc::String& x, const std::string_view& y) {
    return std::equal(x.begin(), x.end(), y.begin(), y.end());
  });

template <class T>
struct SucceedsInParsing : Catch::Matchers::MatcherGenericBase {
private:
  std::string_view sv_{};
  T expected_{};

public:
  SucceedsInParsing(std::string_view sv, const T& expected)
    : sv_(std::move(sv)), expected_(expected) {}
  SucceedsInParsing(std::string_view sv, T&& expected)
    : sv_(std::move(sv)), expected_(std::move(expected)) {}
  bool match(const auto& parser) const {
    auto result = mpc::eval_StateT % parser % mpc::String(sv_.begin(), sv_.end());
    if (result.index() == 0)
      return false;
    return equal_to(*mpc::snd(result), expected_);
  }
  std::string describe() const override {
    std::ostringstream ss;
    ss << "succeeds in parsing ";
    to_stream(ss, sv_);
    ss << ", which results in ";
    to_stream(ss, expected_);
    return ss.str();
  }
};

template <class T>
struct SucceedsInParsing<std::list<T>> : Catch::Matchers::MatcherGenericBase {
private:
  std::string_view sv_{};
  std::list<T> expected_{};

public:
  SucceedsInParsing(std::string_view sv, const std::list<T>& expected)
    : sv_(std::move(sv)), expected_(expected) {}
  SucceedsInParsing(std::string_view sv, std::list<T>&& expected)
    : sv_(std::move(sv)), expected_(std::move(expected)) {}
  bool match(const auto& parser) const {
    auto result = mpc::eval_StateT % parser % mpc::String(sv_.begin(), sv_.end());
    if (result.index() == 0)
      return false;
    auto x = *mpc::snd(std::move(result));
    return std::equal(x.begin(), x.end(), expected_.begin(), expected_.end(), equal_to);
  }
  std::string describe() const override {
    std::ostringstream ss;
    ss << "succeeds in parsing ";
    to_stream(ss, sv_);
    ss << ", which results in {";
    const char* dlm = "";
    for (const auto& x : expected_) {
      ss << std::exchange(dlm, ", ");
      to_stream(ss, x);
    }
    ss << "}";
    return ss.str();
  }
};

TEST_CASE("parser min", "[parser][min]") {
  CHECK_THAT(mpc::satisfy % mpc::isdigit, SucceedsInParsing("0", '0'));
  CHECK_THAT(mpc::satisfy % mpc::isdigit, FailsToParse("a"));
  CHECK_THAT(mpc::char1 % 'a', SucceedsInParsing("a", 'a'));
  CHECK_THAT(mpc::char1 % 'a', FailsToParse("b"));
  CHECK_THAT(mpc::string % "abc"sv, SucceedsInParsing("abc", "abc"sv));
  CHECK_THAT(mpc::string % "abc"sv, FailsToParse("abd"));
  {
    const auto alpha = mpc::satisfy % mpc::isalpha;
    CHECK_THAT(mpc::many % alpha, SucceedsInParsing("abc0", "abc"sv));
    CHECK_THAT(mpc::many % alpha, SucceedsInParsing("0", ""sv));
  }
  {
    const auto alnum = mpc::satisfy % mpc::isalnum;
    CHECK_THAT(mpc::many1 % alnum, SucceedsInParsing("abc012 ", "abc012"sv));
    CHECK_THAT(mpc::many1 % alnum, FailsToParse(" "));
  }
  CHECK_THAT(mpc::alnum, SucceedsInParsing("a", 'a'));
  CHECK_THAT(mpc::alnum, FailsToParse(","));
  CHECK_THAT(mpc::alpha, SucceedsInParsing("b", 'b'));
  CHECK_THAT(mpc::alpha, FailsToParse("1"));
  CHECK_THAT(mpc::digit, SucceedsInParsing("1", '1'));
  CHECK_THAT(mpc::digit, FailsToParse("b"));
  CHECK_THAT(mpc::any_char, SucceedsInParsing("`", '`'));
  CHECK_THAT(mpc::any_char, FailsToParse(""));
  {
    const auto alpha_digit = mpc::sequence % std::list{mpc::alpha, mpc::digit};
    CHECK_THAT(alpha_digit, SucceedsInParsing("a1", "a1"sv));
    CHECK_THAT(alpha_digit, FailsToParse("1a"));
  }
  {
    const auto alpha_digits = mpc::liftA2(mpc::cons, mpc::alpha, mpc::many % mpc::digit);
    CHECK_THAT(alpha_digits, SucceedsInParsing("a1234", "a1234"sv));
    CHECK_THAT(alpha_digits, SucceedsInParsing("ab1234", "a"sv));
    CHECK_THAT(alpha_digits, FailsToParse("1234"));
  }
  {
    const auto alphas_digits =
      mpc::liftA2(mpc::append, mpc::many % mpc::alpha, mpc::many % mpc::digit);
    CHECK_THAT(alphas_digits, SucceedsInParsing("abc123", "abc123"sv));
    CHECK_THAT(alphas_digits, SucceedsInParsing("123abc", "123"sv));
  }
  {
    const auto alpha_sep_by1_comma = mpc::sep_by1(mpc::alpha, mpc::char1 % ',');
    CHECK_THAT(alpha_sep_by1_comma, SucceedsInParsing("a,b,c", "abc"sv));
    CHECK_THAT(alpha_sep_by1_comma, SucceedsInParsing("ab", "a"sv));
    CHECK_THAT(alpha_sep_by1_comma, FailsToParse(","));
    CHECK_THAT(alpha_sep_by1_comma, FailsToParse(""));
  }
  {
    const auto alpha_sep_by_comma = mpc::sep_by(mpc::alpha, mpc::char1 % ',');
    CHECK_THAT(alpha_sep_by_comma, SucceedsInParsing("a,b,c", "abc"sv));
    CHECK_THAT(alpha_sep_by_comma, SucceedsInParsing("ab", "a"sv));
    CHECK_THAT(alpha_sep_by_comma, SucceedsInParsing("", ""sv));
    CHECK_THAT(alpha_sep_by_comma, SucceedsInParsing(",", ""sv));
  }
  {
    const auto braced_digits =
      mpc::between(mpc::char1 % '{', mpc::many % mpc::digit, mpc::char1 % '}');
    CHECK_THAT(braced_digits, SucceedsInParsing("{123}", "123"sv));
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
  CHECK_THAT(mpc::any_char, SucceedsInParsing("abc", 'a'));
  CHECK_THAT(test1, SucceedsInParsing("abc", "ab"sv));
  CHECK_THAT(test2, SucceedsInParsing("abc", "abc"sv));
  CHECK_THAT(test2, FailsToParse("12"));
  CHECK_THAT(test2, SucceedsInParsing("123", "123"sv));
  CHECK_THAT(mpc::char1 % 'a', SucceedsInParsing("abc", 'a'));
  CHECK_THAT(mpc::char1 % 'a', FailsToParse("123"));
  CHECK_THAT(mpc::digit, FailsToParse("abc"));
  CHECK_THAT(mpc::digit, SucceedsInParsing("123", '1'));
  CHECK_THAT(mpc::alpha, SucceedsInParsing("abc", 'a'));
  CHECK_THAT(mpc::alpha, FailsToParse("123"));
  CHECK_THAT(test3, FailsToParse("abc"));
  CHECK_THAT(test3, FailsToParse("123"));
  CHECK_THAT(test3, SucceedsInParsing("a23", "a23"sv));
  CHECK_THAT(test3, SucceedsInParsing("a234", "a23"sv));
  CHECK_THAT(test4, SucceedsInParsing("a", 'a'));
  CHECK_THAT(test4, SucceedsInParsing("1", '1'));
  CHECK_THAT(test4, FailsToParse("!"));
  CHECK_THAT(test5, SucceedsInParsing("a123", "a123"sv));
  CHECK_THAT(test5, FailsToParse("ab123"));
  // CHECK_THAT(test6, SucceedsInParsing("a123", "a123"sv));
  // CHECK_THAT(test6, FailsToParse("ab123"));
  CHECK_THAT(test7, SucceedsInParsing("abc123", "abc"sv));
  CHECK_THAT(test7, SucceedsInParsing("123abc", ""sv));
  CHECK_THAT(test8, SucceedsInParsing("abc123", "abc123"sv));
  CHECK_THAT(test8, SucceedsInParsing("123abc", "123abc"sv));
  CHECK_THAT(test9, SucceedsInParsing("ab", "ab"sv));
  CHECK_THAT(test9, SucceedsInParsing("ac", "ac"sv));
  CHECK_THAT(test10, SucceedsInParsing("ab", "ab"sv));
  CHECK_THAT(test10, SucceedsInParsing("ac", "ac"sv));
  CHECK_THAT(test11, SucceedsInParsing("ab", "ab"sv));
  CHECK_THAT(test11, SucceedsInParsing("ac", "ac"sv));
  CHECK_THAT(test12, SucceedsInParsing("ab", "ab"sv));
  CHECK_THAT(test12, SucceedsInParsing("ac", "ac"sv));
  CHECK_THAT(test13, SucceedsInParsing("ab", "ab"sv));
  CHECK_THAT(test13, SucceedsInParsing("ac", "ac"sv));
}

TEST_CASE("parser ident", "[parser][ident]") {
  // Reference: https://deepsource.io/blog/monadic-parser-combinators/

  using namespace mpc::operators::alternatives;
  const auto ident1 = mpc::alpha or mpc::char1 % '_';
  const auto ident2 = mpc::alnum or mpc::char1 % '_';
  const auto ident = mpc::liftA2(mpc::cons, ident1, mpc::many % ident2);
  CHECK_THAT(ident, SucceedsInParsing("__cplu5plu5", "__cplu5plu5"sv));
  CHECK_THAT(ident, FailsToParse("1"));

  const auto ids = mpc::sep_by1(ident, mpc::char1 % ',');
  CHECK_THAT(ids,
             SucceedsInParsing("Alice,Bolice,Chris", std::list{"Alice"sv, "Bolice"sv, "Chris"sv}));
  CHECK_THAT(ids, SucceedsInParsing("Alice", std::list{"Alice"sv}));
  CHECK_THAT(ids, SucceedsInParsing("Alice ", std::list{"Alice"sv}));
  CHECK_THAT(ids, SucceedsInParsing("Alice,", std::list{"Alice"sv}));
  CHECK_THAT(ids, FailsToParse(","));

  const auto id_list = mpc::between(mpc::char1 % '[', ids, mpc::char1 % ']');
  CHECK_THAT(id_list, SucceedsInParsing("[Alice,Bolice,Chris]",
                                        std::list{"Alice"sv, "Bolice"sv, "Chris"sv}));
  CHECK_THAT(id_list, SucceedsInParsing("[Alice]", std::list{"Alice"sv}));
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

  CHECK_THAT(expr, SucceedsInParsing("10", 10));
  CHECK_THAT(expr, SucceedsInParsing("1 + 2 + 3 + 4", 10));
  CHECK_THAT(expr, SucceedsInParsing("1 + 2 - 3 + 4", 4));
  // clang-format off
  CHECK_THAT(expr, SucceedsInParsing("1*2*3 + 3*4*5 + 5*6*7", 1*2*3 + 3*4*5 + 5*6*7));
  CHECK_THAT(expr, SucceedsInParsing("1*2*3 + 3*4*5 - 5*6*7", 1*2*3 + 3*4*5 - 5*6*7));
  CHECK_THAT(expr, SucceedsInParsing("1*2/3 + 3/4*5 + 5*6/7", 1*2/3 + 3/4*5 + 5*6/7));
  CHECK_THAT(expr, SucceedsInParsing("1*2/3 + 3/4*5 - 5*6/7", 1*2/3 + 3/4*5 - 5*6/7));
  // clang-format on

  // stmts   = stmt?
  // stmt    = assign ("," assign)*
  // assign  = ident "=" expr
  const auto assign =
    mpc::liftA2(make_pair, mpc::fmap(readstr, ident), mpc::discard1st(char_token % '=', expr));
  const auto stmts = mpc::sep_by(assign, char_token % ',');
  // clang-format off
  using assign_result = std::pair<std::string, std::int64_t>;
  CHECK_THAT(assign, SucceedsInParsing("num = 1*2*3 + 3*4*5 + 5*6*7", assign_result{"num", 1*2*3 + 3*4*5 + 5*6*7}));
  CHECK_THAT(assign, SucceedsInParsing("num = 1*2*3 + 3*4*5 - 5*6*7", assign_result{"num", 1*2*3 + 3*4*5 - 5*6*7}));
  CHECK_THAT(assign, SucceedsInParsing("num = 1*2/3 + 3/4*5 + 5*6/7", assign_result{"num", 1*2/3 + 3/4*5 + 5*6/7}));
  CHECK_THAT(assign, SucceedsInParsing("num = 1*2/3 + 3/4*5 - 5*6/7", assign_result{"num", 1*2/3 + 3/4*5 - 5*6/7}));
  using stmts_result = std::list<assign_result>;
  CHECK_THAT(stmts, SucceedsInParsing("num1 = 1*2*3 , num2 = 3*4*5 , num3 = 5*6*7", stmts_result{{"num1", 1*2*3}, {"num2", 3*4*5}, {"num3", 5*6*7}}));
  CHECK_THAT(stmts, SucceedsInParsing("num1 = 1*2*3 , num2 = 3*4*5 , num3 = 5*6*7", stmts_result{{"num1", 1*2*3}, {"num2", 3*4*5}, {"num3", 5*6*7}}));
  CHECK_THAT(stmts, SucceedsInParsing("num1 = 1*2/3 , num2 = 3/4*5 , num3 = 5*6/7", stmts_result{{"num1", 1*2/3}, {"num2", 3/4*5}, {"num3", 5*6/7}}));
  CHECK_THAT(stmts, SucceedsInParsing("num1 = 1*2/3 , num2 = 3/4*5 , num3 = 5*6/7", stmts_result{{"num1", 1*2/3}, {"num2", 3/4*5}, {"num3", 5*6/7}}));
  // clang-format on
}
