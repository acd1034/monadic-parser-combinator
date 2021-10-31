#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <mpc/type_traits.hpp>

struct ExplicitlyDefaultConstructible {
  explicit ExplicitlyDefaultConstructible();
};

struct ImplicitlyDefaultConstructible {
  ImplicitlyDefaultConstructible();
};

// Conversion from `ExplicitlyConvertible` to `ExplicitlyConstructible` is explicit.
// Conversion from `ImplicitlyConvertible` to `ImplicitlyConstructible` is implicit.
// https://github.com/llvm/llvm-project/blob/e356027016c6365b3d8924f54c33e2c63d931492/libcxx/test/std/concepts/concepts.lang/concept.convertible/convertible_to.pass.cpp
struct ExplicitlyConvertible;
struct ImplicitlyConvertible;

struct ExplicitlyConstructible {
  explicit ExplicitlyConstructible(int);
  explicit ExplicitlyConstructible(ExplicitlyConvertible);
  ExplicitlyConstructible(ImplicitlyConvertible) = delete;
};

struct ExplicitlyConvertible {
  explicit operator ExplicitlyConstructible() const {
    return ExplicitlyConstructible(0);
  }
};

struct ImplicitlyConstructible {
  ImplicitlyConstructible(ImplicitlyConvertible);
};

struct ImplicitlyConvertible {
  operator ExplicitlyConstructible() const {
    return ExplicitlyConstructible(0);
  }
  // FIXME: conversion from 'ImplicitlyConvertible' to 'ImplicitlyConstructible' is ambiguous.
  // GCC bug???
  // operator ImplicitlyConstructible() = delete;
};

TEST_CASE("type_traits", "[type_traits][meta]") {
  {
    using mpc::detail::is_implicitly_default_constructible_v;
    // clang-format off
    static_assert(    is_implicitly_default_constructible_v<ImplicitlyDefaultConstructible>);
    static_assert(not is_implicitly_default_constructible_v<ExplicitlyDefaultConstructible>);
    // clang-format on
  }
  {
    using mpc::detail::is_explicitly_convertible_v, mpc::detail::is_implicitly_convertible_v;
    // clang-format off
    static_assert(    is_explicitly_convertible_v<ExplicitlyConvertible, ExplicitlyConstructible>);
    static_assert(    is_explicitly_convertible_v<ImplicitlyConvertible, ImplicitlyConstructible>);
    static_assert(not is_implicitly_convertible_v<ExplicitlyConvertible, ExplicitlyConstructible>);
    static_assert(    is_implicitly_convertible_v<ImplicitlyConvertible, ImplicitlyConstructible>);
    // clang-format on
  }
}
