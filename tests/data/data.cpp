#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <mpc/control.hpp>
#include <mpc/data.hpp>
#include "../stdfundamental.hpp"

TEST_CASE("data Identity", "[data]") {
  static_assert(mpc::functor<mpc::Identity<int>>);
  static_assert(mpc::applicative<mpc::Identity<int>>);
  static_assert(mpc::monad<mpc::Identity<int>>);
}

TEST_CASE("data list", "[data]") {
  static_assert(mpc::functor<std::list<int>>);
  static_assert(mpc::applicative<std::list<int>>);
  // static_assert(mpc::monad<std::list<int>>);
  {
    std::list l{1, 2, 3, 4, 5};
    CHECK(mpc::foldr(mpc::plus, 0, l) == 15);
  }
  {
    using maybe1 = mpc::maybe<int>;
    std::list m{
      maybe1{mpc::make_just(1)}, maybe1{mpc::make_just(2)}, maybe1{mpc::make_just(3)},
      maybe1{mpc::make_just(4)}, maybe1{mpc::make_just(5)},
    };
    std::list l{1, 2, 3, 4, 5};
    CHECK(mpc::sequence(m) == mpc::maybe<std::list<int>>{mpc::make_just(l)});
  }
}

TEST_CASE("data maybe", "[data]") {
  { // maybe
    using maybe1 = mpc::maybe<double>;
    {
      const maybe1 e{mpc::nothing};
      CHECK(e.index() == 0);
    }
    {
      const maybe1 e{mpc::make_just(2.0)};
      CHECK(e.index() == 1);
    }
    { // functor
      static_assert(mpc::functor<maybe1>);
      const maybe1 e1{mpc::make_just(2.0)};
      const maybe1 e2{mpc::make_just(4.0)};
      const auto f = [](const auto& x) { return x * 2; };
      CHECK(mpc::fmap(f, e1) == e2);
    }
    { // applicative
      static_assert(mpc::applicative<maybe1>);
      const maybe1 e1{mpc::make_just(2.0)};
      const maybe1 e2{mpc::make_just(4.0)};
      const mpc::maybe<std::function<double(double)>> f1{
        mpc::make_just([](double x) { return x * 2; })};
      constexpr auto add =
        mpc::partial([](const auto& x, const auto& y) { return x + y; });
      constexpr auto add3 = mpc::partial(
        [](const auto& x, const auto& y, const auto& z) { return x + y + z; });

      CHECK(mpc::seq_apply(f1, e1) == e2);
      CHECK(mpc::liftA2(add, e1, e1) == e2);
      CHECK(mpc::liftA<3>(add3, e1, e1, e1) == maybe1{mpc::make_just(6.0)});
      CHECK(mpc::liftA<3>(add3, maybe1{mpc::make_just(1.0)}, maybe1{mpc::make_just(2.0)},
                          maybe1{mpc::make_just(3.0)})
            == maybe1{mpc::make_just(6.0)});
    }
    { // monad
      static_assert(mpc::monad<maybe1>);
      const maybe1 e1{mpc::make_just(2.0)};
      const maybe1 e2{mpc::nothing};
      const auto f = [](const auto& x) { return maybe1{mpc::make_just(x + 1)}; };

      CHECK(mpc::bind(e1, f) == maybe1{mpc::make_just(3.0)});
      CHECK(mpc::bind(mpc::bind(e1, f), f) == maybe1{mpc::make_just(4.0)});
      CHECK(mpc::bind(e2, f) == e2);
      CHECK(mpc::bind(mpc::bind(e2, f), f) == e2);

      CHECK(mpc::infixl(e1, mpc::bind, f) == maybe1{mpc::make_just(3.0)});
      CHECK(mpc::infixl(e1, mpc::bind, f, mpc::bind, f) == maybe1{mpc::make_just(4.0)});
      CHECK(mpc::infixl(e2, mpc::bind, f) == e2);
      CHECK(mpc::infixl(e2, mpc::bind, f, mpc::bind, f) == e2);
    }
    { // alternative
      static_assert(mpc::alternative<maybe1>);
      const maybe1 e1{mpc::make_just(2.0)};
      const maybe1 e2{mpc::make_just(4.0)};
      const maybe1 e3{mpc::nothing};

      CHECK(*mpc::empty<maybe1> == maybe1{mpc::nothing});
      CHECK(mpc::combine(e1, e2) == e1);
      CHECK(mpc::combine(e3, e2) == e2);

      using mpc::operators::alternatives::operator||;
      CHECK((e1 or e2) == e1);
      CHECK((e3 or e2) == e2);
    }
  }
}

TEST_CASE("data either", "[data]") {
  { // either
    using either1 = mpc::either<int, double>;
    {
      const either1 e{mpc::make_left(1)};
      CHECK(e.index() == 0);
    }
    {
      const either1 e{mpc::make_right(2.0)};
      CHECK(e.index() == 1);
    }
    { // functor
      static_assert(mpc::functor<either1>);
      const either1 e1{mpc::make_right(2.0)};
      const either1 e2{mpc::make_right(4.0)};
      const auto f = [](const auto& x) { return x * 2; };

      const mpc::detail::fmap_op fmap_op;
      CHECK(fmap_op(f, e1) == e2);
      CHECK(mpc::fmap(f, e1) == e2);
    }
    { // applicative
      static_assert(mpc::applicative<either1>);
      const either1 e1{mpc::make_right(2.0)};
      const either1 e2{mpc::make_right(4.0)};
      const mpc::either<int, std::function<double(double)>> f1{
        mpc::make_right([](double x) { return x * 2; })};

      CHECK(mpc::seq_apply(f1, e1) == e2);

      constexpr auto add =
        mpc::partial([](const auto& x, const auto& y) { return x + y; });
      const mpc::applicatives::detail::liftA2_op liftA2_op;
      CHECK(liftA2_op(add, e1, e1) == e2);
      CHECK(mpc::liftA2(add, e1, e1) == e2);
    }
    { // monad
      static_assert(mpc::monad<either1>);
      const either1 e1{mpc::make_right(2.0)};
      const either1 e2{mpc::make_left(0)};
      const auto f = [](const auto& x) { return either1{mpc::make_right(x + 1)}; };

      CHECK(mpc::bind(e1, f) == either1{mpc::make_right(3.0)});
      CHECK(mpc::bind(mpc::bind(e1, f), f) == either1{mpc::make_right(4.0)});
      CHECK(mpc::bind(e2, f) == e2);
      CHECK(mpc::bind(mpc::bind(e2, f), f) == e2);

      const auto bind = mpc::bind;
      CHECK(mpc::infixl(e1, bind, f) == either1{mpc::make_right(3.0)});
      CHECK(mpc::infixl(e1, bind, f, bind, f) == either1{mpc::make_right(4.0)});
      CHECK(mpc::infixl(e2, bind, f) == e2);
      CHECK(mpc::infixl(e2, bind, f, bind, f) == e2);
    }
  }
}
