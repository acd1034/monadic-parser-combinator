#define CATCH_CONFIG_MAIN
#include <algorithm>
#include <catch2/catch.hpp>
#include "./stdfundamental.hpp"

TEST_CASE("test name", "[label1][label2]") {}

TEMPLATE_TEST_CASE("algorithm equal", "[algorithm][equal]", (std::array<int, 3>), std::vector<int>,
                   int[]) {
  TestType a{1, 2, 3};
  CHECK(std::ranges::equal(a, a));
}
