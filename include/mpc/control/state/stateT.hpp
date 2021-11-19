/// @file stateT.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/alternative.hpp>
#include <mpc/control/monad.hpp>
#include <mpc/control/state/class.hpp>
#include <mpc/control/trans/class.hpp>
#include <mpc/data/functor/identity.hpp>
#include <mpc/prelude/compose.hpp>
#include <mpc/prelude/fst.hpp>

// clang-format off

namespace mpc {
  // StateT
  // https://hackage.haskell.org/package/transformers-0.6.0.2/docs/Control-Monad-Trans-State-Lazy.html
  // [x] StateT
  // [x] is_StateT
  // [x] make_StateT
  // [x] run_StateT
  // [x] StateT_state_t
  // [x] StateT_monad_t

  /// newtype StateT s m a = StateT { run_StateT :: s -> m (a,s) }
  template <copy_constructible_object Fn, class S>
  requires std::invocable<Fn, S> and monad<std::invoke_result_t<Fn, S>>
  struct StateT : Identity<Fn> {
    using Identity<Fn>::Identity;
    using state_type = S;
    using monad_type = std::invoke_result_t<Fn, S>;
  };

  // is_StateT
  namespace detail {
    template <class>
    struct is_StateT_impl : std::false_type {};

    template <copy_constructible_object Fn, class S>
    struct is_StateT_impl<StateT<Fn, S>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept is_StateT = detail::is_StateT_impl<std::remove_cvref_t<T>>::value;

  template <is_StateT ST>
  using StateT_state_t = typename std::remove_cvref_t<ST>::state_type;

  template <is_StateT ST>
  using StateT_monad_t = typename std::remove_cvref_t<ST>::monad_type;

  // make_StateT, run_StateT
  namespace detail {
    template <class S>
    struct make_StateT_op {
      template <copy_constructible_object Fn>
      constexpr auto operator()(Fn&& f) const {
        return StateT<std::decay_t<Fn>, std::decay_t<S>>(std::forward<Fn>(f));
      }
    };

    struct run_StateT_op {
      template <is_StateT ST>
      constexpr auto operator()(ST&& x) const noexcept -> decltype(*std::forward<ST>(x)) {
        return *std::forward<ST>(x);
      }
    };
  } // namespace detail

  namespace cpo {
    template <class S>
    inline constexpr perfect_forwarded_t<detail::make_StateT_op<S>> make_StateT{};

    inline constexpr perfect_forwarded_t<detail::run_StateT_op> run_StateT{};
  } // namespace cpo

  // instances:
  // [x] functor
  // [x] monad
  // [x] applicative
  // [x] alternative
  // [x] monad_trans

  /// instance (Monad m) => Monad (StateT s m) where
  ///     m >>= k  = StateT $ \ s -> do
  ///         ~(a, s') <- run_StateT m s
  ///         run_StateT (k a) s'
  template <copy_constructible_object Fn, class S>
  struct monad_traits<StateT<Fn, S>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b -- infixl 1
    struct bind_op {
      struct nested_closure {
        template <class F, class U>
        constexpr auto operator()(F&& f, U&& u) const
        // FIXME
        // noexcept(
        //   noexcept())
        //   -> decltype()
        {
          auto&& [a, s] = std::forward<U>(u);
          return run_StateT % std::invoke(std::forward<F>(f), std::forward<decltype(a)>(a)) % std::forward<decltype(s)>(s);
        }
      };

