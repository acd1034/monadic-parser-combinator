/// @file identity.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/monad.hpp>
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  template <class T>
  struct identity {
  private:
    T instance_{};

  public:
    template <class U = T>
    constexpr identity(U&& u) : instance_{std::forward<U>(u)} {}
    constexpr auto run_identity() const noexcept -> decltype(instance_) {
      return instance_;
    }
  };

  template <class T>
  identity(T) -> identity<T>;

  namespace detail {
    template <class>
    struct is_identity : std::false_type {};

    template <class T>
    struct is_identity<identity<T>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept Identity = detail::is_identity<std::remove_cvref_t<T>>::value;

  namespace detail {
    struct run_identity_op {
      template <class T>
      constexpr auto operator()(const identity<T>& st) const noexcept
        -> decltype((st.run_identity())) {
        return st.run_identity();
      }
    };
  } // namespace detail

  inline constexpr perfect_forwarded_t<detail::run_identity_op> run_identity{};

  // https://hackage.haskell.org/package/base-4.15.0.0/docs/src/Data-Functor-Identity.html#Identity

  /// instance Monad Identity where
  ///     m >>= k  = k (runIdentity m)
  template <class T>
  struct monad_traits<identity<T>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b -- infixl 1
    struct bind_op {
      template <Identity I, class F>
      constexpr auto operator()(I&& x, F&& f) const //
        -> decltype(std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x))) {
        return std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x));
      }
    };

    static constexpr bind_op bind{};
  };

  template <class T>
  struct functor_traits<identity<T>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class F, Identity I>
      constexpr auto operator()(F&& f, I&& x) const //
        -> decltype(identity{std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x))}) {
        return identity{std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x))};
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd<identity<T>>;
  };

  template <class T>
  struct applicative_traits<identity<T>> {
    /// pure   :: a -> f a
    struct pure_op {
      template <class U>
      constexpr auto operator()(U&& u) const {
        return identity{std::forward<U>(u)};
      }
    };

    static constexpr pure_op pure{};
    static constexpr auto seq_apply = monads::seq_apply<identity<T>>;
    static constexpr auto liftA2 = applicatives::liftA2<identity<T>>;
    static constexpr auto discard2nd = applicatives::discard2nd<identity<T>>;
    static constexpr auto discard1st = monads::discard1st<identity<T>>;
  };
} // namespace mpc
