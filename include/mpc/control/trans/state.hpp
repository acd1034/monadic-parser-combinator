/// @file state.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/monad.hpp>
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

  /// newtype StateT s m a = StateT { run_StateT :: s -> m (a,s) }
  template <copy_constructible_object Fn, class S>
  requires std::invocable<Fn, S> and monad<std::invoke_result_t<Fn, S>>
  struct StateT : Identity<Fn> {
    using Identity<Fn>::Identity;
  };

  namespace detail {
    template <class>
    struct is_StateT : std::false_type {};

    template <copy_constructible_object Fn, class S>
    struct is_StateT<StateT<Fn, S>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept isStateT = detail::is_StateT<std::remove_cvref_t<T>>::value;

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

  template <class S>
  inline constexpr perfect_forwarded_t<detail::make_StateT_op<S>> make_StateT{};
  inline constexpr perfect_forwarded_t<detail::run_StateT_op> run_StateT{};

  // instances:
  // [x] functor
  // [x] monad
  // [x] applicative
  // [ ] alternative
  // [ ] monad_trans

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
        //   noexcept(   run_StateT % std::invoke(std::forward<F>(f), get<0>(std::forward<U>(u)))
        //                          % get<1>(std::forward<U>(u))))
        //   -> decltype(run_StateT % std::invoke(std::forward<F>(f), get<0>(std::forward<U>(u)))
        //                          % get<1>(std::forward<U>(u)))
        {
          auto&& [a, s] = std::forward<U>(u);
          return      run_StateT % std::invoke(std::forward<F>(f), std::forward<decltype(a)>(a))
                                 % std::forward<decltype(s)>(s);
        }
      };

      struct closure {
        // clang-format off
        template <isStateT ST, class F, class T>
        constexpr auto operator()(ST&& x, F&& f, T&& t) const noexcept(
          noexcept(
            mpc::bind<decltype(run_StateT % x % t)>(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)))))
          -> decltype(
            mpc::bind<decltype(run_StateT % x % t)>(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)))) {
          return
            mpc::bind<decltype(run_StateT % x % t)>(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)));
        }
        // clang-format on
      };

      template <isStateT ST, class F>
      constexpr auto operator()(ST&& x, F&& f) const
        noexcept(noexcept(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x),
                                                                        std::forward<F>(f)))))
          -> decltype(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<ST>(x),
                                                                    std::forward<F>(f)))) {
        return make_StateT<S>(
          perfect_forwarded_t<closure>{}(std::forward<ST>(x), std::forward<F>(f)));
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
        //   noexcept(   std::make_pair(std::invoke(std::forward<F>(f), get<0>(std::forward<U>(u))),
        //                              get<1>(std::forward<U>(u)))))
        //   -> decltype(std::make_pair(std::invoke(std::forward<F>(f), get<0>(std::forward<U>(u))),
        //                              get<1>(std::forward<U>(u))))
        {
          auto&& [a, s] = std::forward<U>(u);
          return std::make_pair(std::invoke(std::forward<F>(f), std::forward<decltype(a)>(a)),
                                std::forward<decltype(s)>(s));
        }
      };

      struct closure {
        template <class F, isStateT ST, class T>
        constexpr auto operator()(F&& f, ST&& x, T&& t) const
          noexcept(noexcept(mpc::fmap<decltype(run_StateT % x % t)>(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
            -> decltype(mpc::fmap<decltype(run_StateT % x % t)>(
              perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
              run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
          return mpc::fmap<decltype(run_StateT % x % t)>(
            perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t));
        }
      };

      template <class F, isStateT ST>
      constexpr auto operator()(F&& f, ST&& x) const
        noexcept(noexcept(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f),
                                                                        std::forward<ST>(x)))))
          -> decltype(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<F>(f),
                                                                    std::forward<ST>(x)))) {
        return make_StateT<S>(
          perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x)));
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
        constexpr auto operator()(A&& a, T&& t) const
          noexcept(noexcept(returns<std::invoke_result_t<Fn, S>>(
            std::make_pair(std::forward<A>(a), std::forward<T>(t)))))
            -> decltype(returns<std::invoke_result_t<Fn, S>>(std::make_pair(std::forward<A>(a),
                                                                            std::forward<T>(t)))) {
          return returns<std::invoke_result_t<Fn, S>>(
            std::make_pair(std::forward<A>(a), std::forward<T>(t)));
        }
      };

      template <class A>
      constexpr auto operator()(A&& a) const
        noexcept(noexcept(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<A>(a)))))
          -> decltype(make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<A>(a)))) {
        return make_StateT<S>(perfect_forwarded_t<closure>{}(std::forward<A>(a)));
      }
    };

    static constexpr pure_op pure{};
    static constexpr auto seq_apply = monads::seq_apply<StateT<Fn, S>>;
    static constexpr auto liftA2 = applicatives::liftA2<StateT<Fn, S>>;
    static constexpr auto discard2nd = applicatives::discard2nd<StateT<Fn, S>>;
    static constexpr auto discard1st = monads::discard1st<StateT<Fn, S>>;
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

  namespace detail {
    // clang-format off

    template <class>
    struct eval_StateT_op;

    template <copy_constructible_object Fn, class S>
    struct eval_StateT_op<StateT<Fn, S>> {
      template <isStateT ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   mpc::fmap<std::invoke_result_t<Fn, T&&>>(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(mpc::fmap<std::invoke_result_t<Fn, T&&>>(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
        return      mpc::fmap<std::invoke_result_t<Fn, T&&>>(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    template <class>
    struct exec_StateT_op;

    template <copy_constructible_object Fn, class S>
    struct exec_StateT_op<StateT<Fn, S>> {
      template <isStateT ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   mpc::fmap<std::invoke_result_t<Fn, T&&>>(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(mpc::fmap<std::invoke_result_t<Fn, T&&>>(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
        return      mpc::fmap<std::invoke_result_t<Fn, T&&>>(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    template <class>
    struct map_StateT_op;

    template <copy_constructible_object Fn, class S>
    struct map_StateT_op<StateT<Fn, S>> {
      template <class Fn2, isStateT ST>
      constexpr auto operator()(Fn2&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<S> % compose(std::forward<Fn2>(f), run_StateT % std::forward<ST>(x))))
        -> decltype(make_StateT<S> % compose(std::forward<Fn2>(f), run_StateT % std::forward<ST>(x))) {
        return      make_StateT<S> % compose(std::forward<Fn2>(f), run_StateT % std::forward<ST>(x));
      }
    };

    template <class>
    struct with_StateT_op;

    template <copy_constructible_object Fn, class S>
    struct with_StateT_op<StateT<Fn, S>> {
      template <class Fn2, isStateT ST>
      constexpr auto operator()(Fn2&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<S> % compose(run_StateT % std::forward<ST>(x), std::forward<Fn2>(f))))
        -> decltype(make_StateT<S> % compose(run_StateT % std::forward<ST>(x), std::forward<Fn2>(f))) {
        return      make_StateT<S> % compose(run_StateT % std::forward<ST>(x), std::forward<Fn2>(f));
      }
    };

    template <class>
    struct state_op;

    template <copy_constructible_object Fn, class S>
    struct state_op<StateT<Fn, S>> {
      template <copy_constructible_object Fn2>
      constexpr auto operator()(Fn2&& f) const noexcept(
        noexcept(   make_StateT<S>(compose(mpc::returns<std::invoke_result_t<Fn, S>>, std::forward<Fn2>(f)))))
        -> decltype(make_StateT<S>(compose(mpc::returns<std::invoke_result_t<Fn, S>>, std::forward<Fn2>(f)))) {
        return      make_StateT<S>(compose(mpc::returns<std::invoke_result_t<Fn, S>>, std::forward<Fn2>(f)));
      }
    };
    // clang-format on
  } // namespace detail

  inline namespace cpo {
    template <isStateT ST>
    inline constexpr perfect_forwarded_t<detail::eval_StateT_op<std::remove_cvref_t<ST>>>
      eval_StateT{};

    template <isStateT ST>
    inline constexpr perfect_forwarded_t<detail::exec_StateT_op<std::remove_cvref_t<ST>>>
      exec_StateT{};

    template <isStateT ST>
    inline constexpr perfect_forwarded_t<detail::map_StateT_op<std::remove_cvref_t<ST>>>
      map_StateT{};

    template <isStateT ST>
    inline constexpr perfect_forwarded_t<detail::with_StateT_op<std::remove_cvref_t<ST>>>
      with_StateT{};

    template <isStateT ST>
    inline constexpr perfect_forwarded_t<detail::state_op<std::remove_cvref_t<ST>>> state{};
  } // namespace cpo

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
} // namespace mpc