      struct closure {
        template <is_StateT ST, class F, class T>
        constexpr auto operator()(ST&& x, F&& f, T&& t) const noexcept(
          noexcept(
            mpc::bind(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)))))
          -> decltype(
            mpc::bind(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)))) {
          return
            mpc::bind(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)));
        }
      };

      template <is_StateT ST, class F>
      constexpr auto operator()(ST&& x, F&& f) const noexcept(
          noexcept(   make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x), std::forward<F>(f)))))
          -> decltype(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x), std::forward<F>(f)))) {
        return        make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x), std::forward<F>(f)));
      }
    };

    static constexpr bind_op bind{};
  };

  /// instance (Functor m) => Functor (StateT s m) where
  ///     fmap f m = StateT $ \ s ->
  ///         fmap (\ ~(a, s') -> (f a, s')) $ run_StateT m s
  template <copy_constructible_object Fn, class S>
  struct functor_traits<StateT<Fn, S>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      struct nested_closure {
        template <class F, class U>
        constexpr auto operator()(F&& f, U&& u) const
        // FIXME
        // noexcept(
        //   noexcept())
        //   -> decltype()
        {
          auto&& [a, s] = std::forward<U>(u);
          return std::make_pair(std::invoke(std::forward<F>(f), std::forward<decltype(a)>(a)), std::forward<decltype(s)>(s));
        }
      };

      struct closure {
        template <class F, is_StateT ST, class T>
        constexpr auto operator()(F&& f, ST&& x, T&& t) const noexcept(
          noexcept(
            mpc::fmap(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
          -> decltype(
            mpc::fmap(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
          return
            mpc::fmap(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t));
        }
      };

      template <class F, is_StateT ST>
      constexpr auto operator()(F&& f, ST&& x) const noexcept(
          noexcept(   make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)))))
          -> decltype(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)))) {
        return        make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)));
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd;
  };

  /// instance (Functor m, Monad m) => Applicative (StateT s m) where
  ///     pure a = StateT $ \ s -> return (a, s)
  ///     StateT mf <*> StateT mx = StateT $ \ s -> do
  ///         ~(f, s') <- mf s
  ///         ~(x, s'') <- mx s'
  ///         return (f x, s'')
  ///     m *> k = m >>= \_ -> k
  template <copy_constructible_object Fn, class S>
  struct applicative_traits<StateT<Fn, S>> {
    /// pure   :: a -> f a
    struct pure_op {
      struct closure {
        template <class A, class T>
        constexpr auto operator()(A&& a, T&& t) const noexcept(
          noexcept(   returns<StateT_monad_t<StateT<Fn, S>>>(std::make_pair(std::forward<A>(a), std::forward<T>(t)))))
          -> decltype(returns<StateT_monad_t<StateT<Fn, S>>>(std::make_pair(std::forward<A>(a), std::forward<T>(t)))) {
          return      returns<StateT_monad_t<StateT<Fn, S>>>(std::make_pair(std::forward<A>(a), std::forward<T>(t)));
        }
      };

      template <class A>
      constexpr auto operator()(A&& a) const noexcept(
        noexcept(   make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<A>(a)))))
        -> decltype(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<A>(a)))) {
        return      make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<A>(a)));
      }
    };

    static constexpr pure_op pure{};
    static constexpr auto seq_apply = monads::seq_apply;
    static constexpr auto liftA2 = applicatives::liftA2;
    static constexpr auto discard2nd = applicatives::discard2nd;
    static constexpr auto discard1st = monads::discard1st;
  };

  /// instance (Functor m, MonadPlus m) => Alternative (StateT s m) where
  ///     empty = StateT $ \ _ -> mzero
  ///     StateT m <|> StateT n = StateT $ \ s -> m s `mplus` n s
  namespace detail {
    template <class ST>
    struct StateT_alternative_traits_empty {};

    template <is_StateT ST>
    requires has_alternative_traits_empty<StateT_monad_t<ST>>
    struct StateT_alternative_traits_empty<ST> {
      static constexpr auto empty = make_StateT<StateT_state_t<ST>>([](auto&&) { return mpc::empty<StateT_monad_t<ST>>; });
    };

    template <class ST>
    struct StateT_alternative_traits_combine {};

    template <is_StateT ST>
    requires has_alternative_traits_combine<StateT_monad_t<ST>>
    struct StateT_alternative_traits_combine<ST> {
      struct combine_op {
        struct closure {
          template <is_StateT ST1, is_StateT ST2, class T>
          constexpr auto operator()(ST1&& x, ST2&& y, const T& t) const
            noexcept(noexcept(mpc::combine(run_StateT % std::forward<ST1>(x) % t, run_StateT % std::forward<ST2>(y) % t)))
            -> decltype(      mpc::combine(run_StateT % std::forward<ST1>(x) % t, run_StateT % std::forward<ST2>(y) % t)) {
            return            mpc::combine(run_StateT % std::forward<ST1>(x) % t, run_StateT % std::forward<ST2>(y) % t);
          }
        };

        template <is_StateT ST1, is_StateT ST2>
        constexpr auto operator()(ST1&& x, ST2&& y) const
          noexcept(noexcept(make_StateT<StateT_state_t<ST>>(perfect_forwarded_t<closure>{}(std::forward<ST1>(x), std::forward<ST2>(y)))))
          -> decltype(      make_StateT<StateT_state_t<ST>>(perfect_forwarded_t<closure>{}(std::forward<ST1>(x), std::forward<ST2>(y)))) {
          return            make_StateT<StateT_state_t<ST>>(perfect_forwarded_t<closure>{}(std::forward<ST1>(x), std::forward<ST2>(y)));
        }
      };

      static constexpr combine_op combine{};
    };
  } // namespace detail

  template <copy_constructible_object Fn, class S>
  struct alternative_traits<StateT<Fn, S>>
    : detail::StateT_alternative_traits_empty<StateT<Fn, S>>,
      detail::StateT_alternative_traits_combine<StateT<Fn, S>> {};

  /// instance MonadTrans (StateT s) where
  ///     lift m = StateT $ \ s -> do
  ///         a <- m
  ///         return (a, s)
  template <copy_constructible_object Fn, class S>
  struct monad_trans_traits<StateT<Fn, S>> {
    /// lift :: (Monad m) => m a -> t m a
    struct lift_op {
      struct nested_closure {
        template <class T, class A>
        constexpr auto operator()(T&& t, A&& a) const
          noexcept(noexcept(std::make_pair(std::forward<A>(a), std::forward<T>(t))))
          -> decltype(      std::make_pair(std::forward<A>(a), std::forward<T>(t))) {
          return            std::make_pair(std::forward<A>(a), std::forward<T>(t));
        }
      };

      struct closure {
        template <monad M, class T>
        constexpr auto operator()(M&& m, T&& t) const
          noexcept(noexcept(mpc::fmap(perfect_forwarded_t<nested_closure>{}(std::forward<T>(t)), std::forward<M>(m))))
          -> decltype(      mpc::fmap(perfect_forwarded_t<nested_closure>{}(std::forward<T>(t)), std::forward<M>(m))) {
          return            mpc::fmap(perfect_forwarded_t<nested_closure>{}(std::forward<T>(t)), std::forward<M>(m));
        }
      };

      template <monad M>
      constexpr auto operator()(M&& m) const
        noexcept(noexcept(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<M>(m)))))
        -> decltype(      make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<M>(m)))) {
        return            make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<M>(m)));
      }
    };

    static constexpr lift_op lift{};
  };

  /// instance MonadState (StateT s) where
  ///     state f = StateT (return . f)
  template <copy_constructible_object Fn, class S>
  struct monad_state_traits<StateT<Fn, S>> {
    struct state_op {
      template <copy_constructible_object Fn2>
      constexpr auto operator()(Fn2&& f) const noexcept(
        noexcept(   make_StateT<S>(compose(mpc::returns<StateT_monad_t<StateT<Fn, S>>>, std::forward<Fn2>(f)))))
        -> decltype(make_StateT<S>(compose(mpc::returns<StateT_monad_t<StateT<Fn, S>>>, std::forward<Fn2>(f)))) {
        return      make_StateT<S>(compose(mpc::returns<StateT_monad_t<StateT<Fn, S>>>, std::forward<Fn2>(f)));
      }
    };

    static constexpr state_op state{};
    static constexpr auto get1 = states::get1<StateT<Fn, S>>;
    static constexpr auto put = states::put<StateT<Fn, S>>;
  };

  // Grobal methods:
  // [x] eval_StateT
  // [x] exec_StateT
  // [x] map_StateT
  // [x] with_StateT

  // eval_StateT, exec_StateT, map_StateT, with_StateT
  namespace detail {
    struct eval_StateT_op {
      template <is_StateT ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   mpc::fmap(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(mpc::fmap(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
        return      mpc::fmap(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct exec_StateT_op {
      template <is_StateT ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   mpc::fmap(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(mpc::fmap(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
        return      mpc::fmap(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct map_StateT_op {
      template <class Fn2, is_StateT ST>
      constexpr auto operator()(Fn2&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn2>(f), run_StateT % std::forward<ST>(x)))))
        -> decltype(make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn2>(f), run_StateT % std::forward<ST>(x)))) {
        return      make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn2>(f), run_StateT % std::forward<ST>(x)));
      }
    };

    struct with_StateT_op {
      template <class Fn2, is_StateT ST>
      constexpr auto operator()(Fn2&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn2>(f)))))
        -> decltype(make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn2>(f)))) {
        return      make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn2>(f)));
      }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr perfect_forwarded_t<detail::eval_StateT_op> eval_StateT{};

    inline constexpr perfect_forwarded_t<detail::exec_StateT_op> exec_StateT{};

    inline constexpr perfect_forwarded_t<detail::map_StateT_op> map_StateT{};

    inline constexpr perfect_forwarded_t<detail::with_StateT_op> with_StateT{};
  } // namespace cpo
} // namespace mpc

// clang-format on
