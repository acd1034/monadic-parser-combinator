#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <mpc/control/trans.hpp>
#include <mpc/data.hpp>
#include "../../stdfundamental.hpp"

template<class S, class M>
using deduce_StateT = mpc::StateT<std::function<M(S)>, S>;

TEST_CASE("trans", "[trans]") {}

TEST_CASE("trans StateT alternative", "[trans][statet][alternative]") {
  static_assert(mpc::alternative<deduce_StateT<int, mpc::Maybe<int>>>);
  static_assert(not mpc::alternative<deduce_StateT<int, mpc::Identity<int>>>);
}
