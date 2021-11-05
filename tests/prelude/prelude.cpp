#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/prelude.hpp>
#include "../stdfundamental.hpp"

using add_op = decltype([](const auto& x, const auto& y) { return x + y; });

struct add_t : mpc::perfect_forward<add_op> {
  using mpc::perfect_forward<add_op>::perfect_forward;
};

inline constexpr add_t add;

TEST_CASE("prelude compose", "[prelude][compose]") {
  {
    const auto inc = [](const auto& x) { return x + 1; };
    const auto two = mpc::compose(inc, inc);
    CHECK(two(1) == 3);
  }
  {
    CHECK(add(1, 1) == 2);
    const auto add1 = add(1);
    CHECK(add1(1) == 2);
    const auto add2 = mpc::compose(add1, add1);
    CHECK(add2(1) == 3);
  }
  {
    CHECK(add % 1 % 1 == 2);
    const auto add1 = add % 1;
    CHECK(add1 % 1 == 2);
    CHECK(mpc::compose % add1 % add1 % 1 == 3);
    const auto composed = mpc::compose % add1 % add1;
    CHECK(composed(1) == 3);
  }
  {
    const auto add1 = add % 1;
    const auto incomplete_compose = mpc::compose % add1;
    const auto composed = incomplete_compose % add1;
    CHECK(composed(1) == 3);
  }
  { // mpc::plus
    CHECK(mpc::plus % 1 % 1 == 2);
    const auto add1 = mpc::plus % 1;
    CHECK(add1 % 1 == 2);
    CHECK(mpc::compose % add1 % add1 % 1 == 3);
    const auto composed = mpc::compose % add1 % add1;
    CHECK(composed(1) == 3);
  }
  {
    const auto add1 = mpc::plus % 1;
    const auto incomplete_compose = mpc::compose % add1;
    const auto composed = incomplete_compose % add1;
    CHECK(composed(1) == 3);
  }
}

using add3way_op = decltype([](const auto& x, const auto& y, const auto& z) { return x + y + z; });

struct add3way_t : mpc::perfect_forward<add3way_op> {
  using mpc::perfect_forward<add3way_op>::perfect_forward;
};

inline constexpr add3way_t add3way;

TEST_CASE("prelude flip", "[prelude][flip]") {
  {
    CHECK(mpc::flip % add % "A"s % "B"s == "BA"s);
    CHECK(mpc::flip % add3way % "A"s % "B"s % "C"s == "BAC"s);
  }
}
