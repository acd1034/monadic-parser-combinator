/// @file stateT.hpp
#pragma once
#include <functional> // std::function, std::invoke
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

  /// newtype StateT s m a = StateT { runStateT :: s -> m (a,s) }
  template <class S, monad M>
  struct StateT : Identity<perfect_forwarded_t<std::function<M(S)>>> {
    using Identity<perfect_forwarded_t<std::function<M(S)>>>::Identity;
    using state_type = S;
    using monad_type = M;
  };

  // is_StateT
  namespace detail {
    template <class>
    struct is_StateT_impl : std::false_type {};

    template <class S, monad M>
    struct is_StateT_impl<StateT<S, M>> : std::true_type {};
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
      template <class Fn>
      requires std::invocable<std::decay_t<Fn>, const std::decay_t<S>&> and monad<std::invoke_result_t<std::decay_t<Fn>, const std::decay_t<S>&>>
      constexpr auto operator()(Fn&& f) const {
        using M = std::invoke_result_t<std::decay_t<Fn>, const std::decay_t<S>&>;
        return StateT<const std::decay_t<S>&, M>(std::forward<Fn>(f));
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
    inline constexpr partially_applicable<detail::make_StateT_op<S>> make_StateT{};

    inline constexpr partially_applicable<detail::run_StateT_op> run_StateT{};
  } // namespace cpo

  // instances:
  // [x] functor
  // [x] monad
  // [x] applicative
  // [x] alternative
  // [x] monad_trans

  /// instance (Monad m) => Monad (StateT s m) where
  ///     m >>= k  = StateT $ s -> do
  ///         ~(a, s') <- runStateT m s
  ///         runStateT (k a) s'
  template <class S, monad M>
  struct monad_traits<StateT<S, M>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b
    struct bind_op {
      struct nested_closure {
        template <class Fn, class U>
        constexpr auto operator()(Fn&& f, U&& u) const
        noexcept(noexcept(run_StateT % std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))) % snd(std::forward<U>(u))))
        -> decltype(      run_StateT % std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))) % snd(std::forward<U>(u)))
        { return          run_StateT % std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))) % snd(std::forward<U>(u)); }
      };

      struct closure {
        template <is_StateT ST, class Fn, class T>
        constexpr auto operator()(ST&& x, Fn&& f, T&& t) const noexcept(
          noexcept(
            mpc::bind(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<Fn>(f)))))
          -> decltype(
            mpc::bind(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<Fn>(f)))) {
          return
            mpc::bind(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<Fn>(f)));
        }
      };

      template <is_StateT ST, class Fn>
      constexpr auto operator()(ST&& x, Fn&& f) const noexcept(
          noexcept(   make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x), std::forward<Fn>(f)))))
          -> decltype(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x), std::forward<Fn>(f)))) {
        return        make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x), std::forward<Fn>(f)));
      }
    };

    static constexpr bind_op bind{};
  };

  /// instance (Functor m) => Functor (StateT s m) where
  ///     fmap f m = StateT $ s ->
  ///         fmap (~(a, s') -> (f a, s')) $ runStateT m s
  template <class S, monad M>
  struct functor_traits<StateT<S, M>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      struct nested_closure {
        template <class Fn, class U>
        constexpr auto operator()(Fn&& f, U&& u) const
        noexcept(noexcept(std::make_pair(std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))), snd(std::forward<U>(u)))))
        -> decltype(      std::make_pair(std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))), snd(std::forward<U>(u))))
        { return          std::make_pair(std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))), snd(std::forward<U>(u))); }
      };

      struct closure {
        template <class Fn, is_StateT ST, class T>
        constexpr auto operator()(Fn&& f, ST&& x, T&& t) const noexcept(
          noexcept(
            mpc::fmap(
            perfect_forwarded_t<nested_closure>{}(std::forward<Fn>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
          -> decltype(
            mpc::fmap(
            perfect_forwarded_t<nested_closure>{}(std::forward<Fn>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
          return
            mpc::fmap(
            perfect_forwarded_t<nested_closure>{}(std::forward<Fn>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t));
        }
      };

      template <class Fn, is_StateT ST>
      constexpr auto operator()(Fn&& f, ST&& x) const noexcept(
          noexcept(   make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f), std::forward<ST>(x)))))
          -> decltype(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f), std::forward<ST>(x)))) {
        return        make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f), std::forward<ST>(x)));
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd;
  };

  /// instance (Functor m, Monad m) => Applicative (StateT s m) where
  ///     pure a = StateT $ s -> return (a, s)
  ///     StateT mf <*> StateT mx = StateT $ s -> do
  ///         ~(f, s') <- mf s
  ///         ~(x, s'') <- mx s'
  ///         return (f x, s'')
  ///     m *> k = m >>= (_ -> k)
  template <class S, monad M>
  struct applicative_traits<StateT<S, M>> {
    /// pure   :: a -> f a
    struct pure_op {
      struct closure {
        template <class A, class T>
        constexpr auto operator()(A&& a, T&& t) const noexcept(
          noexcept(   returns<M>(std::make_pair(std::forward<A>(a), std::forward<T>(t)))))
          -> decltype(returns<M>(std::make_pair(std::forward<A>(a), std::forward<T>(t)))) {
          return      returns<M>(std::make_pair(std::forward<A>(a), std::forward<T>(t)));
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
  ///     empty = StateT $ (_ -> mzero)
  ///     StateT m <|> StateT n = StateT $ s -> m s `mplus` n s
  namespace detail {
    template <class ST>
    struct StateT_alternative_traits_empty {};

    template <is_StateT ST>
    requires has_alternative_traits_empty<StateT_monad_t<ST>>
    struct StateT_alternative_traits_empty<ST> {
      struct empty_op {
        struct closure {
          template <class T>
          constexpr auto operator()(const T& t) const noexcept(
            noexcept(   *mpc::empty<StateT_monad_t<ST>>))
            -> decltype(*mpc::empty<StateT_monad_t<ST>>) {
            return      *mpc::empty<StateT_monad_t<ST>>;
          }
        };

        constexpr auto operator()() const noexcept(
          noexcept(   make_StateT<StateT_state_t<ST>>(closure{})))
          -> decltype(make_StateT<StateT_state_t<ST>>(closure{})) {
          return      make_StateT<StateT_state_t<ST>>(closure{});
        }
      };

      static constexpr empty_op empty{};
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

  template <class S, monad M>
  struct alternative_traits<StateT<S, M>>
    : detail::StateT_alternative_traits_empty<StateT<S, M>>,
      detail::StateT_alternative_traits_combine<StateT<S, M>> {};

  /// instance MonadTrans (StateT s) where
  ///     lift m = StateT $ s -> do
  ///         a <- m
  ///         return (a, s)
  template <class S, monad M>
  struct monad_trans_traits<StateT<S, M>> {
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
        template <monad N, class T>
        constexpr auto operator()(N&& n, T&& t) const
          noexcept(noexcept(mpc::fmap(perfect_forwarded_t<nested_closure>{}(std::forward<T>(t)), std::forward<N>(n))))
          -> decltype(      mpc::fmap(perfect_forwarded_t<nested_closure>{}(std::forward<T>(t)), std::forward<N>(n))) {
          return            mpc::fmap(perfect_forwarded_t<nested_closure>{}(std::forward<T>(t)), std::forward<N>(n));
        }
      };

      template <monad N>
      constexpr auto operator()(N&& n) const
        noexcept(noexcept(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<N>(n)))))
        -> decltype(      make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<N>(n)))) {
        return            make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<N>(n)));
      }
    };

    static constexpr lift_op lift{};
  };

  /// instance MonadState (StateT s) where
  ///     state f = StateT (return . f)
  template <class S, monad M>
  struct monad_state_traits<StateT<S, M>> {
    struct state_op {
      template <copy_constructible_object Fn>
      constexpr auto operator()(Fn&& f) const noexcept(
        noexcept(   make_StateT<S>(compose(mpc::returns<M>, std::forward<Fn>(f)))))
        -> decltype(make_StateT<S>(compose(mpc::returns<M>, std::forward<Fn>(f)))) {
        return      make_StateT<S>(compose(mpc::returns<M>, std::forward<Fn>(f)));
      }
    };

    static constexpr state_op state{};
    static constexpr auto gets = states::gets<StateT<S, M>>;
    static constexpr auto put = states::put<StateT<S, M>>;
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
      template <class Fn, is_StateT ST>
      constexpr auto operator()(Fn&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn>(f), run_StateT % std::forward<ST>(x)))))
        -> decltype(make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn>(f), run_StateT % std::forward<ST>(x)))) {
        return      make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn>(f), run_StateT % std::forward<ST>(x)));
      }
    };

    struct with_StateT_op {
      template <class Fn, is_StateT ST>
      constexpr auto operator()(Fn&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn>(f)))))
        -> decltype(make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn>(f)))) {
        return      make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn>(f)));
      }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr partially_applicable<detail::eval_StateT_op> eval_StateT{};

    inline constexpr partially_applicable<detail::exec_StateT_op> exec_StateT{};

    inline constexpr partially_applicable<detail::map_StateT_op> map_StateT{};

    inline constexpr partially_applicable<detail::with_StateT_op> with_StateT{};
  } // namespace cpo
} // namespace mpc

// clang-format on
