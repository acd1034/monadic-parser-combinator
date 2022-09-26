#include <catch2/catch_test_macros.hpp>
#include <mpc/control.hpp>
#include <mpc/data.hpp>
#include "../stdfundamental.hpp"

TEST_CASE("control maybe", "[control]") {
  {
    using maybe = mpc::maybe<int>;
    maybe m1{mpc::make_just(1.0)};
    maybe m2{mpc::make_just(2.0)};
    maybe m3{mpc::nothing};
    const auto fn = [](auto x) -> maybe {
      if (x % 2 == 0)
        return mpc::nothing;
      else
        return mpc::make_just(x * 2);
    };
    CHECK(mpc::bind(m1, fn) == maybe{mpc::make_just(2.0)});
    CHECK(mpc::bind(m2, fn) == maybe{mpc::nothing});
    CHECK(mpc::bind(m3, fn) == maybe{mpc::nothing});
  }
}
