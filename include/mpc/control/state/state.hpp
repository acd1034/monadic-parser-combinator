/// @file state.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/state/stateT.hpp>
#include <mpc/data/functor/identity.hpp>
#include <mpc/prelude/compose.hpp>
#include <mpc/prelude/fst.hpp>

// clang-format off

namespace mpc {
  // State
  // [x] State
  // [x] isState
  // [x] State_state_t
  // [ ] State_monad_t
  // [x] make_State
  // [x] run_State

  /// type State s = StateT s Identity
  template <class S, is_Identity M>
  using State = StateT<S, M>;

  // isState
  namespace detail {
    template <class>
    struct is_State : std::false_type {};

    template <class S, is_Identity M>
    struct is_State<StateT<S, M>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept isState = detail::is_State<std::remove_cvref_t<T>>::value;

  template <isState ST>
  using State_state_t = StateT_state_t<ST>;

  template<isState ST>
  using State_monad_t = StateT_monad_t<ST>;

  // make_State, run_State
  namespace detail {
    template <class S>
    struct make_State_op {
      template <class Fn>
      requires std::invocable<std::decay_t<Fn>, const std::decay_t<S>&> and is_Identity<std::invoke_result_t<std::decay_t<Fn>, const std::decay_t<S>&>>
      constexpr auto operator()(Fn&& f) const {
        using M = std::invoke_result_t<std::decay_t<Fn>, const std::decay_t<S>&>;
        return State<const std::decay_t<S>&, M>(std::forward<Fn>(f));
      }
    };

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
    inline constexpr perfect_forwarded_t<detail::make_State_op<S>> make_State{};

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
}

// clang-format on
