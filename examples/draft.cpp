#include <iostream>
#include <string>
#include <string_view>
#include <mpc/control.hpp>
#include <mpc/data.hpp>
using namespace std::literals;
using mpc::operators::alternatives::operator||;
template <class Op, class... Args>
using applicable = mpc::partially_applicable<Op, Args...>;

template <class T>
[[deprecated]] constexpr void type_of() {}

template <class T>
[[deprecated]] constexpr void type_of(T&&) {}

// Test implementation of parser combinator
// Reference: https://qiita.com/7shi/items/b8c741e78a96ea2c10fe#%E3%83%A2%E3%83%8A%E3%83%89%E5%A4%89%E6%8F%9B%E5%AD%90%E3%81%A7%E5%90%88%E6%88%90

template <class V>
using Monad = mpc::either<std::string, V>;

using ST = mpc::StateT<std::string_view, Monad<std::pair<char, std::string_view>>>;

template <mpc::is_StateT ST2>
using StateT_value_in_monad_t =
  decltype(mpc::fmap(mpc::fst, std::declval<mpc::StateT_monad_t<ST2>>()));

constexpr std::pair<char, std::string_view> //
decomp(std::string_view sv) {
  return {sv.front(), sv.substr(1)};
}

void parseTest(const std::size_t n, const auto& st, std::string_view sv) {
  const auto result = mpc::eval_StateT % st % sv;
  if (result.index() == 0) {
    std::cout << n << " [" << sv << "] " << *std::get<0>(result) << std::endl;
  } else {
    std::cout << n << ' ' << *std::get<1>(result) << std::endl;
  }
};

template <class T>
struct mpc::alternative_traits<mpc::either<std::string, T>> {
  static constexpr auto combine = [](const auto& m1, const auto& m2) //
    -> mpc::either<std::string, T> {
    if (m1.index() == 0) {
      if (m2.index() == 0) {
        return mpc::make_left(*std::get<0>(m1) + " AND " + *std::get<0>(m2));
      } else {
        return m2;
      }
    } else {
      return m1;
    }
  };
};

inline constexpr auto satisfy = //
  applicable([](std::predicate<char> auto pred) {
    return mpc::make_StateT<std::string_view>(applicable(
      [](auto pred, std::string_view sv) -> Monad<std::pair<char, std::string_view>> {
        if (std::ranges::empty(sv)) {
          return mpc::make_left("unexpected end of input"s);
        } else if (auto [x, xs] = decomp(sv); not std::invoke(pred, x)) {
          return mpc::make_left("unexpected "s + mpc::quoted(x));
        } else {
          return mpc::make_right(std::make_pair(x, xs));
        }
      },
      std::move(pred)));
  });

inline constexpr auto left =
  mpc::compose % mpc::lift<ST> % applicable([](const auto& str) -> StateT_value_in_monad_t<ST> {
    return mpc::make_left(str);
  });

inline constexpr auto char1 = applicable([](char c) {
  return satisfy % (mpc::equal_to % c) or left % ("expecting char "s + mpc::quoted(c));
});

int main() {
  const auto anyChar = satisfy % (mpc::constant % true);
  const auto digit = satisfy % mpc::isdigit or left % "expecting digit"s;
  const auto alpha = satisfy % mpc::isalpha or left % "expecting alpha"s;
  // FIXME
  // const auto test1 = mpc::sequence % std::list{anyChar, anyChar};
  // const auto test2 = mpc::sequence % std::list{anyChar, anyChar, anyChar};
  // const auto test3 = mpc::sequence % std::list{alpha, digit, digit};
  const auto test4 = digit or alpha;

  parseTest(1, anyChar, ""); // NG
  parseTest(2, anyChar, "abc");
  // parseTest(3, test1, "abc");
  // parseTest(4, test2, "abc");
  // parseTest(5, test2, "12"); // NG
  // parseTest(6, test2, "123");
  parseTest(7, char1 % 'a', "abc");
  parseTest(8, char1 % 'a', "123"); // NG
  parseTest(9, digit, "abc");       // NG
  parseTest(10, digit, "123");
  parseTest(11, alpha, "abc");
  parseTest(12, alpha, "123"); // NG
  // parseTest(13, test3, "abc"); // NG
  // parseTest(14, test3, "123"); // NG
  // parseTest(15, test3, "a23");
  // parseTest(16, test3, "a234");
  parseTest(17, test4, "a");
  parseTest(18, test4, "1");
  parseTest(19, test4, "!"); // NG
}
