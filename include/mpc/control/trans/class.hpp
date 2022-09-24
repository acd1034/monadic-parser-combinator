/// @file class.hpp
#pragma once
#include <mpc/functional/partial.hpp>

namespace mpc {
  // monad_trans
  // https://hackage.haskell.org/package/transformers-0.6.0.2/docs/Control-Monad-Trans-Class.html

  /// class (forall m. Monad m => Monad (t m)) => MonadTrans t where
  template <class>
  struct monad_trans_traits;

  template <class TR>
  concept monad_trans = requires {
    monad_trans_traits<std::remove_cvref_t<TR>>::lift;
  };

  // Methods required for the class definition.

  namespace detail {
    /// lift :: (Monad m) => m a -> t m a
    template <class TR>
    struct lift_op {
      template <class M>
      constexpr auto operator()(M&& m) const noexcept(
      noexcept(   monad_trans_traits<std::remove_cvref_t<TR>>::lift(std::forward<M>(m))))
      -> decltype(monad_trans_traits<std::remove_cvref_t<TR>>::lift(std::forward<M>(m)))
      { return    monad_trans_traits<std::remove_cvref_t<TR>>::lift(std::forward<M>(m)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// lift :: (Monad m) => m a -> t m a
    template <class TR>
    inline constexpr partial<detail::lift_op<TR>> lift{};
  } // namespace cpo
} // namespace mpc
