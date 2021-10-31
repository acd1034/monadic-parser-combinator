/// @file compose.hpp
#pragma once

#include <functional>
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  // clang-format off
  struct compose_op {
    template<class Fn1, class Fn2, class... Args>
    constexpr auto operator()(Fn1&& f1, Fn2&& f2, Args&&... args) const
      noexcept(noexcept(std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...))))
      -> decltype(std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...)))
      { return std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...)); }
  };

  template <class Fn1, class Fn2>
  struct compose_t : perfect_forward<compose_op, Fn1, Fn2> {
    using perfect_forward<compose_op, Fn1, Fn2>::perfect_forward;
  };

  template <class Fn1, class Fn2>
  constexpr auto compose(Fn1&& f1, Fn2&& f2)
    noexcept(noexcept(compose_t<std::decay_t<Fn1>, std::decay_t<Fn2>>(std::forward<Fn1>(f1), std::forward<Fn2>(f2))))
    -> decltype(compose_t<std::decay_t<Fn1>, std::decay_t<Fn2>>(std::forward<Fn1>(f1), std::forward<Fn2>(f2)))
    { return compose_t<std::decay_t<Fn1>, std::decay_t<Fn2>>(std::forward<Fn1>(f1), std::forward<Fn2>(f2)); }
  // clang-format on
} // namespace mpc
