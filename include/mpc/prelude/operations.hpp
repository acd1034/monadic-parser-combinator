/// @file operations.hpp
#pragma once
#include <functional> // std::plus, etc.
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  /// Customization point objects are here.
  inline namespace cpo {
    // Arithmetic operations
    /// Partially applicable std::plus<>.
    inline constexpr partially_applicable<std::plus<>> plus;
    /// Partially applicable std::minus<>.
    inline constexpr partially_applicable<std::minus<>> minus;
    /// Partially applicable std::multiplies<>.
    inline constexpr partially_applicable<std::multiplies<>> multiplies;
    /// Partially applicable std::divides<>.
    inline constexpr partially_applicable<std::divides<>> divides;
    /// Partially applicable std::modulus<>.
    inline constexpr partially_applicable<std::modulus<>> modulus;
    /// Partially applicable std::negate<>.
    inline constexpr partially_applicable<std::negate<>> negate;

    // Comparisons
    /// Partially applicable std::ranges::equal_to.
    inline constexpr partially_applicable<std::ranges::equal_to> equal_to;
    /// Partially applicable std::ranges::not_equal_to.
    inline constexpr partially_applicable<std::ranges::not_equal_to> not_equal_to;
    /// Partially applicable std::ranges::greater.
    inline constexpr partially_applicable<std::ranges::greater> greater;
    /// Partially applicable std::ranges::less.
    inline constexpr partially_applicable<std::ranges::less> less;
    /// Partially applicable std::ranges::greater_equal.
    inline constexpr partially_applicable<std::ranges::greater_equal> greater_equal;
    /// Partially applicable std::ranges::less_equal.
    inline constexpr partially_applicable<std::ranges::less_equal> less_equal;
    // WORKAROUND: LLVM 13.0.0 has not implemented std::compare_three_way
    // /// Partially applicable std::compare_three_way.
    // inline constexpr partially_applicable<std::compare_three_way> compare_three_way;

    // Logical operations
    /// Partially applicable std::logical_and<>.
    inline constexpr partially_applicable<std::logical_and<>> logical_and;
    /// Partially applicable std::logical_or<>.
    inline constexpr partially_applicable<std::logical_or<>> logical_or;
    /// Partially applicable std::logical_not<>.
    inline constexpr partially_applicable<std::logical_not<>> logical_not;

    // Bitwise operations
    /// Partially applicable std::bit_and<>.
    inline constexpr partially_applicable<std::bit_and<>> bit_and;
    /// Partially applicable std::bit_or<>.
    inline constexpr partially_applicable<std::bit_or<>> bit_or;
    /// Partially applicable std::bit_xor<>.
    inline constexpr partially_applicable<std::bit_xor<>> bit_xor;
    /// Partially applicable std::bit_not<>.
    inline constexpr partially_applicable<std::bit_not<>> bit_not;
  } // namespace cpo
} // namespace mpc
