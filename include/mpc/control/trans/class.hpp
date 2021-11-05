/// @file class.hpp
#pragma once
#include <mpc/stdfundamental.hpp>
#include <mpc/functional/perfect_forward.hpp>

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

  namespace detail {
    template <class TR>
    using lift_op =
      std::remove_cvref_t<decltype(monad_trans_traits<std::remove_cvref_t<TR>>::lift)>;

    template <class TR>
    struct lift_t : perfect_forward<lift_op<TR>> {
      using perfect_forward<lift_op<TR>>::perfect_forward;
    };
  } // namespace detail

  inline namespace cpo {
    /// lift :: (Monad m) => m a -> t m a
    template <class TR>
    inline constexpr detail::lift_t<std::remove_cvref_t<TR>> lift{};
  } // namespace cpo
} // namespace mpc
