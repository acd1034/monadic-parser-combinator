/// @file compose.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional/perfect_forward.hpp>

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

      // NOTE: You cannot write as `perfect_forwarded_t<closure>{}(std::forward<Fn1>(f1), std::forward<Fn2>(f2))`.
      template<class Fn1, class Fn2>
      constexpr auto operator()(Fn1&& f1, Fn2&& f2) const noexcept(
        noexcept(   perfect_forwarded_t<closure, std::decay_t<Fn1>, std::decay_t<Fn2>>(closure{}, std::forward<Fn1>(f1), std::forward<Fn2>(f2))))
        -> decltype(perfect_forwarded_t<closure, std::decay_t<Fn1>, std::decay_t<Fn2>>(closure{}, std::forward<Fn1>(f1), std::forward<Fn2>(f2)))
        { return    perfect_forwarded_t<closure, std::decay_t<Fn1>, std::decay_t<Fn2>>(closure{}, std::forward<Fn1>(f1), std::forward<Fn2>(f2)); }
    };
  } // namespace detail

  inline namespace cpo {
    /**
     * @brief Function composition.
     * @rel prelude.hpp
     */
    inline constexpr partially_applicable<detail::compose_op> compose;
  }
} // namespace mpc

// clang-format on
