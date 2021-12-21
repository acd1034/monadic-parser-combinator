#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <mpc/control.hpp>
#include <mpc/data.hpp>
#include <mpc/prelude.hpp>
#include "../../stdfundamental.hpp"

TEST_CASE("trans", "[trans][state]") {
  using ST = mpc::StateT<const int&, mpc::Identity<std::pair<int, int>>>;
  static_assert(mpc::functor<ST>);
  static_assert(mpc::applicative<ST>);
  static_assert(mpc::monad<ST>);
  static_assert(mpc::monad_trans<ST>);
  static_assert(mpc::monad_state<ST>);
  {
    const auto gets = *mpc::gets<ST>;
    const auto [a, s] = mpc::run_State % gets % 5;
    CHECK(a == 5);
    CHECK(s == 5);
  }
  {
    const auto put = mpc::put<ST> % 3;
    const auto [a, s] = mpc::run_State % put % 5;
    CHECK(s == 3);
  }
  {
    const auto modify = mpc::modify<ST> % (mpc::plus % 1);
    const auto [a, s] = mpc::run_State % modify % 5;
    CHECK(s == 6);
  }
  {
    const auto gets = *mpc::gets<ST>;
    const auto [a1, s1] = mpc::run_State % gets % 5;
    const auto modify = mpc::modify<ST> % (mpc::plus % 1);
    const auto [a2, s2] = mpc::run_State % modify % s1;
    CHECK(a1 == 5);
    CHECK(s2 == 6);
  }
  {
    // NOTE: fn1 を partially_applicable でなくすことはできない(なんで?)
    // constexpr auto fn1 = [](int n, auto&&) { return n; };
    constexpr auto fn1 = mpc::partially_applicable([](int n, auto&&) { return n; });
    const auto gets = *mpc::gets<ST>;
    const auto fn2 = mpc::fmap(fn1, gets);
    const auto modify = mpc::modify<ST> % (mpc::plus % 1);
    const auto tick = mpc::seq_apply(fn2, modify);
    const auto [a, s] = mpc::run_State % tick % 5;
    CHECK(a == 5);
    CHECK(s == 6);
  }
  {
    // Reference:
    // https://qiita.com/sand/items/802b8c4a8ae19f04102b#3-statet%E3%83%A2%E3%83%8A%E3%83%89%E3%81%AE%E4%BD%BF%E7%94%A8%E4%BE%8B
    // clang-format off
    constexpr auto fn1 = [](int n, auto&&) { return n; };
    const auto tick = mpc::liftA2(
      mpc::partially_applicable(fn1),
      *mpc::gets<ST>,
      mpc::modify<ST> % (mpc::plus % 1)
    );
    {
      const auto [a, s] = mpc::run_State % tick % 5;
      CHECK(a == 5);
      CHECK(s == 6);
    }

    constexpr auto fn2 = [](int n1, int n2, int n3) { return n1 + n2 + n3; };
    const auto threeTicks = mpc::liftA<3>(
      mpc::partially_applicable(fn2),
      tick,
      tick,
      tick
    );
    {
      const auto [a, s] = mpc::run_State % threeTicks % 5;
      CHECK(a == 18);
      CHECK(s == 8);
    }
    // clang-format on
  }
}

TEST_CASE("trans StateT alternative", "[trans][statet][alternative]") {
  using ST1 = mpc::StateT<const int&, mpc::Identity<std::pair<int, int>>>;
  using ST2 = mpc::StateT<const int&, mpc::maybe<std::pair<int, int>>>;
  static_assert(not mpc::alternative<ST1>);
  static_assert(mpc::alternative<ST2>);
}
