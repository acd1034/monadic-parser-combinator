/// @file flip.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional/perfect_forward.hpp>

// clang-format off

namespace mpc {
  namespace detail {
    struct flip_op {
      struct closure {
        template<class Fn, class T, class U, class... Args>
        constexpr auto operator()(Fn&& f, T&& t, U&& u, Args&&... args) const noexcept(
          noexcept(   std::invoke(std::forward<Fn>(f), std::forward<U>(u), std::forward<T>(t), std::forward<Args>(args)...)))
          -> decltype(std::invoke(std::forward<Fn>(f), std::forward<U>(u), std::forward<T>(t), std::forward<Args>(args)...))
          { return    std::invoke(std::forward<Fn>(f), std::forward<U>(u), std::forward<T>(t), std::forward<Args>(args)...); }
      };

      // NOTE: You cannot write as `perfect_forwarded_t<closure>{}(std::forward<Fn>(f))`.
      template<class Fn>
      constexpr auto operator()(Fn&& f) const noexcept(
        noexcept(   perfect_forwarded_t<closure, std::decay_t<Fn>>(closure{}, std::forward<Fn>(f))))
        -> decltype(perfect_forwarded_t<closure, std::decay_t<Fn>>(closure{}, std::forward<Fn>(f)))
        { return    perfect_forwarded_t<closure, std::decay_t<Fn>>(closure{}, std::forward<Fn>(f)); }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr perfect_forwarded_t<detail::flip_op> flip;
  }
} // namespace mpc

// clang-format on
