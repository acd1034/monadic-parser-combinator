/// @file operations.hpp
#pragma once
#include <functional> // std::plus, etc.
#include <mpc/functional/perfect_forward.hpp>

namespace mpc::inline cpo {
  // Arithmetic operations
  inline constexpr perfect_forwarded_t<std::plus<>> plus;
  inline constexpr perfect_forwarded_t<std::minus<>> minus;
  inline constexpr perfect_forwarded_t<std::multiplies<>> multiplies;
  inline constexpr perfect_forwarded_t<std::divides<>> divides;
  inline constexpr perfect_forwarded_t<std::modulus<>> modulus;
  inline constexpr perfect_forwarded_t<std::negate<>> negate;

  // Comparisons
  inline constexpr perfect_forwarded_t<std::ranges::equal_to> equal_to;
  inline constexpr perfect_forwarded_t<std::ranges::not_equal_to> not_equal_to;
  inline constexpr perfect_forwarded_t<std::ranges::greater> greater;
  inline constexpr perfect_forwarded_t<std::ranges::less> less;
  inline constexpr perfect_forwarded_t<std::ranges::greater_equal> greater_equal;
  inline constexpr perfect_forwarded_t<std::ranges::less_equal> less_equal;
  // WORKAROUND
  // inline constexpr perfect_forwarded_t<std::compare_three_way> compare_three_way;

  // Logical operations
  inline constexpr perfect_forwarded_t<std::logical_and<>> logical_and;
  inline constexpr perfect_forwarded_t<std::logical_or<>> logical_or;
  inline constexpr perfect_forwarded_t<std::logical_not<>> logical_not;

  // Bitwise operations
  inline constexpr perfect_forwarded_t<std::bit_and<>> bit_and;
  inline constexpr perfect_forwarded_t<std::bit_or<>> bit_or;
  inline constexpr perfect_forwarded_t<std::bit_xor<>> bit_xor;
  inline constexpr perfect_forwarded_t<std::bit_not<>> bit_not;
} // namespace mpc::inline cpo
