/// @file alternative.hpp
#pragma once
#include <mpc/control/applicative.hpp>
#include <mpc/functional/perfect_forward.hpp>

// clang-format off

namespace mpc {
  // alternative
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html

  /// class Applicative f => Alternative f where
  template <class>
  struct alternative_traits;

  namespace detail {
    template <class F>
    concept has_alternative_traits_empty = requires {
      alternative_traits<std::remove_cvref_t<F>>::empty;
    };

    template <class F>
    concept has_alternative_traits_combine = requires {
      alternative_traits<std::remove_cvref_t<F>>::combine;
    };
  } // namespace detail

  template <class F>
  concept alternative_traits_specialized =
    detail::has_alternative_traits_empty<F> and detail::has_alternative_traits_combine<F>;

  template <class F>
  concept alternative = applicative<F> and alternative_traits_specialized<F>;

  // class requirements

  namespace detail {
    /// combine :: f a -> f a -> f a
    struct combine_op {
      template <class Fa, class Fb>
      /* requires std::same_as<std::remove_cvref_t<Fa>, std::remove_cvref_t<Fb>> */
      constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
      noexcept(   alternative_traits<std::remove_cvref_t<Fa>>::combine(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(alternative_traits<std::remove_cvref_t<Fa>>::combine(std::forward<Fa>(fa), std::forward<Fb>(fb)))
      { return    alternative_traits<std::remove_cvref_t<Fa>>::combine(std::forward<Fa>(fa), std::forward<Fb>(fb)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// empty :: f a
    template <class F>
    requires requires {
      alternative_traits<std::remove_cvref_t<F>>::empty;
    }
    inline constexpr auto empty = alternative_traits<std::remove_cvref_t<F>>::empty;

    /// combine :: f a -> f a -> f a -- infixl 3
    inline constexpr perfect_forwarded_t<detail::combine_op> combine{};
  } // namespace cpo

  namespace operators::alternatives {
    template <class Fa, class Fb>
    requires /* std::same_as<std::remove_cvref_t<Fa>, std::remove_cvref_t<Fb>> and */ requires {
      alternative_traits<std::remove_cvref_t<Fa>>::combine;
    }
    inline constexpr auto operator||(Fa&& fa, Fb&& fb)
      noexcept(noexcept(mpc::combine(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(      mpc::combine(std::forward<Fa>(fa), std::forward<Fb>(fb))) {
      return            mpc::combine(std::forward<Fa>(fa), std::forward<Fb>(fb));
    }
  } // namespace operators::alternatives
} // namespace mpc

// clang-format on
