/// @file state.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/monad.hpp>
#include <mpc/data/functor/identity.hpp>
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/utility/nil.hpp>

namespace mpc {
  // stateT
  // https://hackage.haskell.org/package/transformers-0.6.0.2/docs/Control-Monad-Trans-State-Lazy.html
  // [x] stateT
  // [x] StateT
  // [x] make_stateT
  // [x] run_stateT

  /// newtype StateT s m a = StateT { runStateT :: s -> m (a,s) }
  template <copy_constructible_object Fn, class S>
  requires std::invocable<Fn, S> and monad<std::invoke_result_t<Fn, S>>
  struct stateT : identity<Fn> {
    using identity<Fn>::identity;
  };

  namespace detail {
    template <class>
    struct is_stateT : std::false_type {};

    template <copy_constructible_object Fn, class S>
    struct is_stateT<stateT<Fn, S>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept StateT = detail::is_stateT<std::remove_cvref_t<T>>::value;

  namespace detail {
    template <class S>
    struct make_stateT_op {
      template <copy_constructible_object Fn>
      constexpr auto operator()(Fn&& f) const {
        return stateT<std::decay_t<Fn>, std::decay_t<S>>(std::forward<Fn>(f));
      }
    };

    struct run_stateT_op {
      template <StateT ST>
      constexpr auto operator()(ST&& x) const noexcept -> decltype(*std::forward<ST>(x)) {
        return *std::forward<ST>(x);
      }
    };
  } // namespace detail

  template <class S>
  inline constexpr perfect_forwarded_t<detail::make_stateT_op<S>> make_stateT{};
  inline constexpr perfect_forwarded_t<detail::run_stateT_op> run_stateT{};

  // instances:
  // [x] functor
  // [x] monad
  // [x] applicative
  // [ ] alternative
  // [ ] monad_trans

  // clang-format off

  /// instance (Monad m) => Monad (StateT s m) where
  ///     m >>= k  = StateT $ \ s -> do
  ///         ~(a, s') <- runStateT m s
  ///         runStateT (k a) s'
  template <copy_constructible_object Fn, class S>
  struct monad_traits<stateT<Fn, S>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b -- infixl 1
    struct bind_op {
      struct nested_closure {
        template <class F, class U>
        constexpr auto operator()(F&& f, U&& u) const
        // FIXME
        // noexcept(
        //   noexcept(   run_stateT % std::invoke(std::forward<F>(f), get<0>(std::forward<U>(u)))
        //                          % get<1>(std::forward<U>(u))))
        //   -> decltype(run_stateT % std::invoke(std::forward<F>(f), get<0>(std::forward<U>(u)))
        //                          % get<1>(std::forward<U>(u)))
        {
          auto&& [a, s] = std::forward<U>(u);
          return      run_stateT % std::invoke(std::forward<F>(f), std::forward<decltype(a)>(a))
                                 % std::forward<decltype(s)>(s);
        }
      };

