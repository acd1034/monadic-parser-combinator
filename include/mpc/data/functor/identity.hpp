/// @file identity.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/monad.hpp>
#include <mpc/prelude.hpp>
#include <mpc/utility/copyable_box.hpp>

namespace mpc {
  // Identity
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Data-Functor-Identity.html

  /// newtype Identity a = Identity { runIdentity :: a }
  template <copy_constructible_object T>
  struct Identity {
  private:
    copyable_box<T> instance_{};

  public:
    constexpr Identity()                                   //
      noexcept(std::is_nothrow_default_constructible_v<T>) //
      requires std::default_initializable<T>               //
      : instance_{std::in_place} {}

    template <class U = T>
    requires std::constructible_from<T, U&&>
    constexpr explicit Identity(U&& u) //
      noexcept(std::is_nothrow_constructible_v<T, U&&>)
      : instance_{std::in_place, std::forward<U>(u)} {}

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
  Identity(T) -> Identity<T>;

  namespace detail {
    template <class>
    struct is_Identity_impl : std::false_type {};

    template <copy_constructible_object T>
    struct is_Identity_impl<Identity<T>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept is_Identity = detail::is_Identity_impl<std::remove_cvref_t<T>>::value;

  namespace detail {
    struct make_Identity_op {
      template <copy_constructible_object U>
      constexpr auto operator()(U&& u) const {
        return Identity<std::decay_t<U>>(std::forward<U>(u));
      }
    };

    struct run_Identity_op {
      template <is_Identity I>
      constexpr auto operator()(I&& x) const noexcept -> decltype(*std::forward<I>(x)) {
        return *std::forward<I>(x);
      }
    };
  } // namespace detail

  namespace cpo {
    inline constexpr perfect_forwarded_t<detail::make_Identity_op> make_Identity{};

    inline constexpr perfect_forwarded_t<detail::run_Identity_op> run_Identity{};
  } // namespace cpo

  // clang-format off

  /// instance Monad Identity where
  ///     m >>= k  = k (runIdentity m)
  template <copy_constructible_object T>
  struct monad_traits<Identity<T>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b
    struct bind_op {
      template <is_Identity I, class F>
      constexpr auto operator()(I&& x, F&& f) const
        noexcept(noexcept(std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x))))
        -> decltype(      std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x))) {
        return            std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x));
      }
    };

    static constexpr bind_op bind{};
  };

  template <copy_constructible_object T>
  struct functor_traits<Identity<T>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class F, is_Identity I>
      constexpr auto operator()(F&& f, I&& x) const
        noexcept(noexcept(make_Identity(std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x)))))
        -> decltype(      make_Identity(std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x)))) {
        return            make_Identity(std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x)));
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd;
  };

  template <copy_constructible_object T>
  struct applicative_traits<Identity<T>> {
    static constexpr auto pure = make_Identity;
    static constexpr auto seq_apply = monads::seq_apply;
    static constexpr auto liftA2 = applicatives::liftA2;
    static constexpr auto discard2nd = applicatives::discard2nd;
    static constexpr auto discard1st = monads::discard1st;
  };
  // clang-format on
} // namespace mpc
