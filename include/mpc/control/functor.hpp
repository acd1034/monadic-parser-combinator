/// @file functor.hpp
#pragma once
#include <mpc/control/basic_traits.hpp>
#include <mpc/prelude.hpp> // compose, constant, flip

namespace mpc {
  namespace detail {
    template <class F>
    using fmap_op = std::remove_cvref_t<decltype(functor_traits<std::remove_cvref_t<F>>::fmap)>;

    template <class F>
    using replace2nd_op =
      std::remove_cvref_t<decltype(functor_traits<std::remove_cvref_t<F>>::replace2nd)>;

    template <class F>
    struct fmap_t : perfect_forward<fmap_op<F>> {
      using perfect_forward<fmap_op<F>>::perfect_forward;
    };

    template <class F>
    struct replace2nd_t : perfect_forward<replace2nd_op<F>> {
      using perfect_forward<replace2nd_op<F>>::perfect_forward;
    };
  } // namespace detail

  inline namespace cpo {
    /// fmap :: (a -> b) -> f a -> f b
    template <class F>
    inline constexpr detail::fmap_t<std::remove_cvref_t<F>> fmap{};

    /// ( $>) :: a -> f b -> f a -- infixl 4
    template <class F>
    inline constexpr detail::replace2nd_t<std::remove_cvref_t<F>> replace2nd{};
  } // namespace cpo

  namespace functors {
    /// @brief ( $>) = fmap . const
    /// @details If you define `functor_traits<F>::fmap`, you can deduce `replace2nd`.
    template <class F>
    requires requires {
      functor_traits<std::remove_cvref_t<F>>::fmap;
    }
    inline constexpr auto replace2nd = compose % mpc::fmap<F> % constant;
  } // namespace functors

  // Grobal methods

  inline namespace cpo {
    /// replace1st :: Functor f => f a -> b -> f b
    /// replace1st = flip replace2nd
    template <functor F>
    inline constexpr auto replace1st = flip % mpc::replace2nd<F>;
  } // namespace cpo
} // namespace mpc
