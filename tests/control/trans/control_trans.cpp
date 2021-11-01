#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <mpc/control/trans.hpp>
#include <mpc/data.hpp>
#include <mpc/functional/infix.hpp>
#include <mpc/functional/operations.hpp>
#include "../../stdfundamental.hpp"

template <class S, class M>
using deduce_StateT = mpc::StateT<std::function<M(S)>, S>;

TEST_CASE("trans", "[trans][state]") {
  using ST = deduce_StateT<int, mpc::Identity<int>>;
  static_assert(mpc::functor<ST>);
  static_assert(mpc::applicative<ST>);
  static_assert(mpc::monad<ST>);
  static_assert(mpc::monad_trans<ST>);
  {
    // Reference:
    // https://qiita.com/sand/items/802b8c4a8ae19f04102b#3-statet%E3%83%A2%E3%83%8A%E3%83%89%E3%81%AE%E4%BD%BF%E7%94%A8%E4%BE%8B
    using Fn1 = decltype([](int n, auto&&) { return n; });
    using Fn2 = decltype([](int n1, int n2, int n3) { return n1 + n2 + n3; });
    // clang-format off
    const auto tick = mpc::liftA2<ST>(
      mpc::perfect_forwarded_t<Fn1>{},
      mpc::get1<ST>,
      mpc::modify<ST> % (mpc::plus % 1)
    );
    const auto threeTicks = mpc::liftA3<ST>(
      mpc::perfect_forwarded_t<Fn2>{},
      tick,
      tick,
      tick
    );
    // clang-format on
    const auto [a, s] = mpc::run_State % threeTicks % 5;
    CHECK(a == 18);
    CHECK(s == 8);
  }
}

TEST_CASE("trans StateT alternative", "[trans][statet][alternative]") {
  static_assert(mpc::alternative<deduce_StateT<int, mpc::Maybe<int>>>);
  static_assert(not mpc::alternative<deduce_StateT<int, mpc::Identity<int>>>);
}
