/// @file identity.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/monad.hpp>
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/utility/copyable_box.hpp>

namespace mpc {
  // identity
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Data-Functor-Identity.html

  /// newtype Identity a = Identity { runIdentity :: a }
  template <copy_constructible_object T>
  struct identity {
  private:
    copyable_box<T> instance_{};

  public:
    constexpr identity()                                   //
      noexcept(std::is_nothrow_default_constructible_v<T>) //
      requires std::default_initializable<T>               //
      : instance_{std::in_place} {}

    constexpr explicit identity(const T& t) //
      noexcept(std::is_nothrow_copy_constructible_v<T>)
      : instance_{std::in_place, t} {}

    constexpr explicit identity(T&& t) //
      noexcept(std::is_nothrow_move_constructible_v<T>)
      : instance_{std::in_place, std::move(t)} {}

    constexpr const T& operator*() const noexcept {
      return *instance_;
    }
    constexpr T& operator*() noexcept {
      return *instance_;
    }

    constexpr const T* operator->() const noexcept {
      return instance_.operator->();
    }
    constexpr T* operator->() noexcept {
      return instance_.operator->();
    }
  };

  template <class T>
  identity(T) -> identity<T>;

  namespace detail {
    template <class>
    struct is_identity : std::false_type {};

    template <copy_constructible_object T>
    struct is_identity<identity<T>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept Identity = detail::is_identity<std::remove_cvref_t<T>>::value;

  namespace detail {
    struct make_identity_op {
      template <copy_constructible_object U>
      constexpr auto operator()(U&& u) const {
        return identity<std::decay_t<U>>(std::forward<U>(u));
      }
    };

    struct run_identity_op {
      template <Identity I>
      constexpr auto operator()(I&& x) const noexcept -> decltype(*std::forward<I>(x)) {
        return *std::forward<I>(x);
      }
    };
  } // namespace detail

  inline constexpr perfect_forwarded_t<detail::make_identity_op> make_identity{};
  inline constexpr perfect_forwarded_t<detail::run_identity_op> run_identity{};

  // clang-format off

  /// instance Monad Identity where
  ///     m >>= k  = k (runIdentity m)
  template <copy_constructible_object T>
  struct monad_traits<identity<T>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b -- infixl 1
    struct bind_op {
      template <Identity I, class F>
      constexpr auto operator()(I&& x, F&& f) const
        noexcept(noexcept(std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x))))
        -> decltype(      std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x))) {
        return            std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x));
      }
    };

    static constexpr bind_op bind{};
  };

  template <copy_constructible_object T>
  struct functor_traits<identity<T>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class F, Identity I>
      constexpr auto operator()(F&& f, I&& x) const
        noexcept(noexcept(make_identity(std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x)))))
        -> decltype(      make_identity(std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x)))) {
        return            make_identity(std::invoke(std::forward<F>(f), run_identity % std::forward<I>(x)));
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd<identity<T>>;
  };

  template <copy_constructible_object T>
  struct applicative_traits<identity<T>> {
    static constexpr auto pure = make_identity;
    static constexpr auto seq_apply = monads::seq_apply<identity<T>>;
    static constexpr auto liftA2 = applicatives::liftA2<identity<T>>;
    static constexpr auto discard2nd = applicatives::discard2nd<identity<T>>;
    static constexpr auto discard1st = monads::discard1st<identity<T>>;
  };
  // clang-format on
} // namespace mpc
