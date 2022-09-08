#include <iostream>
#include <string>
#include <string_view>
#include <mpc/control.hpp>
#include <mpc/data.hpp>
#define MPC_FORWARD(x) std::forward<decltype(x)>(x)
using namespace std::literals;
using mpc::operators::alternatives::operator||;
template <class Op, class... Args>
using applicable = mpc::partially_applicable<Op, Args...>;

template <class CharT, class Traits, class T>
auto& operator<<(std::basic_ostream<CharT, Traits>& os, const std::list<T>& l) {
  for (const auto& x : l) { os << x; }
  return os;
}

template <class T>
[[deprecated]] constexpr void type_of() {}

template <class T>
[[deprecated]] constexpr void type_of(T&&) {}

// Test implementation of parser combinator
// Reference: https://qiita.com/7shi/items/b8c741e78a96ea2c10fe#%E3%83%A2%E3%83%8A%E3%83%89%E5%A4%89%E6%8F%9B%E5%AD%90%E3%81%A7%E5%90%88%E6%88%90

using String = std::list<char>;
using Result = std::pair<char, String>;
using Parser = mpc::StateT<String, mpc::either<std::string, Result>>;

Result decomp(String&& str) {
  auto c = str.front();
  str.pop_front();
  return {c, std::move(str)};
}

void parseTest(const std::size_t n, const auto& parser, std::string_view sv) {
  const auto result = mpc::eval_StateT % parser % std::list(sv.begin(), sv.end());
  if (result.index() == 0) {
    std::cout << n << " [" << sv << "] " << *mpc::fst(result) << std::endl;
  } else {
    std::cout << n << ' ' << *mpc::snd(result) << std::endl;
  }
};

template <class T>
struct mpc::alternative_traits<mpc::either<std::string, T>> {
  static constexpr auto combine = [](auto&& m1, auto&& m2) //
    -> mpc::either<std::string, T> {
    if (m1.index() == 0) {
      if (m2.index() == 0) {
        return mpc::make_left(*mpc::fst(MPC_FORWARD(m1)) + " AND " + *mpc::fst(MPC_FORWARD(m2)));
      } else {
        return MPC_FORWARD(m2);
      }
    } else {
      return MPC_FORWARD(m1);
    }
  };
};

inline constexpr auto satisfy = //
  applicable([](std::predicate<char> auto pred) {
    return mpc::make_StateT<String>(applicable(
      [](auto pred, String str) -> mpc::either<std::string, Result> {
        if (str.empty()) {
          return mpc::make_left("unexpected end of input"s);
        } else if (auto [x, xs] = decomp(std::move(str)); not std::invoke(pred, x)) {
          return mpc::make_left("unexpected "s + mpc::quoted(x));
        } else {
          return mpc::make_right(std::make_pair(x, std::move(xs)));
        }
      },
      std::move(pred)));
  });

inline constexpr auto try1 = applicable([](/* Parser */ auto&& parser) {
  return mpc::make_StateT<String>(applicable(
    [](
      /* Parser */ auto&& parser, /* String */ auto&& str) {
      return mpc::run_StateT % MPC_FORWARD(parser) % MPC_FORWARD(str);
    },
    MPC_FORWARD(parser)));
});

inline constexpr auto left =
  mpc::compose % mpc::lift<Parser> //
  % applicable([](/* std::string */ auto&& str) -> mpc::eval_StateT_t<Parser> {
      return mpc::make_left(MPC_FORWARD(str));
    });

inline constexpr auto char1 = applicable([](char c) {
  auto c2 = c;
  return satisfy % (mpc::equal_to % std::move(c))
         or left % ("expecting char "s + mpc::quoted(std::move(c2)));
});

inline constexpr auto string = applicable([](std::string_view sv) {
  std::list l(sv.begin(), sv.end());
  return mpc::sequence % mpc::fmap(applicable([](char c) { return char1 % std::move(c); }), std::move(l));
});

// https://hackage.haskell.org/package/base-4.17.0.0/docs/Control-Applicative.html#v:many
// https://hackage.haskell.org/package/base-4.17.0.0/docs/src/GHC.Base.html#many
// some v = (:) <$> v <*> many v
// many v = some v <|> pure []

int main() {
  const auto anyChar = satisfy % (mpc::constant % true);
  const auto digit = satisfy % mpc::isdigit or left % "expecting digit"s;
  const auto alpha = satisfy % mpc::isalpha or left % "expecting alpha"s;
  // auto test = mpc::foldr(mpc::liftA2 % mpc::cons, mpc::returns<decltype(anyChar)> % std::list<char>{}, std::list{anyChar, anyChar});
  const auto test1 = mpc::sequence % std::list{anyChar, anyChar};
  const auto test2 = mpc::sequence % std::list{anyChar, anyChar, anyChar};
  const auto test3 = mpc::sequence % std::list{alpha, digit, digit};
  const auto test4 = alpha or digit;
  const auto test5 = mpc::sequence % std::list{alpha, digit, digit, digit};
  // test6 = sequence $ letter : replicate 3 digit
  // const auto test7 = many % alpha;
  // const auto test8 = many % test4;
  const auto test9 = mpc::sequence % std::list{char1 % 'a', char1 % 'b'}
                     or mpc::sequence % std::list{char1 % 'a', char1 % 'c'};
  const auto test10 = try1(mpc::sequence % std::list{char1 % 'a', char1 % 'b'})
                      or mpc::sequence % std::list{char1 % 'a', char1 % 'c'};
  const auto test11 = string % "ab" or string % "ac";
  const auto test12 = try1(string % "ab") or string % "ac";
  const auto test13 = mpc::sequence % std::list{char1 % 'a', char1 % 'b' or char1 % 'c'};

  parseTest(1, anyChar, ""); // NG
  parseTest(2, anyChar, "abc");
  parseTest(3, test1, "abc");
  parseTest(4, test2, "abc");
  parseTest(5, test2, "12"); // NG
  parseTest(6, test2, "123");
  parseTest(7, char1 % 'a', "abc");
  parseTest(8, char1 % 'a', "123"); // NG
  parseTest(9, digit, "abc");       // NG
  parseTest(10, digit, "123");
  parseTest(11, alpha, "abc");
  parseTest(12, alpha, "123"); // NG
  parseTest(13, test3, "abc"); // NG
  parseTest(14, test3, "123"); // NG
  parseTest(15, test3, "a23");
  parseTest(16, test3, "a234");
  parseTest(17, test4, "a");
  parseTest(18, test4, "1");
  parseTest(19, test4, "!"); // NG
  parseTest(20, test5, "a123");
  parseTest(21, test5, "ab123"); // NG
  // parseTest(22, test6, "a123");
  // parseTest(23, test6, "ab123"); // NG
  // parseTest(24, test7, "abc123");
  // parseTest(25, test7, "123abc");
  // parseTest(26, test8, "abc123");
  // parseTest(27, test8, "123abc");
  parseTest(28, test9, "ab");
  parseTest(29, test9, "ac"); // NG but succeed
  parseTest(30, test10, "ab");
  parseTest(31, test10, "ac");
  parseTest(32, test11, "ab");
  parseTest(33, test11, "ac"); // NG but succeed
  parseTest(34, test12, "ab");
  parseTest(35, test12, "ac");
  parseTest(36, test13, "ab");
  parseTest(37, test13, "ac");
}
