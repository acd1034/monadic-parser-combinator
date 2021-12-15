/// @file infix.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/stdfundamental.hpp>

// clang-format off

namespace mpc {
  /**
   * @brief A function that implements the right-associative infix notation.
   * @details @code
   * infixr(a1, op, a2) = op(a1, a2)
   * infixr(a1, op, args...) = op(a1, infixr(args...))
   * @endcode
   */
  template<class A1, class Op, class A2>
  constexpr auto infixr(A1&& a1, Op&& op, A2&& a2)
  noexcept(noexcept(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2))))
  -> decltype(      std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)))
  { return          std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)); }

  /// @overl `infixr`
  template<class A1, class Op, class... Args>
  constexpr auto infixr(A1&& a1, Op&& op, Args&&... args)
  noexcept(noexcept(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), infixr(std::forward<Args>(args)...))))
  -> decltype(      std::invoke(std::forward<Op>(op), std::forward<A1>(a1), infixr(std::forward<Args>(args)...)))
  { return          std::invoke(std::forward<Op>(op), std::forward<A1>(a1), infixr(std::forward<Args>(args)...)); }

  /**
   * @brief A function that implements the left-associative infix notation.
   * @details @code
   * infixl(a1, op, a2) = op(a1, a2)
   * infixl(a1, op, a2, args...) = infixl(op(a1, a2), args...)
   * @endcode
   */
  template<class A1, class Op, class A2>
  constexpr auto infixl(A1&& a1, Op&& op, A2&& a2)
  noexcept(noexcept(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2))))
  -> decltype(      std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)))
  { return          std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)); }

  /// @overl `infixl`
  template<class A1, class Op, class A2, class... Args>
  constexpr auto infixl(A1&& a1, Op&& op, A2&& a2, Args&&... args)
  noexcept(noexcept(infixl(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)), std::forward<Args>(args)...)))
  -> decltype(      infixl(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)), std::forward<Args>(args)...))
  { return          infixl(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)), std::forward<Args>(args)...); }
} // namespace mpc

// clang-format on
