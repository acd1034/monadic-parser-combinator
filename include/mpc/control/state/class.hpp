/// @file class.hpp
#pragma once
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/prelude/unit.hpp>

// clang-format off

namespace mpc {
  // monad_state
  // https://hackage.haskell.org/package/mtl-2.2.2/docs/Control-Monad-State-Class.html

  template <class>
  struct monad_state_traits;

  template <class ST>
  concept monad_state = requires {
    monad_state_traits<std::remove_cvref_t<ST>>::state;
    monad_state_traits<std::remove_cvref_t<ST>>::get1;
    monad_state_traits<std::remove_cvref_t<ST>>::put;
  };

  // class requirements

  namespace detail {
    /// state :: (s -> (a, s)) -> m a
    template <class ST>
    struct state_op {
      template <class Fn>
      constexpr auto operator()(Fn&& fn) const noexcept(
      noexcept(   monad_state_traits<std::remove_cvref_t<ST>>::state(std::forward<Fn>(fn))))
      -> decltype(monad_state_traits<std::remove_cvref_t<ST>>::state(std::forward<Fn>(fn)))
      { return    monad_state_traits<std::remove_cvref_t<ST>>::state(std::forward<Fn>(fn)); }
    };

    /// put :: s -> m ()
    template <class ST>
    struct put_op {
      template <class S>
      constexpr auto operator()(S&& s) const noexcept(
      noexcept(   monad_state_traits<std::remove_cvref_t<ST>>::put(std::forward<S>(s))))
      -> decltype(monad_state_traits<std::remove_cvref_t<ST>>::put(std::forward<S>(s)))
      { return    monad_state_traits<std::remove_cvref_t<ST>>::put(std::forward<S>(s)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// state :: (s -> (a, s)) -> m a
    template <class ST>
    inline constexpr perfect_forwarded_t<detail::state_op<ST>> state{};

    /// get1 :: m s
    template <class ST>
    requires requires {
      monad_state_traits<std::remove_cvref_t<ST>>::get1;
    }
    inline constexpr auto get1 = monad_state_traits<std::remove_cvref_t<ST>>::get1;

    /// put :: s -> m ()
    template <class ST>
    inline constexpr perfect_forwarded_t<detail::put_op<ST>> put{};
  } // namespace cpo

  // Deducibles
  // [ ] state
  // [x] get1
  // [x] put

  namespace states {
    namespace detail {
      /// state :: (s -> (a, s)) -> m a
      /// state f = do
      ///   s <- get
      ///   let ~(a, s') = f s
      ///   put s'
      ///   return a

      /// put :: s -> m ()
      /// put s = state (\_ -> ((), s))
      template <class ST>
      struct put_op {
        struct closure {
          template <class T, class A>
          constexpr auto operator()(T&& t, A&&) const noexcept(
            noexcept(   std::make_pair(unit, std::forward<T>(t))))
            -> decltype(std::make_pair(unit, std::forward<T>(t))) {
            return      std::make_pair(unit, std::forward<T>(t));
          }
        };

        template <class T>
        constexpr auto operator()(T&& t) const noexcept(
          noexcept(   mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<T>(t)))))
          -> decltype(mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<T>(t)))) {
          return      mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<T>(t)));
        }
      };
    } // namespace detail

    // /// state :: (s -> (a, s)) -> m a
    // template <class ST>
    // inline constexpr perfect_forwarded_t<detail::state_op<ST>> state{};

    /// get1 :: m s
    /// get1 = state (\s -> (s, s))
    template <class ST>
    requires requires {
      monad_state_traits<std::remove_cvref_t<ST>>::state;
    }
    inline constexpr auto get1 = mpc::state<ST>([](const auto& t) { return std::make_pair(t, t); });

    /// put :: s -> m ()
    template <class ST>
    requires requires {
      monad_state_traits<std::remove_cvref_t<ST>>::state;
    }
    inline constexpr perfect_forwarded_t<detail::put_op<ST>> put{};
  } // namespace states

  // Grobal methods
  // [x] modify
  // [ ] modify'
  // [x] get2

  // modify, get2
  namespace detail {
    /// modify :: MonadState s m => (s -> s) -> m ()
    template <class ST>
    struct modify_op {
      struct closure {
        template <class Fn, class T>
        constexpr auto operator()(Fn&& f, T&& t) const noexcept(
          noexcept(   std::make_pair(unit, std::invoke(std::forward<Fn>(f), std::forward<T>(t)))))
          -> decltype(std::make_pair(unit, std::invoke(std::forward<Fn>(f), std::forward<T>(t)))) {
          return      std::make_pair(unit, std::invoke(std::forward<Fn>(f), std::forward<T>(t)));
        }
      };

      template <class Fn>
      constexpr auto operator()(Fn&& f) const noexcept(
        noexcept(   mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f)))))
        -> decltype(mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f)))) {
        return      mpc::state<ST>(perfect_forwarded_t<closure>{}(std::forward<Fn>(f)));
      }
    };

    /// get2 :: MonadState s m => (s -> a) -> m a
    template <class ST>
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
  } // namespace detail

  inline namespace cpo {
    /// modify :: MonadState s m => (s -> s) -> m ()
    template <class ST>
    inline constexpr perfect_forwarded_t<detail::modify_op<std::remove_cvref_t<ST>>> modify{};

    /// get2 :: MonadState s m => (s -> a) -> m a
    template <class ST>
    inline constexpr perfect_forwarded_t<detail::get2_op<std::remove_cvref_t<ST>>> get2{};
  } // namespace cpo
}

// clang-format on
