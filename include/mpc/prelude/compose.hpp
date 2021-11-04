/// @file compose.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional.hpp> // perfect_forward

namespace mpc {
  // compose
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__functional/compose.h

  // clang-format off
  namespace detail {
    struct composed_fn_op {
      template<class Fn1, class Fn2, class... Args>
      constexpr auto operator()(Fn1&& f1, Fn2&& f2, Args&&... args) const
        noexcept(noexcept(std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...))))
        -> decltype(      std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...)))
        { return          std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...)); }
    };

    template <class Fn1, class Fn2>
    struct composed_fn_t : perfect_forward<composed_fn_op, Fn1, Fn2> {
      using perfect_forward<composed_fn_op, Fn1, Fn2>::perfect_forward;
    };

    struct compose_op {
      template<class Fn1, class Fn2>
      constexpr auto operator()(Fn1&& f1, Fn2&& f2) const
        noexcept(noexcept(composed_fn_t<std::decay_t<Fn1>, std::decay_t<Fn2>>(std::forward<Fn1>(f1), std::forward<Fn2>(f2))))
        -> decltype(      composed_fn_t<std::decay_t<Fn1>, std::decay_t<Fn2>>(std::forward<Fn1>(f1), std::forward<Fn2>(f2)))
        { return          composed_fn_t<std::decay_t<Fn1>, std::decay_t<Fn2>>(std::forward<Fn1>(f1), std::forward<Fn2>(f2)); }
    };

    struct compose_t : perfect_forward<compose_op> {
      using perfect_forward<compose_op>::perfect_forward;
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr detail::compose_t compose;
  }
  // clang-format on
} // namespace mpc
