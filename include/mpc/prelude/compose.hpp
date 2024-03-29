/// @file compose.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional/partial.hpp>

// clang-format off

namespace mpc {
  // compose
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__functional/compose.h

  namespace detail {
    struct compose_op {
      struct closure {
        template<class Fn1, class Fn2, class... Args>
        constexpr auto operator()(Fn1&& f1, Fn2&& f2, Args&&... args) const noexcept(
          noexcept(   std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...))))
          -> decltype(std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...)))
          { return    std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...)); }
      };

      // NOTE: You cannot write as `partial<closure>{}(std::forward<Fn1>(f1), std::forward<Fn2>(f2))`.
      template<class Fn1, class Fn2>
      constexpr auto operator()(Fn1&& f1, Fn2&& f2) const noexcept(
        noexcept(   partial(closure{}, std::forward<Fn1>(f1), std::forward<Fn2>(f2))))
        -> decltype(partial(closure{}, std::forward<Fn1>(f1), std::forward<Fn2>(f2)))
        { return    partial(closure{}, std::forward<Fn1>(f1), std::forward<Fn2>(f2)); }
    };
  } // namespace detail

  inline namespace cpo {
    /**
     * @brief Function composition.
     */
    inline constexpr partial<detail::compose_op> compose;
  }
} // namespace mpc

// clang-format on
