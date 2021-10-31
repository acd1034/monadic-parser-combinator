/// @file stateT.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/monad.hpp>
#include <mpc/control/monad_state.hpp>
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  template <class S, class Fn>
  requires std::is_copy_constructible_v<Fn> and std::invocable<Fn, S> and monad<
    std::invoke_result_t<Fn, S>>
  struct stateT {
  private:
    std::optional<Fn> fn{};

  public:
    template <class S2 = S, class Fn2 = Fn>
    constexpr stateT(S2&&, Fn2&& fn2) : fn{fn2} {}

    constexpr auto run_stateT() const -> decltype(fn.value()) {
      return fn.value();
    }
  };

  template <class S, class Fn>
  stateT(S, Fn) -> stateT<S, Fn>;

  namespace detail {
    struct run_stateT_op {
      template <class S, class Fn>
      constexpr auto operator()(const stateT<S, Fn>& st) const noexcept(noexcept(st.run_stateT()))
        -> decltype((st.run_stateT())) {
        return st.run_stateT();
      }
    };
  } // namespace detail

  inline constexpr perfect_forwarded_t<detail::run_stateT_op> run_stateT{};

  // See below for instances of stateT
  // https://hackage.haskell.org/package/transformers-0.6.0.2/docs/src/Control.Monad.Trans.State.Lazy.html#StateT

  template <class S, class Fn>
  struct monad_traits<stateT<S, Fn>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b -- infixl 1
    struct bind_op {
      struct nested_closure {
        template <class F, class U>
        constexpr auto operator()(F&& f, U&& u) const
        // FIXME
        // noexcept(noexcept())
        // -> decltype(      )
        {
          auto&& [a, s] = std::forward<U>(u);
          return run_stateT % std::invoke(std::forward<F>(f), std::forward<decltype(a)>(a))
                 % std::forward<decltype(s)>(s);
        }
      };

      struct closure {
        template <class ST, class F, class T>
        constexpr auto operator()(ST&& x, F&& f, T&& t) const
        // FIXME
        // noexcept(noexcept())
        // -> decltype()
        {
          // clang-format off
          return mpc::bind<decltype(run_stateT % std::forward<ST>(x) % std::forward<T>(t))>(run_stateT % std::forward<ST>(x) % std::forward<T>(t), perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)));
          // clang-format on
        }
      };

      template <class ST, class F>
      constexpr auto operator()(ST&& x, F&& f) const {
        return stateT{S{}, perfect_forwarded_t<closure>{}(std::forward<ST>(x), std::forward<F>(f))};
      }
    };

    static constexpr bind_op bind{};
  };

  template <class S, class Fn>
  struct functor_traits<stateT<S, Fn>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      struct nested_closure {
        template <class F, class U>
        constexpr auto operator()(F&& f, U&& u) const {
          auto&& [a, s] = std::forward<U>(u);
          return std::make_tuple(std::invoke(std::forward<F>(f), std::forward<decltype(a)>(a)),
                                 std::forward<decltype(s)>(s));
        }
      };

      struct closure {
        template <class F, class ST, class T>
        constexpr auto operator()(F&& f, ST&& x, T&& t) const {
          using M = decltype(run_stateT % x % t);
          return mpc::fmap<M>(perfect_forwarded_t<nested_closure>{}(std::forward<F>(f)),
                              run_stateT % std::forward<ST>(x) % std::forward<T>(t));
        }
      };

      static_assert(std::default_initializable<closure>);
      template <class F, class ST>
      constexpr auto operator()(F&& f, ST&& x) const {
        return stateT{S{}, perfect_forwarded_t<closure>{}(std::forward<F>(f), std::forward<ST>(x))};
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd<stateT<S, Fn>>;
  };

  // clang-format off
  template <class S, class Fn>
  struct applicative_traits<stateT<S, Fn>> {
    /// pure   :: a -> f a
    struct pure_op {
      struct closure {
        template <class A, class T>
        requires std::invocable<Fn, T&&> and monad<std::invoke_result_t<Fn, T&&>>
        constexpr auto operator()(A&& a, T&& t) const {
          return returns<std::invoke_result_t<Fn, T&&>>(
            std::make_tuple(std::forward<A>(a), std::forward<T>(t)));
        }
      };

      template <class A>
      constexpr auto operator()(A&& a) const {
        return stateT{S{}, perfect_forwarded_t<closure>{}(std::forward<A>(a))};
      }
    };

    static constexpr pure_op pure{};
    static constexpr auto seq_apply = monads::seq_apply<stateT<S, Fn>>;
    static constexpr auto liftA2 = applicatives::liftA2<stateT<S, Fn>>;
    static constexpr auto discard2nd = applicatives::discard2nd<stateT<S, Fn>>;
    static constexpr auto discard1st = monads::discard1st<stateT<S, Fn>>;
  };
  // clang-format on

  // clang-format off
  template <class S, class Fn>
  struct monad_state_traits<stateT<S, Fn>> {
    struct states_op {
      template <class Fn2>
      constexpr auto operator()(Fn2&& f) const {
        return stateT{
          S{},
          compose(mpc::returns<std::invoke_result_t<Fn, S>>, std::forward<Fn2>(f))
        };
      }
    };

    struct puts_op {
      struct closure {
        template <class T, class A>
        requires std::invocable<Fn, T&&> and monad<std::invoke_result_t<Fn, T&&>>
        constexpr auto operator()(T&& t, A&&) const {
          return std::make_tuple(std::tuple<>{}, std::forward<T>(t));
        }
      };

      template <class T>
      constexpr auto operator()(T&& t) const {
        return states_op{}(perfect_forwarded_t<closure>{}(std::forward<T>(t)));
      }
    };

    static constexpr states_op states{};
    static constexpr puts_op puts{};
    static constexpr auto gets = states_op{}([](const auto& t) { return std::make_tuple(t, t); });
  };
  // clang-format on
} // namespace mpc
