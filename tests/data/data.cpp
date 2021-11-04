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
  std::list l{1, 2, 3, 4, 5};
  CHECK(mpc::foldr(mpc::plus, 0, l) == 15);
  {
    using Maybe1 = mpc::Maybe<int>;
    std::list m{Maybe1{mpc::make_just(1)}, Maybe1{mpc::make_just(2)}, Maybe1{mpc::make_just(3)}, Maybe1{mpc::make_just(4)}, Maybe1{mpc::make_just(5)}};
    type_of(mpc::sequence(m));
    CHECK(mpc::sequence(m) == mpc::Maybe<std::list<int>>{mpc::make_just(l)});
  }
}

TEST_CASE("data Maybe", "[data]") {
  { // Maybe
    using Maybe1 = mpc::Maybe<double>;
    {
      const Maybe1 e{mpc::nothing};
      CHECK(e.index() == 0);
    }
    {
      const Maybe1 e{mpc::make_just(2.0)};
      CHECK(e.index() == 1);
    }
    { // functor
      static_assert(mpc::functor<Maybe1>);
      const Maybe1 e1{mpc::make_just(2.0)};
      const Maybe1 e2{mpc::make_just(4.0)};
      const auto f = [](const auto& x) { return x * 2; };

      const mpc::detail::fmap_op<Maybe1> fmap_op;
      CHECK(fmap_op(f, e1) == e2);
      CHECK(mpc::fmap<Maybe1>(f, e1) == e2);
    }
    { // applicative
      static_assert(mpc::applicative<Maybe1>);
      const Maybe1 e1{mpc::make_just(2.0)};
      const Maybe1 e2{mpc::make_just(4.0)};
      const mpc::Maybe<std::function<double(double)>> f1{
        mpc::make_just([](double x) { return x * 2; })};

      CHECK(mpc::seq_apply<Maybe1>(f1, e1) == e2);

      const auto add =
        mpc::perfect_forwarded_t<decltype([](const auto& x, const auto& y) { return x + y; })>{};
      const auto add3 = mpc::perfect_forwarded_t<decltype(
        [](const auto& x, const auto& y, const auto& z) { return x + y + z; })>{};
      const mpc::applicatives::detail::liftA2_op<Maybe1> liftA2_op;
      CHECK(liftA2_op(add, e1, e1) == e2);
      CHECK(mpc::liftA2<Maybe1>(add, e1, e1) == e2);

      CHECK(mpc::liftA3<Maybe1>(add3, e1, e1, e1) == Maybe1{mpc::make_just(6.0)});
      CHECK(mpc::liftA3<Maybe1>(add3, Maybe1{mpc::make_just(1.0)}, Maybe1{mpc::make_just(2.0)},
                                Maybe1{mpc::make_just(3.0)})
            == Maybe1{mpc::make_just(6.0)});
    }
    { // monad
      static_assert(mpc::monad<Maybe1>);
      const Maybe1 e1{mpc::make_just(2.0)};
      const Maybe1 e2{mpc::nothing};
      const auto f = [](const auto& x) { return Maybe1{mpc::make_just(x + 1)}; };

      CHECK(mpc::bind<Maybe1>(e1, f) == Maybe1{mpc::make_just(3.0)});
      CHECK(mpc::bind<Maybe1>(mpc::bind<Maybe1>(e1, f), f) == Maybe1{mpc::make_just(4.0)});
      CHECK(mpc::bind<Maybe1>(e2, f) == e2);
      CHECK(mpc::bind<Maybe1>(mpc::bind<Maybe1>(e2, f), f) == e2);

      const auto bind = mpc::bind<Maybe1>;
      CHECK(mpc::infixl(e1, bind, f) == Maybe1{mpc::make_just(3.0)});
      CHECK(mpc::infixl(e1, bind, f, bind, f) == Maybe1{mpc::make_just(4.0)});
      CHECK(mpc::infixl(e2, bind, f) == e2);
      CHECK(mpc::infixl(e2, bind, f, bind, f) == e2);
    }
    { // alternative
      static_assert(mpc::alternative<Maybe1>);
      const Maybe1 e1{mpc::make_just(2.0)};
      const Maybe1 e2{mpc::make_just(4.0)};
      const Maybe1 e3{mpc::nothing};

      CHECK(mpc::combine<Maybe1>(e1, e2) == e1);
      CHECK(mpc::combine<Maybe1>(e3, e2) == e2);

      using mpc::operators::alternatives::operator||;
      CHECK((e1 or e2) == e1);
      CHECK((e3 or e2) == e2);
    }
  }
}

/*
TEST_CASE("data Either", "[data]") {
  { // Either
    using Either1 = mpc::Either<int, double>;
    {
      const Either1 e{mpc::make_left(1)};
      CHECK(e.index() == 0);
    }
    {
      const Either1 e{mpc::make_right(2.0)};
      CHECK(e.index() == 1);
    }
    { // functor
      static_assert(mpc::functor<Either1>);
      const Either1 e1{mpc::make_right(2.0)};
      const Either1 e2{mpc::make_right(4.0)};
      const auto f = [](const auto& x) { return x * 2; };

      const mpc::detail::fmap_op<Either1> fmap_op;
      CHECK(fmap_op(f, e1) == e2);
      CHECK(mpc::fmap<Either1>(f, e1) == e2);
    }
    { // applicative
      static_assert(mpc::applicative<Either1>);
      const Either1 e1{mpc::make_right(2.0)};
      const Either1 e2{mpc::make_right(4.0)};
      const mpc::Either<int, std::function<double(double)>> f1{
        mpc::make_right([](double x) { return x * 2; })};

      CHECK(mpc::seq_apply<Either1>(f1, e1) == e2);

      const auto add =
        mpc::perfect_forwarded_t<decltype([](const auto& x, const auto& y) { return x + y; })>{};
      const mpc::applicatives::detail::liftA2_op<Either1> liftA2_op;
      CHECK(liftA2_op(add, e1, e1) == e2);
      CHECK(mpc::liftA2<Either1>(add, e1, e1) == e2);
    }
    { // monad
      static_assert(mpc::monad<Either1>);
      const Either1 e1{mpc::make_right(2.0)};
      const Either1 e2{mpc::make_left(0)};
      const auto f = [](const auto& x) { return Either1{mpc::make_right(x + 1)}; };

      CHECK(mpc::bind<Either1>(e1, f) == Either1{mpc::make_right(3.0)});
      CHECK(mpc::bind<Either1>(mpc::bind<Either1>(e1, f), f) == Either1{mpc::make_right(4.0)});
      CHECK(mpc::bind<Either1>(e2, f) == e2);
      CHECK(mpc::bind<Either1>(mpc::bind<Either1>(e2, f), f) == e2);

      const auto bind = mpc::bind<Either1>;
      CHECK(mpc::infixl(e1, bind, f) == Either1{mpc::make_right(3.0)});
      CHECK(mpc::infixl(e1, bind, f, bind, f) == Either1{mpc::make_right(4.0)});
      CHECK(mpc::infixl(e2, bind, f) == e2);
      CHECK(mpc::infixl(e2, bind, f, bind, f) == e2);
    }
  }
}
*/
