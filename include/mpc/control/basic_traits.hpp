/// @file basic_traits.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc {
  /// class Functor f where
  template <class>
  struct functor_traits;

  /// class Applicative f where
  template <class>
  struct applicative_traits;

  /// class Monad m where
  template <class>
  struct monad_traits;

  // clang-format off
  template <class F>
  concept functor_traits_specialized = requires {
    functor_traits<std::remove_cvref_t<F>>::fmap;
    functor_traits<std::remove_cvref_t<F>>::replace2nd;
  };

  template <class F>
  concept functor = functor_traits_specialized<F>;

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
