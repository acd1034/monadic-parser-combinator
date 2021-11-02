/// @file alternative.hpp
#pragma once
#include <mpc/control/applicative.hpp>
#include <mpc/control/basic_traits.hpp>
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  // alternative
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html

  /// class Applicative f => Alternative f where
  template <class>
  struct alternative_traits;

  template <class F>
  concept alternative_traits_specialized = requires {
    alternative_traits<std::remove_cvref_t<F>>::empty;
    alternative_traits<std::remove_cvref_t<F>>::combine;
  };

  template <class F>
  concept alternative = applicative<F> and alternative_traits_specialized<F>;

  namespace detail {
    template <class F>
    using combine_op =
      std::remove_cvref_t<decltype(alternative_traits<std::remove_cvref_t<F>>::combine)>;

    template <class F>
    struct combine_t : perfect_forward<combine_op<F>> {
      using perfect_forward<combine_op<F>>::perfect_forward;
    };
  } // namespace detail

  inline namespace cpo {
    /// empty :: f a
    template <class F>
    requires requires {
      alternative_traits<std::remove_cvref_t<F>>::empty;
    }
    inline constexpr auto empty = alternative_traits<std::remove_cvref_t<F>>::empty;

    /// (<|>) :: f a -> f a -> f a -- infixl 3
    template <class F>
    inline constexpr detail::combine_t<std::remove_cvref_t<F>> combine{};

    // TODO: Implement `some`, `many`
  } // namespace cpo

  namespace operators::alternatives {
    // clang-format off
    template <class F1, class F2>
    requires /* std::same_as<std::remove_cvref_t<F1>, std::remove_cvref_t<F2>> and  */ requires {
      alternative_traits<std::remove_cvref_t<F1>>::combine;
    }
    constexpr auto operator||(F1&& a1, F2&& a2)
      noexcept(noexcept(mpc::combine<F1>(std::forward<F1>(a1), std::forward<F2>(a2))))
      -> decltype(      mpc::combine<F1>(std::forward<F1>(a1), std::forward<F2>(a2))) {
      return            mpc::combine<F1>(std::forward<F1>(a1), std::forward<F2>(a2));
    }
    // clang-format on
  } // namespace operators::alternatives
} // namespace mpc
