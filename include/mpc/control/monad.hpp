/// @file monad.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/basic_traits.hpp>
#include <mpc/control/applicative.hpp>
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  namespace detail {
    template <class M>
    using bind_op = std::remove_cvref_t<decltype(monad_traits<std::remove_cvref_t<M>>::bind)>;

    template <class M>
    struct bind_t : perfect_forward<bind_op<M>> {
      using perfect_forward<bind_op<M>>::perfect_forward;
    };
  } // namespace detail

  inline namespace cpo {
    /// return = pure
    /// A mere alias for a method of Functor
    template <class M>
    requires requires {
      applicative_traits<std::remove_cvref_t<M>>::pure;
    }
    inline constexpr auto returns = mpc::pure<M>;

    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b -- infixl 1
    template <class M>
    inline constexpr detail::bind_t<std::remove_cvref_t<M>> bind{};
  } // namespace cpo

  namespace monads {
    namespace detail {
      // clang-format off
      /// fmap :: (a -> b) -> f a -> f b
      /// fmap f x = bind x (y -> return (f y))
      template<monad_traits_specialized M>
      requires requires { applicative_traits<std::remove_cvref_t<M>>::pure; }
      struct fmap_op {
        struct closure {
          template<class A1, class B1>
          constexpr auto operator()(A1&& f, B1&& y) const
          noexcept(noexcept(mpc::returns<M>(std::invoke(std::forward<A1>(f), std::forward<B1>(y)))))
          -> decltype(      mpc::returns<M>(std::invoke(std::forward<A1>(f), std::forward<B1>(y))))
          { return          mpc::returns<M>(std::invoke(std::forward<A1>(f), std::forward<B1>(y))); }
        };

        template<class A1, class A2>
        constexpr auto operator()(A1&& f, A2&& x) const
        noexcept(noexcept(mpc::bind<M>(std::forward<A2>(x), perfect_forwarded_t<closure>{}(std::forward<A1>(f)))))
        -> decltype(      mpc::bind<M>(std::forward<A2>(x), perfect_forwarded_t<closure>{}(std::forward<A1>(f))))
        { return          mpc::bind<M>(std::forward<A2>(x), perfect_forwarded_t<closure>{}(std::forward<A1>(f))); }
      };

      /// seq_apply :: f (a -> b) -> f a -> f b
      /// seq_apply m1 m2 = bind m1 (f -> bind m2 (x -> return (f x)))
      template<monad_traits_specialized M>
      requires requires { applicative_traits<std::remove_cvref_t<M>>::pure; }
      struct seq_apply_op {
        struct nested_closure {
          template<class B1, class B2>
          constexpr auto operator()(B1&& f, B2&& x) const
          noexcept(noexcept(mpc::returns<M>(std::invoke(std::forward<B1>(f), std::forward<B2>(x)))))
          -> decltype(      mpc::returns<M>(std::invoke(std::forward<B1>(f), std::forward<B2>(x))))
          { return          mpc::returns<M>(std::invoke(std::forward<B1>(f), std::forward<B2>(x))); }
        };

        struct closure {
          template<class A2, class B1>
          constexpr auto operator()(A2&& m2, B1&& f) const
          noexcept(noexcept(mpc::bind<M>(std::forward<A2>(m2), perfect_forwarded_t<nested_closure>{}(std::forward<B1>(f)))))
          -> decltype(      mpc::bind<M>(std::forward<A2>(m2), perfect_forwarded_t<nested_closure>{}(std::forward<B1>(f))))
          { return          mpc::bind<M>(std::forward<A2>(m2), perfect_forwarded_t<nested_closure>{}(std::forward<B1>(f))); }
        };

        template<class A1, class A2>
        constexpr auto operator()(A1&& m1, A2&& m2) const
        noexcept(noexcept(mpc::bind<M>(std::forward<A1>(m1), perfect_forwarded_t<closure>{}(std::forward<A2>(m2)))))
        -> decltype(      mpc::bind<M>(std::forward<A1>(m1), perfect_forwarded_t<closure>{}(std::forward<A2>(m2))))
        { return          mpc::bind<M>(std::forward<A1>(m1), perfect_forwarded_t<closure>{}(std::forward<A2>(m2))); }
      };

      /// discard1st :: f a -> f b -> f b
      /// discard1st m1 m2 = bind m1 (_ -> m2)
      template<monad_traits_specialized M>
      struct discard1st_op {
        struct closure {
          template<class A2, class B1>
          constexpr auto operator()(A2&& m2, B1&&) const
          noexcept(noexcept(std::forward<A2>(m2)))
          -> decltype(      std::forward<A2>(m2))
          { return          std::forward<A2>(m2); }
        };

        template<class A1, class A2>
        constexpr auto operator()(A1&& m1, A2&& m2) const
        noexcept(noexcept(mpc::bind<M>(std::forward<A1>(m1), perfect_forwarded_t<closure>{}(std::forward<A2>(m2)))))
        -> decltype(      mpc::bind<M>(std::forward<A1>(m1), perfect_forwarded_t<closure>{}(std::forward<A2>(m2))))
        { return          mpc::bind<M>(std::forward<A1>(m1), perfect_forwarded_t<closure>{}(std::forward<A2>(m2))); }
      };

      template <monad_traits_specialized M>
      requires requires { applicative_traits<std::remove_cvref_t<M>>::pure; }
      struct fmap_t : perfect_forward<fmap_op<M>> {
        using perfect_forward<fmap_op<M>>::perfect_forward;
      };

      template <monad_traits_specialized M>
      requires requires { applicative_traits<std::remove_cvref_t<M>>::pure; }
      struct seq_apply_t : perfect_forward<seq_apply_op<M>> {
        using perfect_forward<seq_apply_op<M>>::perfect_forward;
      };

      template <monad_traits_specialized M>
      struct discard1st_t : perfect_forward<discard1st_op<M>> {
        using perfect_forward<discard1st_op<M>>::perfect_forward;
      };
      // clang-format on
    } // namespace detail

    /// @brief fmap f x = bind x (y -> return (f y))
    /// @details If you fully specialize `monad_traits<M>`, you can deduce `fmap`.
    template <monad_traits_specialized M>
    requires requires {
      applicative_traits<std::remove_cvref_t<M>>::pure;
    }
    inline constexpr detail::fmap_t<std::remove_cvref_t<M>> fmap{};

    /// @brief seq_apply m1 m2 = bind m1 (f -> bind m2 (x -> return (f x)))
    /// @details If you fully specialize `monad_traits<M>`, you can deduce `seq_apply`.
    template <monad_traits_specialized M>
    requires requires {
      applicative_traits<std::remove_cvref_t<M>>::pure;
    }
    inline constexpr detail::seq_apply_t<std::remove_cvref_t<M>> seq_apply{};

    /// @brief discard1st m1 m2 = bind m1 (_ -> m2)
    /// @details If you fully specialize `monad_traits<M>`, you can deduce `discard1st`.
    template <monad_traits_specialized M>
    inline constexpr detail::discard1st_t<std::remove_cvref_t<M>> discard1st{};
  } // namespace monads

  // Grobal methods

  namespace detail {
    // clang-format off
    /// karrow :: Monad m => (a -> m b) -> (b -> m c) -> (a -> m c) -- infixr 1
    /// karrow f g = \x -> f x >>= g
    template<monad M>
    struct karrow_op {
      struct closure {
        template<class A1, class A2, class B1>
        constexpr auto operator()(A1&& f, A2&& g, B1&& x) const
        noexcept(noexcept(mpc::bind<M>(std::invoke(std::forward<A1>(f), std::forward<B1>(x)), std::forward<A2>(g))))
        -> decltype(      mpc::bind<M>(std::invoke(std::forward<A1>(f), std::forward<B1>(x)), std::forward<A2>(g)))
        { return          mpc::bind<M>(std::invoke(std::forward<A1>(f), std::forward<B1>(x)), std::forward<A2>(g)); }
      };

      template<class A1, class A2>
      constexpr auto operator()(A1&& f, A2&& g) const
      noexcept(noexcept(perfect_forwarded_t<closure>{}(std::forward<A1>(f), std::forward<A2>(g))))
      -> decltype(      perfect_forwarded_t<closure>{}(std::forward<A1>(f), std::forward<A2>(g)))
      { return          perfect_forwarded_t<closure>{}(std::forward<A1>(f), std::forward<A2>(g)); }
    };
    // clang-format on
  } // namespace detail

  inline namespace cpo {
    /// Kleisli arrows (>=>)
    template <monad F>
    inline constexpr perfect_forwarded_t<detail::karrow_op<std::remove_cvref_t<F>>> karrow{};
  } // namespace cpo
} // namespace mpc
