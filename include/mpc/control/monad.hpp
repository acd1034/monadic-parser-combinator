/// @file monad.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/applicative.hpp>
#include <mpc/functional/partial.hpp>

// clang-format off

namespace mpc {
  // monad
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html#t:Monad

  /// class Applicative m => Monad m where
  template <class>
  struct monad_traits;

  /// monad_traits_specialized
  template <class M>
  concept monad_traits_specialized = requires {
    monad_traits<std::remove_cvref_t<M>>::bind;
  };

  /// Requires applicative and bind is valid in @link mpc::monad_traits monad_traits @endlink.
  template <class M>
  concept monad = applicative<M> and monad_traits_specialized<M>;

  // Methods required for the class definition.

  namespace detail {
    /**
     * @brief bind :: forall a b. m a -> (a -> m b) -> m b
     * @details (>>=) in Haskell
     */
    struct bind_op {
      template <class Ma, class Fn>
      constexpr auto operator()(Ma&& ma, Fn&& fn) const noexcept(
      noexcept(   monad_traits<std::remove_cvref_t<Ma>>::bind(std::forward<Ma>(ma), std::forward<Fn>(fn))))
      -> decltype(monad_traits<std::remove_cvref_t<Ma>>::bind(std::forward<Ma>(ma), std::forward<Fn>(fn)))
      { return    monad_traits<std::remove_cvref_t<Ma>>::bind(std::forward<Ma>(ma), std::forward<Fn>(fn)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// @copydoc mpc::detail::bind_op
    inline constexpr partial<detail::bind_op> bind{};
  } // namespace cpo

  /// Methods deducible from other methods of @link mpc::monad monad @endlink.
  namespace monads {
    namespace detail {
      /**
       * @copydoc mpc::detail::fmap_op
       * ```
       * fmap f xs = xs `bind` (returns . f)
       * ```
       */
      struct fmap_op {
        template <class Fn, class Ma>
        constexpr auto operator()(Fn&& fn, Ma&& ma) const noexcept(
        noexcept(   mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn)))))
        -> decltype(mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn))))
        { return    mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn))); }
      };

      /**
       * @copydoc mpc::detail::seq_apply_op
       * ```
       * seq_apply mf xs = mf `bind` (\f -> xs `bind` (returns . f))
       * ```
       */
      struct seq_apply_op {
        struct closure {
          template<class Ma, class Fn>
          constexpr auto operator()(Ma&& ma, Fn&& fn) const noexcept(
          noexcept(   mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn)))))
          -> decltype(mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn))))
          { return    mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn))); }
        };

        template<class Mab, class Ma>
        constexpr auto operator()(Mab&& mab, Ma&& ma) const noexcept(
        noexcept(   mpc::bind(std::forward<Mab>(mab), partial(closure{}, std::forward<Ma>(ma)))))
        -> decltype(mpc::bind(std::forward<Mab>(mab), partial(closure{}, std::forward<Ma>(ma))))
        { return    mpc::bind(std::forward<Mab>(mab), partial(closure{}, std::forward<Ma>(ma))); }
      };

      /**
       * @copydoc mpc::detail::discard1st_op
       * ```
       * discard1st m1 m2 = m1 `bind` (constant m2)
       * ```
       */
      struct discard1st_op {
        template<class Ma, class Mb>
        constexpr auto operator()(Ma&& ma, Mb&& mb) const noexcept(
        noexcept(   mpc::bind(std::forward<Ma>(ma), constant % std::forward<Mb>(mb))))
        -> decltype(mpc::bind(std::forward<Ma>(ma), constant % std::forward<Mb>(mb)))
        { return    mpc::bind(std::forward<Ma>(ma), constant % std::forward<Mb>(mb)); }
      };
    } // namespace detail

    /// @copydoc mpc::monads::detail::fmap_op
    inline constexpr partial<detail::fmap_op> fmap{};

    /// @copydoc mpc::monads::detail::seq_apply_op
    inline constexpr partial<detail::seq_apply_op> seq_apply{};

    /// @copydoc mpc::monads::detail::discard1st_op
    inline constexpr partial<detail::discard1st_op> discard1st{};
  } // namespace monads

  // Grobal methods

  namespace detail {
    /**
     * @brief karrow :: Monad m => (a -> m b) -> (b -> m c) -> (a -> m c)
     * @details (>=>) in Haskell
     * ```
     * karrow f g x = f x `bind` g
     * ```
     */
    struct karrow_op {
      template <class Fn, class Gn, class A>
      constexpr auto operator()(Fn&& fn, Gn&& gn, A&& a) const noexcept(
        noexcept(   mpc::bind(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)), std::forward<Gn>(gn))))
        -> decltype(mpc::bind(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)), std::forward<Gn>(gn)))
        { return    mpc::bind(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)), std::forward<Gn>(gn)); }
    };
  } // namespace detail

  inline namespace cpo {
    /**
     * @brief returns :: a -> m a
     * @details
     * ```
     * returns = pure
     * ```
     */
    template <class M>
    requires requires {
      applicative_traits<std::remove_cvref_t<M>>::pure;
    }
    inline constexpr auto returns = mpc::pure<M>;

    /// @copydoc mpc::detail::karrow_op
    inline constexpr partial<detail::karrow_op> karrow{};
  } // namespace cpo
} // namespace mpc

// clang-format on
