#include <catch2/catch_test_macros.hpp>
#include <mpc/utility.hpp>
#include "../stdfundamental.hpp"

TEST_CASE("utility", "[utility][meta]") {
  {
    using mpc::is_tuple_like_v;
    // clang-format off
    static_assert(    is_tuple_like_v<std::tuple<int, double, std::nullptr_t>>);
    static_assert(    is_tuple_like_v<std::pair<int, double>>);
    static_assert(    is_tuple_like_v<std::array<int, 3>>);
    static_assert(not is_tuple_like_v<std::vector<int>>);
    static_assert(not is_tuple_like_v<int>);
    static_assert(not is_tuple_like_v<double>);
    // clang-format on
  }
  { // single ctors
    using mpc::single;
    static_assert(mpc::is_tuple_like_v<single<int>>);
    {
      single<int> s1{}; // (1)
      CHECK(*s1 == 0);
    }
    {
      const single s1{1}; // (4)
      CHECK(*s1 == 1);
      single s2{s1}; // (2)
      CHECK(*s2 == 1);
    }
    {
      single s1{1};
      single s2{std::move(s1)}; // (3)
      CHECK(*s2 == 1);
    }
    {
      single<int> s1{1.0}; // (5)
      CHECK(*s1 == 1);
    }
    {
      single<int> s1{std::tuple{1}}; // (6)
      CHECK(*s1 == 1);
    }
    {
      single<std::pair<int, double>> s1{std::in_place, 1, 1.0}; // (7)
      CHECK(s1->first == 1);
      CHECK(s1->second == 1.0);
    }
    {
      // clang-format off
      single<std::vector<int, std::allocator<int>>> s1{
        std::in_place,
        {0, 1, 2},
        std::allocator<int>{}
      }; // (8)
      // clang-format on
      CHECK(s1->front() == 0);
      CHECK(s1->back() == 2);
    }
  }
  { // single ctors with tuple
    using mpc::single;
    static_assert(mpc::is_tuple_like_v<single<int>>);
    std::tuple<int> t1{1};
    {
      single<std::tuple<int>> s1{}; // (1)
      CHECK(*s1 == std::tuple{0});
    }
    {
      const single s1{t1}; // (4)
      CHECK(*s1 == t1);
      single s2{s1}; // (2)
      CHECK(*s2 == t1);
    }
    {
      single s1{t1};
      single s2{std::move(s1)}; // (3)
      CHECK(*s2 == t1);
    }
    {
      single<std::tuple<int>> s1{std::tuple{1.0}}; // (5)
      CHECK(*s1 == t1);
    }
    {
      single<std::tuple<int>> s1{std::tuple{std::tuple{1}}}; // (6)
      CHECK(*s1 == t1);
    }
  }
}
