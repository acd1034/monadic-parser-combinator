/// @file functor.hpp
#pragma once
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/prelude/compose.hpp>
#include <mpc/prelude/constant.hpp>
#include <mpc/prelude/flip.hpp>

namespace mpc {
  // functor
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html

  /// class Functor f where
  template <class>
  struct functor_traits;

  template <class F>
  concept functor = requires {
    functor_traits<std::remove_cvref_t<F>>::fmap;
    functor_traits<std::remove_cvref_t<F>>::replace2nd;
  };

  // class requirements

  namespace detail {
    /// fmap :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class Fn, class Fa>
      constexpr auto operator()(Fn&& fn, Fa&& fa) const noexcept(
      noexcept(   functor_traits<std::remove_cvref_t<Fa>>::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa))))
      -> decltype(functor_traits<std::remove_cvref_t<Fa>>::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)))
      { return    functor_traits<std::remove_cvref_t<Fa>>::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)); }
    };

    /// replace2nd :: a -> f b -> f a
    struct replace2nd_op {
      template <class A, class Fb>
      constexpr auto operator()(A&& a, Fb&& fb) const noexcept(
      noexcept(   functor_traits<std::remove_cvref_t<Fb>>::replace2nd(std::forward<A>(a), std::forward<Fb>(fb))))
      -> decltype(functor_traits<std::remove_cvref_t<Fb>>::replace2nd(std::forward<A>(a), std::forward<Fb>(fb)))
      { return    functor_traits<std::remove_cvref_t<Fb>>::replace2nd(std::forward<A>(a), std::forward<Fb>(fb)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// fmap :: (a -> b) -> f a -> f b
    inline constexpr perfect_forwarded_t<detail::fmap_op> fmap{};

    /// replace2nd :: a -> f b -> f a
    inline constexpr perfect_forwarded_t<detail::replace2nd_op> replace2nd{};
  } // namespace cpo

  // Deducibles

  namespace functors {
    /// @brief replace2nd = fmap . const
    /// @details If you define `functor_traits<F>::fmap`, you can deduce `replace2nd`.
    inline constexpr auto replace2nd = compose(mpc::fmap, constant);
  } // namespace functors

  // Grobal methods

  inline namespace cpo {
    /// replace1st :: Functor f => f a -> b -> f b
    /// replace1st = flip replace2nd
    inline constexpr auto replace1st = flip % mpc::replace2nd;
  } // namespace cpo
} // namespace mpc
