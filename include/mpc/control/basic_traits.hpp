/// @file basic_traits.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc {
  // functor, applicative, monad
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html

  /// class Functor f where
  template <class>
  struct functor_traits;

  /// class Functor f => Applicative f where
  template <class>
  struct applicative_traits;

  /// class Applicative m => Monad m where
  template <class>
  struct monad_traits;

  // clang-format off
  template <class F>
  concept functor = requires {
    functor_traits<std::remove_cvref_t<F>>::fmap;
    functor_traits<std::remove_cvref_t<F>>::replace2nd;
  };

  template <class F>
  concept applicative_traits_specialized = requires {
    applicative_traits<std::remove_cvref_t<F>>::pure;
    applicative_traits<std::remove_cvref_t<F>>::seq_apply;
    applicative_traits<std::remove_cvref_t<F>>::liftA2;
    applicative_traits<std::remove_cvref_t<F>>::discard2nd;
    applicative_traits<std::remove_cvref_t<F>>::discard1st;
  };

  template <class F>
  concept applicative = functor<F> and applicative_traits_specialized<F>;

  template <class M>
  concept monad_traits_specialized = requires {
    monad_traits<std::remove_cvref_t<M>>::bind;
  };

  template <class M>
  concept monad = applicative<M> and monad_traits_specialized<M>;
  // clang-format on
} // namespace mpc