      struct closure {
        // clang-format off
        template <StateT ST, class F, class T>
        constexpr auto operator()(ST&& x, F&& f, T&& t) const noexcept(
          noexcept(
            mpc::bind<decltype(run_stateT % x % t)>(
            run_stateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)))))
          -> decltype(
            mpc::bind<decltype(run_stateT % x % t)>(
            run_stateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)))) {
          return
            mpc::bind<decltype(run_stateT % x % t)>(
            run_stateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)));
        }
        // clang-format on
      };

      template <StateT ST, class F>
      constexpr auto operator()(ST&& x, F&& f) const
        noexcept(noexcept(make_stateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x),
                                                                        std::forward<F>(f)))))
          -> decltype(make_stateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x),
                                                                    std::forward<F>(f)))) {
        return make_stateT<S>(
          perfect_forwarded_t<closure>{}(std::forward<ST>(x), std::forward<F>(f)));
      }
    };

    static constexpr bind_op bind{};
  };

  /// instance (Functor m) => Functor (StateT s m) where
  ///     fmap f m = StateT $ \ s ->
  ///         fmap (\ ~(a, s') -> (f a, s')) $ runStateT m s
  template <copy_constructible_object Fn, class S>
  struct functor_traits<stateT<Fn, S>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      struct nested_closure {
        template <class F, class U>
        constexpr auto operator()(F&& f, U&& u) const
        // FIXME
        // noexcept(
        //   noexcept(   std::make_pair(std::invoke(std::forward<F>(f), get<0>(std::forward<U>(u))),
        //                              get<1>(std::forward<U>(u)))))
        //   -> decltype(std::make_pair(std::invoke(std::forward<F>(f), get<0>(std::forward<U>(u))),
        //                              get<1>(std::forward<U>(u))))
        {
          auto&& [a, s] = std::forward<U>(u);
          return      std::make_pair(std::invoke(std::forward<F>(f), std::forward<decltype(a)>(a)),
                                     std::forward<decltype(s)>(s));
        }
      };

      struct closure {
        template <class F, StateT ST, class T>
        constexpr auto operator()(F&& f, ST&& x, T&& t) const noexcept(
          noexcept(
            mpc::fmap<decltype(run_stateT % x % t)>(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_stateT % std::forward<ST>(x) % std::forward<T>(t))))
          -> decltype(
            mpc::fmap<decltype(run_stateT % x % t)>(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_stateT % std::forward<ST>(x) % std::forward<T>(t))) {
          return
            mpc::fmap<decltype(run_stateT % x % t)>(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_stateT % std::forward<ST>(x) % std::forward<T>(t));
        }
      };

      template <class F, StateT ST>
      constexpr auto operator()(F&& f, ST&& x) const noexcept(
        noexcept(make_stateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)))))
        -> decltype(make_stateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)))) {
        return make_stateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)));
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd<stateT<Fn, S>>;
  };

  /// instance (Functor m, Monad m) => Applicative (StateT s m) where
  ///     pure a = StateT $ \ s -> return (a, s)
  ///     StateT mf <*> StateT mx = StateT $ \ s -> do
  ///         ~(f, s') <- mf s
  ///         ~(x, s'') <- mx s'
  ///         return (f x, s'')
  ///     m *> k = m >>= \_ -> k
  template <copy_constructible_object Fn, class S>
  struct applicative_traits<stateT<Fn, S>> {
    /// pure   :: a -> f a
    struct pure_op {
      struct closure {
        template <class A, class T>
        constexpr auto operator()(A&& a, T&& t) const noexcept(
          noexcept(   returns<std::invoke_result_t<Fn, S>>(std::make_pair(std::forward<A>(a), std::forward<T>(t)))))
          -> decltype(returns<std::invoke_result_t<Fn, S>>(std::make_pair(std::forward<A>(a), std::forward<T>(t)))) {
          return      returns<std::invoke_result_t<Fn, S>>(std::make_pair(std::forward<A>(a), std::forward<T>(t)));
        }
      };

      template <class A>
      constexpr auto operator()(A&& a) const noexcept(
        noexcept(   make_stateT<S>(perfect_forwarded_t<closure>{}(std::forward<A>(a)))))
        -> decltype(make_stateT<S>(perfect_forwarded_t<closure>{}(std::forward<A>(a)))) {
        return      make_stateT<S>(perfect_forwarded_t<closure>{}(std::forward<A>(a)));
      }
    };

    static constexpr pure_op pure{};
    static constexpr auto seq_apply = monads::seq_apply<stateT<Fn, S>>;
    static constexpr auto liftA2 = applicatives::liftA2<stateT<Fn, S>>;
    static constexpr auto discard2nd = applicatives::discard2nd<stateT<Fn, S>>;
    static constexpr auto discard1st = monads::discard1st<stateT<Fn, S>>;
  };
  // clang-format on

  namespace detail {
    // clang-format off
    template <class>
    struct state_op;

    template <copy_constructible_object Fn, class S>
    struct state_op<stateT<Fn, S>> {
      template <copy_constructible_object Fn2>
      constexpr auto operator()(Fn2&& f) const noexcept(
        noexcept(   make_stateT<S>(compose(mpc::returns<std::invoke_result_t<Fn, S>>, std::forward<Fn2>(f)))))
        -> decltype(make_stateT<S>(compose(mpc::returns<std::invoke_result_t<Fn, S>>, std::forward<Fn2>(f)))) {
        return      make_stateT<S>(compose(mpc::returns<std::invoke_result_t<Fn, S>>, std::forward<Fn2>(f)));
      }
    };
  }

  inline namespace cpo {
    template <StateT ST>
    inline constexpr perfect_forwarded_t<detail::state_op<std::remove_cvref_t<ST>>> state{};
  }

  namespace detail{
    template <StateT ST>
    struct put_op {
      struct closure {
        template <class T, class A>
        constexpr auto operator()(T&& t, A&&) const noexcept(
          noexcept(   std::make_pair(nil, std::forward<T>(t))))
          -> decltype(std::make_pair(nil, std::forward<T>(t))) {
          return      std::make_pair(nil, std::forward<T>(t));
        }
      };

      template <class T>
      constexpr auto operator()(T&& t) const noexcept(
        noexcept(   mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<T>(t)))))
        -> decltype(mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<T>(t)))) {
        return      mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<T>(t)));
      }
    };
    // clang-format on
  } // namespace detail

  inline namespace cpo {
    template <StateT ST>
    inline constexpr perfect_forwarded_t<detail::put_op<std::remove_cvref_t<ST>>> put{};

    template <StateT ST>
    inline constexpr auto get1 = mpc::state<ST>([](const auto& t) { return std::make_pair(t, t); });
  } // namespace cpo
} // namespace mpc
