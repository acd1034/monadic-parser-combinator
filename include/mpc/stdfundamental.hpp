/// @file stdfundamental.hpp
#pragma once
#include <cassert> // assert
#include <compare>
#include <concepts>
#include <cstddef>          // size_t, ptrdiff_t, nullptr_t
#include <cstdint>          // int32_t
#include <initializer_list> // initializer_list
#include <tuple>            // tuple
#include <type_traits>      // enable_if_t, void_t, true_type, invoke_result, etc.
// #include <utility>          // move, forward, pair, swap, exchange, declval

namespace mpc {
  /// always_false
  template <class...>
  inline constexpr bool always_false = false;

  /// always_true_type
  template <class...>
  using always_true_type = std::true_type;

  /// index_constant
  template <std::size_t N>
  using index_constant = std::integral_constant<std::size_t, N>;

  /// _and
  template <class... Bn>
  using _and = std::conjunction<Bn...>;

  /// _or
  template <class... Bn>
  using _or = std::disjunction<Bn...>;

  /// _not
  template <class B>
  using _not = std::negation<B>;

  /// _and_v
  template <class... Bn>
  inline constexpr bool _and_v = _and<Bn...>::value;

  /// _or_v
  template <class... Bn>
  inline constexpr bool _or_v = _or<Bn...>::value;

  /// _not_v
  template <class B>
  inline constexpr bool _not_v = _not<B>::value;
} // namespace mpc
