/// @file functor.hpp
#pragma once
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/prelude/compose.hpp>
#include <mpc/prelude/constant.hpp>
#include <mpc/prelude/flip.hpp>

namespace mpc {
  // functor
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html#t:Functor

  /// class Functor f where
  template <class>
  struct functor_traits;

  /// Requires fmap and replace2nd is valid in @link mpc::functor_traits functor_traits @endlink.
  template <class F>
  concept functor = requires {
    functor_traits<std::remove_cvref_t<F>>::fmap;
    functor_traits<std::remove_cvref_t<F>>::replace2nd;
  };

  // Methods required for the class definition.

  namespace detail {
    /// fmap :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class Fn, class Fa>
      constexpr auto operator()(Fn&& fn, Fa&& fa) const noexcept(
      noexcept(   functor_traits<std::remove_cvref_t<Fa>>::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa))))
      -> decltype(functor_traits<std::remove_cvref_t<Fa>>::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)))
      { return    functor_traits<std::remove_cvref_t<Fa>>::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)); }
    };

    /**
     * @brief replace2nd :: a -> f b -> f a
     * @details (<$) in Haskell
     */
    struct replace2nd_op {
      template <class A, class Fb>
      constexpr auto operator()(A&& a, Fb&& fb) const noexcept(
      noexcept(   functor_traits<std::remove_cvref_t<Fb>>::replace2nd(std::forward<A>(a), std::forward<Fb>(fb))))
      -> decltype(functor_traits<std::remove_cvref_t<Fb>>::replace2nd(std::forward<A>(a), std::forward<Fb>(fb)))
      { return    functor_traits<std::remove_cvref_t<Fb>>::replace2nd(std::forward<A>(a), std::forward<Fb>(fb)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// @copydoc mpc::detail::fmap_op
    inline constexpr perfect_forwarded_t<detail::fmap_op> fmap{};

    /// @copydoc mpc::detail::replace2nd_op
    inline constexpr perfect_forwarded_t<detail::replace2nd_op> replace2nd{};
  } // namespace cpo

  /// Methods deducible from other methods of @link mpc::functor functor @endlink.
  namespace functors {
    /**
     * @copydoc mpc::detail::replace2nd_op
     * @details
     * ```
     * replace2nd = fmap . constant
     * ```
     */
    inline constexpr auto replace2nd = compose(mpc::fmap, constant);
  } // namespace functors

  // Grobal methods

  inline namespace cpo {
    /**
     * @brief replace1st :: Functor f => f a -> b -> f b
     * @details ($>) in Haskell
     * ```
     * replace1st = flip replace2nd
     * ```
     */
    inline constexpr auto replace1st = flip % mpc::replace2nd;
  } // namespace cpo
} // namespace mpc
