/// @file constant.hpp
#pragma once
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  namespace detail {
    struct constant_op {
      template <class T, class U>
      constexpr auto operator()(T&& t, U&&) const //
        noexcept(noexcept(std::forward<T>(t)))    //
        -> decltype(std::forward<T>(t)) {
        return std::forward<T>(t);
      }
    };
  } // namespace detail

  inline namespace cpo {
    /**
     * @brief Returns a unary function always returning the first input.
     */
    inline constexpr partially_applicable<detail::constant_op> constant;
  } // namespace cpo
} // namespace mpc
