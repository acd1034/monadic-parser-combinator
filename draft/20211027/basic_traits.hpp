/// @file basic_traits.hpp
#pragma once
#include <mpc/prelude.hpp>

namespace mpc {
  /// class Functor f where
  template <class>
  struct basic_functor_traits;

  /// class Applicative f where
  template <class>
  struct basic_applicative_traits;

  /// class Alternative f where
  template <class>
  struct basic_alternative_traits;

  /// class Monad m where
  template <class>
  struct basic_monad_traits;

  /// class Functor f where
  template <class>
  struct functor_traits;

  /// class Applicative f where
  template <class>
  struct applicative_traits;

  /// class Alternative f where
  template <class>
  struct alternative_traits;

  /// class Monad m where
  template <class>
  struct monad_traits;

  // clang-format off
  template <class M>
  concept monad = requires {
    basic_monad_traits<std::remove_cvref_t<M>>::returns;
    basic_monad_traits<std::remove_cvref_t<M>>::bind;
  };

  template <class F>
  concept applicative =
    monad<F> or
    requires { basic_applicative_traits<std::remove_cvref_t<F>>::pure; }
    and (
      requires { basic_applicative_traits<std::remove_cvref_t<F>>::seq_apply; } or
      requires { basic_applicative_traits<std::remove_cvref_t<F>>::liftA2; }
    );

  template <class F>
  concept functor =
    applicative<F> or
    requires { basic_functor_traits<std::remove_cvref_t<F>>::fmap; };
  // clang-format on
} // namespace mpc
