#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <mpc/control.hpp>
#include <mpc/data.hpp>
#include "../stdfundamental.hpp"

TEST_CASE("control", "[control]") {
  using myeither = mpc::either<int, double>;
  using traits = mpc::monad_traits<myeither>;
  traits::bind(
    myeither{mpc::make_right(1.0)},
    [](const auto& x) -> myeither { return mpc::make_right(x + 1); }
  );
  mpc::bind<myeither>(
    myeither{mpc::make_right(1.0)},
    [](const auto& x) -> myeither { return mpc::make_right(x + 1); }
  );
  static_assert(mpc::monad<mpc::either<int, double>>);
  static_assert(mpc::applicative<mpc::either<int, double>>);
  static_assert(mpc::functor<mpc::either<int, double>>);
}

template <class>
struct traits;

template <>
struct traits<int> {
  static constexpr auto pure() {
    return 0;
  }
  static constexpr auto sing(auto&&) {
    return 0;
  }
};

template <class T>
concept has_pure = requires {
  traits<T>::pure;
};

template <class T>
concept has_sing = requires {
  traits<T>::sing;
};

static_assert(has_pure<int>); // ok
// static_assert(has_sing<int>); // ng
