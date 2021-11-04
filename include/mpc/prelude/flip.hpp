/// @file compose.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional.hpp> // perfect_forward

namespace mpc {
  // clang-format off
  namespace detail {
    struct flipped_fn_op {
      template<class Fn, class T, class U, class... Args>
      constexpr auto operator()(Fn&& f, T&& t, U&& u, Args&&... args) const
        noexcept(noexcept(std::invoke(std::forward<Fn>(f), std::forward<U>(u), std::forward<T>(t), std::forward<Args>(args)...)))
        -> decltype(      std::invoke(std::forward<Fn>(f), std::forward<U>(u), std::forward<T>(t), std::forward<Args>(args)...))
        { return          std::invoke(std::forward<Fn>(f), std::forward<U>(u), std::forward<T>(t), std::forward<Args>(args)...); }
    };

    template <class Fn>
    struct flipped_fn_t : perfect_forward<flipped_fn_op, Fn> {
      using perfect_forward<flipped_fn_op, Fn>::perfect_forward;
    };

    struct flip_op {
      template<class Fn>
      constexpr auto operator()(Fn&& f) const
        noexcept(noexcept(flipped_fn_t<std::decay_t<Fn>>(std::forward<Fn>(f))))
        -> decltype(      flipped_fn_t<std::decay_t<Fn>>(std::forward<Fn>(f)))
        { return          flipped_fn_t<std::decay_t<Fn>>(std::forward<Fn>(f)); }
    };

    struct flip_t : perfect_forward<flip_op> {
      using perfect_forward<flip_op>::perfect_forward;
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr detail::flip_t flip;
  }
  // clang-format on
} // namespace mpc
