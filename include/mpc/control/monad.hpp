/// @file monad.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/applicative.hpp>
#include <mpc/functional/perfect_forward.hpp>

// clang-format off

namespace mpc {
  // monad
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html

  /// class Applicative m => Monad m where
  template <class>
  struct monad_traits;

  template <class M>
  concept monad_traits_specialized = requires {
    monad_traits<std::remove_cvref_t<M>>::bind;
  };

  template <class M>
  concept monad = applicative<M> and monad_traits_specialized<M>;

  // class requirements

  namespace detail {
    /// bind :: forall a b. m a -> (a -> m b) -> m b
    struct bind_op {
      template <class Ma, class Fn>
      constexpr auto operator()(Ma&& ma, Fn&& fn) const noexcept(
      noexcept(   monad_traits<std::remove_cvref_t<Ma>>::bind(std::forward<Ma>(ma), std::forward<Fn>(fn))))
      -> decltype(monad_traits<std::remove_cvref_t<Ma>>::bind(std::forward<Ma>(ma), std::forward<Fn>(fn)))
      { return    monad_traits<std::remove_cvref_t<Ma>>::bind(std::forward<Ma>(ma), std::forward<Fn>(fn)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// @brief return = pure
    /// @details A mere alias for a method of Applicative
    template <class M>
    requires requires {
      applicative_traits<std::remove_cvref_t<M>>::pure;
    }
    inline constexpr auto returns = mpc::pure<M>;

    /// bind :: forall a b. m a -> (a -> m b) -> m b
    inline constexpr perfect_forwarded_t<detail::bind_op> bind{};
  } // namespace cpo

  // Deducibles

  namespace monads {
    namespace detail {
      /// fmap :: (a -> b) -> f a -> f b
      /// fmap f x = bind x (y -> return (f y))
      struct fmap_op {
        template <class Ma>
        struct closure {
          template <class Fn, class A>
          constexpr auto operator()(Fn&& fn, A&& a) const noexcept(
          noexcept(   mpc::returns<Ma>(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)))))
          -> decltype(mpc::returns<Ma>(std::invoke(std::forward<Fn>(fn), std::forward<A>(a))))
          { return    mpc::returns<Ma>(std::invoke(std::forward<Fn>(fn), std::forward<A>(a))); }
        };

        template <class Fn, class Ma>
        constexpr auto operator()(Fn&& fn, Ma&& ma) const noexcept(
        noexcept(   mpc::bind(std::forward<Ma>(ma), perfect_forwarded_t<closure<Ma>>{}(std::forward<Fn>(fn)))))
        -> decltype(mpc::bind(std::forward<Ma>(ma), perfect_forwarded_t<closure<Ma>>{}(std::forward<Fn>(fn))))
        { return    mpc::bind(std::forward<Ma>(ma), perfect_forwarded_t<closure<Ma>>{}(std::forward<Fn>(fn))); }
      };

      /// seq_apply :: f (a -> b) -> f a -> f b
      /// seq_apply m1 m2 = bind m1 (f -> bind m2 (x -> return (f x)))
      struct seq_apply_op {
        template <class Ma>
        struct nested_closure {
          template<class Fn, class A>
          constexpr auto operator()(Fn&& fn, A&& a) const noexcept(
          noexcept(   mpc::returns<Ma>(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)))))
          -> decltype(mpc::returns<Ma>(std::invoke(std::forward<Fn>(fn), std::forward<A>(a))))
          { return    mpc::returns<Ma>(std::invoke(std::forward<Fn>(fn), std::forward<A>(a))); }
        };

        struct closure {
          template<class Ma, class Fn>
          constexpr auto operator()(Ma&& ma, Fn&& fn) const noexcept(
          noexcept(   mpc::bind(std::forward<Ma>(ma), perfect_forwarded_t<nested_closure<Ma>>{}(std::forward<Fn>(fn)))))
          -> decltype(mpc::bind(std::forward<Ma>(ma), perfect_forwarded_t<nested_closure<Ma>>{}(std::forward<Fn>(fn))))
          { return    mpc::bind(std::forward<Ma>(ma), perfect_forwarded_t<nested_closure<Ma>>{}(std::forward<Fn>(fn))); }
        };

