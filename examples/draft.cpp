#include <iomanip>
#include <iostream>
#include <mpc/control.hpp>
#include <mpc/data.hpp>
using namespace std::literals;

template <class T>
[[deprecated]] constexpr void type_of() {}

template <class T>
[[deprecated]] constexpr void type_of(T&&) {}

// Reference:
// https://qiita.com/7shi/items/b8c741e78a96ea2c10fe#%E3%83%A2%E3%83%8A%E3%83%89%E5%A4%89%E6%8F%9B%E5%AD%90%E3%81%A7%E5%90%88%E6%88%90

using mpc::operators::alternatives::operator||;
using SV = std::string_view;
template <class V>
using Monad = mpc::either<std::string, V>;
using ST = mpc::StateT<SV, Monad<std::pair<char, SV>>>;
template <mpc::is_StateT ST2>
using StateT_value_in_monad_t =
  decltype(mpc::fmap(mpc::fst, std::declval<mpc::StateT_monad_t<ST2>>()));

template <mpc::functor F, class V>
using inject_value_in_functor_t =
  decltype(mpc::replace2nd(std::declval<V>(), std::declval<F>()));

template <class charT, class traits>
inline constexpr auto decomp(std::basic_string_view<charT, traits> sv) {
  return std::make_pair(sv.front(), sv.substr(1));
}

template <class T>
struct mpc::alternative_traits<mpc::either<std::string, T>> {
  static constexpr auto combine = [](const auto& m1,
                                     const auto& m2) -> mpc::either<std::string, T> {
    if (m1.index() == 0) {
      if (m2.index() == 0) {
        return mpc::make_left(*std::get<0>(m1) + ' ' + *std::get<0>(m2));
      } else {
        return m2;
      }
    } else {
      return m1;
    }
  };
};

inline constexpr auto parseTest = [](const auto& st, SV sv) -> void {
  const auto result = mpc::eval_StateT % st % sv;
  if (result.index() == 0) {
    std::cout << mpc::quoted(sv) << ' ' << *std::get<0>(result) << std::endl;
  } else {
    std::cout << *std::get<1>(result) << std::endl;
  }
};

inline constexpr auto satisfy =
  mpc::partially_applicable([]<class Pred>(Pred && pred) requires std::predicate<Pred, char> {
    return mpc::make_StateT<SV>(mpc::partially_applicable(
      [](auto pred, SV sv) -> Monad<std::pair<char, SV>> {
        if (std::ranges::empty(sv)) {
          return mpc::make_left("unexpected end of input"s);
        } else if (auto [x, xs] = decomp(sv); not std::invoke(pred, x)) {
          return mpc::make_left("unexpected "s + mpc::quoted(x) + ",");
        } else {
          return mpc::make_right(std::make_pair(x, xs));
        }
      },
      std::forward<Pred>(pred)));
  });

inline constexpr auto left =
  mpc::compose
  % mpc::lift<ST> % mpc::partially_applicable([](const auto& str) -> StateT_value_in_monad_t<ST> {
      return mpc::make_left(str);
    });

inline constexpr auto char1 = mpc::partially_applicable([](char c) {
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

  parseTest(anyChar, "");
  parseTest(anyChar, "abc");
  // parseTest(test1, "abc");
  // parseTest(test2, "abc");
  // parseTest(test2, "12"); // NG
  // parseTest(test2, "123");
  parseTest(char1 % 'a', "abc");
  parseTest(char1 % 'a', "123"); // NG
  parseTest(digit, "abc");       // NG
  parseTest(digit, "123");
  parseTest(alpha, "abc");
  parseTest(alpha, "123"); // NG
  // parseTest(test3, "abc"); // NG
  // parseTest(test3, "123"); // NG
  // parseTest(test3, "a23");
  // parseTest(test3, "a234");
  parseTest(test4, "a");
  parseTest(test4, "1");
  parseTest(test4, "!"); // NG
}
