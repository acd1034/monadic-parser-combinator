/// @file alternative.hpp
#pragma once
#include <mpc/control/applicative.hpp>
#include <mpc/functional/perfect_forward.hpp>

// clang-format off

namespace mpc {
  // alternative
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html#t:Alternative

  /// class Applicative f => Alternative f where
  template <class>
  struct alternative_traits;

  namespace detail {
    /// has_alternative_traits_empty
    template <class F>
    concept has_alternative_traits_empty = requires {
      alternative_traits<std::remove_cvref_t<F>>::empty();
    };

    /// has_alternative_traits_combine
    template <class F>
    concept has_alternative_traits_combine = requires {
      alternative_traits<std::remove_cvref_t<F>>::combine;
    };
  } // namespace detail

  /// alternative_traits_specialized
  template <class F>
  concept alternative_traits_specialized =
    detail::has_alternative_traits_empty<F> and detail::has_alternative_traits_combine<F>;

  /// Requires applicative and empty and combine is valid in @link mpc::alternative_traits alternative_traits @endlink.
  template <class F>
  concept alternative = applicative<F> and alternative_traits_specialized<F>;

  // Methods required for the class definition.

  namespace detail {
    /**
     * @brief empty :: f a
     * @details Use operator* to access the value.
     */
    template <class F>
    struct empty_op {
      constexpr auto operator*() const noexcept(
      noexcept(   alternative_traits<std::remove_cvref_t<F>>::empty()))
      -> decltype(alternative_traits<std::remove_cvref_t<F>>::empty())
      { return    alternative_traits<std::remove_cvref_t<F>>::empty(); }
    };

    /**
     * @brief combine :: f a -> f a -> f a
     * @details (<|>) in Haskell
     */
    struct combine_op {
      template <class Fa, class Fb>
      requires std::same_as<std::remove_cvref_t<Fa>, std::remove_cvref_t<Fb>>
      constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
      noexcept(   alternative_traits<std::remove_cvref_t<Fa>>::combine(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(alternative_traits<std::remove_cvref_t<Fa>>::combine(std::forward<Fa>(fa), std::forward<Fb>(fb)))
      { return    alternative_traits<std::remove_cvref_t<Fa>>::combine(std::forward<Fa>(fa), std::forward<Fb>(fb)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// @copydoc mpc::detail::empty_op
    template <class F>
    inline constexpr detail::empty_op<F> empty{};

    /// @copydoc mpc::detail::combine_op
    inline constexpr perfect_forwarded_t<detail::combine_op> combine{};
  } // namespace cpo

  // Grobal methods

  namespace operators::alternatives {
    /// @copydoc mpc::detail::combine_op
    template <class Fa, class Fb>
    requires std::same_as<std::remove_cvref_t<Fa>, std::remove_cvref_t<Fb>>
    inline constexpr auto operator||(Fa&& fa, Fb&& fb)
      noexcept(noexcept(mpc::combine(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(      mpc::combine(std::forward<Fa>(fa), std::forward<Fb>(fb))) {
      return            mpc::combine(std::forward<Fa>(fa), std::forward<Fb>(fb));
    }
  } // namespace operators::alternatives
} // namespace mpc

// clang-format on