        template<class Mab, class Ma>
        constexpr auto operator()(Mab&& mab, Ma&& ma) const noexcept(
        noexcept(   mpc::bind(std::forward<Mab>(mab), perfect_forwarded_t<closure>{}(std::forward<Ma>(ma)))))
        -> decltype(mpc::bind(std::forward<Mab>(mab), perfect_forwarded_t<closure>{}(std::forward<Ma>(ma))))
        { return    mpc::bind(std::forward<Mab>(mab), perfect_forwarded_t<closure>{}(std::forward<Ma>(ma))); }
      };

      /// discard1st :: f a -> f b -> f b
      /// discard1st m1 m2 = bind m1 (_ -> m2)
      struct discard1st_op {
        struct closure {
          template<class Mb, class A>
          constexpr auto operator()(Mb&& mb, A&&) const noexcept(
          noexcept(   std::forward<Mb>(mb)))
          -> decltype(std::forward<Mb>(mb))
          { return    std::forward<Mb>(mb); }
        };

        template<class Ma, class Mb>
        constexpr auto operator()(Ma&& ma, Mb&& mb) const noexcept(
        noexcept(   mpc::bind(std::forward<Ma>(ma), perfect_forwarded_t<closure>{}(std::forward<Mb>(mb)))))
        -> decltype(mpc::bind(std::forward<Ma>(ma), perfect_forwarded_t<closure>{}(std::forward<Mb>(mb))))
        { return    mpc::bind(std::forward<Ma>(ma), perfect_forwarded_t<closure>{}(std::forward<Mb>(mb))); }
      };
    } // namespace detail

    /// @brief fmap f x = bind x (y -> return (f y))
    /// @details If you fully specialize `monad_traits<M>`, you can deduce `fmap`.
    inline constexpr perfect_forwarded_t<detail::fmap_op> fmap{};

    /// @brief seq_apply m1 m2 = bind m1 (f -> bind m2 (x -> return (f x)))
    /// @details If you fully specialize `monad_traits<M>`, you can deduce `seq_apply`.
    inline constexpr perfect_forwarded_t<detail::seq_apply_op> seq_apply{};

    /// @brief discard1st m1 m2 = bind m1 (_ -> m2)
    /// @details If you fully specialize `monad_traits<M>`, you can deduce `discard1st`.
    inline constexpr perfect_forwarded_t<detail::discard1st_op> discard1st{};
  } // namespace monads

  // Grobal methods

  namespace detail {
    /// karrow :: Monad m => (a -> m b) -> (b -> m c) -> (a -> m c)
    /// karrow f g = \x -> f x >>= g
    template <monad M>
    struct karrow_op {
      struct closure {
        template <class Fn, class Gn, class A>
        constexpr auto operator()(Fn&& fn, Gn&& gn, A&& a) const noexcept(
        noexcept(mpc::bind(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)), std::forward<Gn>(gn))))
        -> decltype(      mpc::bind(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)), std::forward<Gn>(gn)))
        { return          mpc::bind(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)), std::forward<Gn>(gn)); }
      };

      template <class Fn, class Gn>
      constexpr auto operator()(Fn&& fn, Gn&& gn) const noexcept(
      noexcept(perfect_forwarded_t<closure>{}(std::forward<Fn>(fn), std::forward<Gn>(gn))))
      -> decltype(      perfect_forwarded_t<closure>{}(std::forward<Fn>(fn), std::forward<Gn>(gn)))
      { return          perfect_forwarded_t<closure>{}(std::forward<Fn>(fn), std::forward<Gn>(gn)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// Kleisli arrows (>=>)
    template <monad F>
    inline constexpr perfect_forwarded_t<detail::karrow_op<std::remove_cvref_t<F>>> karrow{};
  } // namespace cpo
} // namespace mpc

// clang-format on
