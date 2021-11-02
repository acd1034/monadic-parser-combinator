/// @file state.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/alternative.hpp>
#include <mpc/control/monad.hpp>
#include <mpc/control/trans/class.hpp>
#include <mpc/data/functor/identity.hpp>
#include <mpc/functional/fst.hpp>
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/utility/nil.hpp>

namespace mpc {
  // StateT
  // https://hackage.haskell.org/package/transformers-0.6.0.2/docs/Control-Monad-Trans-State-Lazy.html
  // [x] StateT
  // [x] isStateT
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

  // isStateT
  namespace detail {
    template <class>
    struct is_StateT : std::false_type {};

    template <copy_constructible_object Fn, class S>
    struct is_StateT<StateT<Fn, S>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept isStateT = detail::is_StateT<std::remove_cvref_t<T>>::value;

  template<isStateT ST>
  using StateT_state_t = typename std::remove_cvref_t<ST>::state_type;

  template<isStateT ST>
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
      template <isStateT ST>
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

  // clang-format off

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
        template <isStateT ST, class F, class T>
        constexpr auto operator()(ST&& x, F&& f, T&& t) const noexcept(
          noexcept(
            mpc::bind<StateT_monad_t<ST>>(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)))))
          -> decltype(
            mpc::bind<StateT_monad_t<ST>>(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)))) {
          return
            mpc::bind<StateT_monad_t<ST>>(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)));
        }
      };

      template <isStateT ST, class F>
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
        template <class F, isStateT ST, class T>
        constexpr auto operator()(F&& f, ST&& x, T&& t) const noexcept(
          noexcept(
            mpc::fmap<StateT_monad_t<ST>>(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
          -> decltype(
            mpc::fmap<StateT_monad_t<ST>>(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
          return
            mpc::fmap<StateT_monad_t<ST>>(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t));
        }
      };

      template <class F, isStateT ST>
      constexpr auto operator()(F&& f, ST&& x) const noexcept(
          noexcept(   make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)))))
          -> decltype(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)))) {
        return        make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)));
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd<StateT<Fn, S>>;
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
    static constexpr auto seq_apply = monads::seq_apply<StateT<Fn, S>>;
    static constexpr auto liftA2 = applicatives::liftA2<StateT<Fn, S>>;
    static constexpr auto discard2nd = applicatives::discard2nd<StateT<Fn, S>>;
    static constexpr auto discard1st = monads::discard1st<StateT<Fn, S>>;
  };

  /// instance (Functor m, MonadPlus m) => Alternative (StateT s m) where
  ///     empty = StateT $ \ _ -> mzero
  ///     StateT m <|> StateT n = StateT $ \ s -> m s `mplus` n s
  template <copy_constructible_object Fn, class S>
  requires std::invocable<Fn, S> and alternative<std::invoke_result_t<Fn, S>>
  struct alternative_traits<StateT<Fn, S>> {
    struct combine_op {
      struct closure {
        template <isStateT ST1, isStateT ST2, class T>
        constexpr auto operator()(ST1&& x, ST2&& y, const T& t) const
          noexcept(noexcept(mpc::combine<StateT_monad_t<ST1>>(run_StateT % std::forward<ST1>(x) % t, run_StateT % std::forward<ST2>(y) % t)))
          -> decltype(      mpc::combine<StateT_monad_t<ST1>>(run_StateT % std::forward<ST1>(x) % t, run_StateT % std::forward<ST2>(y) % t)) {
          return            mpc::combine<StateT_monad_t<ST1>>(run_StateT % std::forward<ST1>(x) % t, run_StateT % std::forward<ST2>(y) % t);
        }
      };

      template <isStateT ST1, isStateT ST2>
      constexpr auto operator()(ST1&& x, ST2&& y) const
        noexcept(noexcept(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST1>(x), std::forward<ST2>(y)))))
        -> decltype(      make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST1>(x), std::forward<ST2>(y)))) {
        return            make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST1>(x), std::forward<ST2>(y)));
      }
    };

    static constexpr auto empty = make_StateT<S>([](auto&&) { return mpc::empty<StateT_monad_t<StateT<Fn, S>>>; });
    static constexpr combine_op combine{};
  };

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
          noexcept(noexcept(mpc::fmap<M>(perfect_forwarded_t<nested_closure>{}(std::forward<T>(t)), std::forward<M>(m))))
          -> decltype(      mpc::fmap<M>(perfect_forwarded_t<nested_closure>{}(std::forward<T>(t)), std::forward<M>(m))) {
          return            mpc::fmap<M>(perfect_forwarded_t<nested_closure>{}(std::forward<T>(t)), std::forward<M>(m));
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
  // clang-format on

  // Grobal methods:
  // [x] eval_StateT
  // [x] exec_StateT
  // [x] map_StateT
  // [x] with_StateT
  // [x] state
  // [x] get1
  // [x] put
  // [x] modify
  // [ ] modify'
  // [x] get2

  // eval_StateT, exec_StateT, map_StateT, with_StateT, state
  namespace detail {
    // clang-format off
    struct eval_StateT_op {
      template <isStateT ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   mpc::fmap<StateT_monad_t<ST>>(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(mpc::fmap<StateT_monad_t<ST>>(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
        return      mpc::fmap<StateT_monad_t<ST>>(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct exec_StateT_op {
      template <isStateT ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   mpc::fmap<StateT_monad_t<ST>>(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(mpc::fmap<StateT_monad_t<ST>>(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
        return      mpc::fmap<StateT_monad_t<ST>>(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct map_StateT_op {
      template <class Fn2, isStateT ST>
      constexpr auto operator()(Fn2&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn2>(f), run_StateT % std::forward<ST>(x)))))
        -> decltype(make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn2>(f), run_StateT % std::forward<ST>(x)))) {
        return      make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn2>(f), run_StateT % std::forward<ST>(x)));
      }
    };

    struct with_StateT_op {
      template <class Fn2, isStateT ST>
      constexpr auto operator()(Fn2&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn2>(f)))))
        -> decltype(make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn2>(f)))) {
        return      make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn2>(f)));
      }
    };

    template <class>
    struct state_op;

    template <copy_constructible_object Fn, class S>
    struct state_op<StateT<Fn, S>> {
      template <copy_constructible_object Fn2>
      constexpr auto operator()(Fn2&& f) const noexcept(
        noexcept(   make_StateT<S>(compose(mpc::returns<StateT_monad_t<StateT<Fn, S>>>, std::forward<Fn2>(f)))))
        -> decltype(make_StateT<S>(compose(mpc::returns<StateT_monad_t<StateT<Fn, S>>>, std::forward<Fn2>(f)))) {
        return      make_StateT<S>(compose(mpc::returns<StateT_monad_t<StateT<Fn, S>>>, std::forward<Fn2>(f)));
      }
    };
    // clang-format on
  } // namespace detail

  inline namespace cpo {
    inline constexpr perfect_forwarded_t<detail::eval_StateT_op> eval_StateT{};

    inline constexpr perfect_forwarded_t<detail::exec_StateT_op> exec_StateT{};

    inline constexpr perfect_forwarded_t<detail::map_StateT_op> map_StateT{};

    inline constexpr perfect_forwarded_t<detail::with_StateT_op> with_StateT{};

    template <isStateT ST>
    inline constexpr perfect_forwarded_t<detail::state_op<std::remove_cvref_t<ST>>> state{};
  } // namespace cpo

  // get1, put, modify, get2
  namespace detail {
    // clang-format off
    template <isStateT ST>
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

    template <isStateT ST>
    struct modify_op {
      struct closure {
        template <class Fn, class T>
        constexpr auto operator()(Fn&& f, T&& t) const noexcept(
          noexcept(   std::make_pair(nil, std::invoke(std::forward<Fn>(f), std::forward<T>(t)))))
          -> decltype(std::make_pair(nil, std::invoke(std::forward<Fn>(f), std::forward<T>(t)))) {
          return      std::make_pair(nil, std::invoke(std::forward<Fn>(f), std::forward<T>(t)));
        }
      };

      template <class Fn>
      constexpr auto operator()(Fn&& f) const noexcept(
        noexcept(   mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f)))))
        -> decltype(mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f)))) {
        return      mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f)));
      }
    };

    template <isStateT ST>
    struct get2_op {
      struct closure {
        template <class Fn, class T>
        constexpr auto operator()(Fn&& f, const T& t) const noexcept(
          noexcept(   std::make_pair(std::invoke(std::forward<Fn>(f), t), t)))
          -> decltype(std::make_pair(std::invoke(std::forward<Fn>(f), t), t)) {
          return      std::make_pair(std::invoke(std::forward<Fn>(f), t), t);
        }
      };

      template <class Fn>
      constexpr auto operator()(Fn&& f) const noexcept(
        noexcept(   mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f)))))
        -> decltype(mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f)))) {
        return      mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f)));
      }
    };
    // clang-format on
  } // namespace detail

  inline namespace cpo {
    template <isStateT ST>
    inline constexpr auto get1 = mpc::state<ST>([](const auto& t) { return std::make_pair(t, t); });

    template <isStateT ST>
    inline constexpr perfect_forwarded_t<detail::put_op<std::remove_cvref_t<ST>>> put{};

    template <isStateT ST>
    inline constexpr perfect_forwarded_t<detail::modify_op<std::remove_cvref_t<ST>>> modify{};

    template <isStateT ST>
    inline constexpr perfect_forwarded_t<detail::get2_op<std::remove_cvref_t<ST>>> get2{};
  } // namespace cpo

  // State
  // [x] State
  // [x] isState
  // [x] State_state_t
  // [ ] State_monad_t
  // [x] make_State
  // [x] run_State

  /// type State s = StateT s Identity
  template <copy_constructible_object Fn, class S>
  requires std::invocable<Fn, S> and isIdentity<std::invoke_result_t<Fn, S>>
  using State = StateT<Fn, S>;

  // isState
  namespace detail {
    template <class>
    struct is_State : std::false_type {};

    template <copy_constructible_object Fn, class S>
    requires std::invocable<Fn, S> and isIdentity<std::invoke_result_t<Fn, S>>
    struct is_State<StateT<Fn, S>> : std::true_type {
    };
  } // namespace detail

  template <class T>
  concept isState = detail::is_State<std::remove_cvref_t<T>>::value;

  template<isState ST>
  using State_state_t = typename std::remove_cvref_t<ST>::state_type;

  // This seems to be unnecessary because we know `State_monad_t = Identity`
  // template<isState ST>
  // using State_monad_t = typename std::remove_cvref_t<ST>::monad_type;

  // make_State, run_State
  namespace detail {
    struct run_State_op {
      template <isState ST>
      constexpr auto operator()(ST&& x) const noexcept
        -> decltype(compose(run_Identity, run_StateT % std::forward<ST>(x))) {
        return compose(run_Identity, run_StateT % std::forward<ST>(x));
      }
    };
  } // namespace detail

  namespace cpo {
    template <class S>
    inline constexpr auto make_State = make_StateT<S>;

    inline constexpr perfect_forwarded_t<detail::run_State_op> run_State{};
  } // namespace cpo

  // Grobal methods:
  // [x] eval_State
  // [x] exec_State
  // [x] map_State
  // [x] with_State

  // eval_State, exec_State, map_State, with_State
  namespace detail {
    // clang-format off
    struct eval_State_op {
      template <isState ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   fst(run_State % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(fst(run_State % std::forward<ST>(x) % std::forward<T>(t))) {
        return      fst(run_State % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct exec_State_op {
      template <isState ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   snd(run_State % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(snd(run_State % std::forward<ST>(x) % std::forward<T>(t))) {
        return      snd(run_State % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct map_State_op {
      template <class Fn2, isState ST>
      constexpr auto operator()(Fn2&& f, ST&& x) const noexcept(
        noexcept(   map_StateT % compose(make_Identity, compose(std::forward<Fn2>(f), run_Identity)) % std::forward<ST>(x)))
        -> decltype(map_StateT % compose(make_Identity, compose(std::forward<Fn2>(f), run_Identity)) % std::forward<ST>(x)) {
        return      map_StateT % compose(make_Identity, compose(std::forward<Fn2>(f), run_Identity)) % std::forward<ST>(x);
      }
    };
    // clang-format on
  } // namespace detail

  inline namespace cpo {
    inline constexpr perfect_forwarded_t<detail::eval_State_op> eval_State{};

    inline constexpr perfect_forwarded_t<detail::exec_State_op> exec_State{};

    inline constexpr perfect_forwarded_t<detail::map_State_op> map_State{};

    inline constexpr auto with_State = with_StateT;
  } // namespace cpo
} // namespace mpc
