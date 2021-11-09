/// @file either.hpp
#pragma once
#include <functional> // std::invoke
#include <variant>
#include <mpc/control/monad.hpp>
#include <mpc/prelude.hpp>
#include <mpc/utility/alternative_value_t.hpp>
#include <mpc/utility/single.hpp>

// clang-format off

namespace mpc {
  // Either
  // https://hackage.haskell.org/package/base-4.15.0.0/docs/src/Data-Either.html#Either

  template <class T>
  using left_t = single<T, std::false_type>;

  template <class T>
  using right_t = single<T, std::true_type>;

  template <class T>
  constexpr left_t<std::unwrap_ref_decay_t<T>> make_left(T&& t) {
    return std::forward<T>(t);
  }

  template <class T>
  constexpr right_t<std::unwrap_ref_decay_t<T>> make_right(T&& t) {
    return std::forward<T>(t);
  }

  /// data Either a b = Left a | Right b
  template <class T, class U>
  using Either = std::variant<left_t<T>, right_t<U>>;

  namespace detail {
    template <class>
    struct is_Either : std::false_type {};

    template <class T, class U>
    struct is_Either<Either<T, U>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept isEither = detail::is_Either<std::remove_cvref_t<T>>::value;

  /// instance Functor (Either a) where
  ///   fmap _ (Left x) = Left x
  ///   fmap f (Right y) = Right (f y)
  template <class T1, class T2>
  struct functor_traits<Either<T1, T2>> {
    /// fmap :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class F, isEither E>
      constexpr auto operator()(F&& f, E&& e) const
        -> Either<alternative_value_t<0, E>,
                  std::unwrap_ref_decay_t<std::invoke_result_t<F, alternative_value_t<1, E>>>> {
        if (e.index() == 0) {
          return std::get<0>(std::forward<E>(e));
        } else {
          return make_right(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<E>(e))));
        }
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd;
  };

  /// instance Monad (Either e) where
  ///   Left  l >>= _ = Left l
  ///   Right r >>= k = k r
  template <class T1, class T2>
  struct monad_traits<Either<T1, T2>> {
    /// bind :: forall a b. m a -> (a -> m b) -> m b
    struct bind_op {
      template <isEither E, class F>
      constexpr auto operator()(E&& e, F&& f) const //
        -> decltype(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<E>(e)))) {
        if (e.index() == 0) {
          return std::get<0>(std::forward<E>(e));
        } else {
          return std::invoke(std::forward<F>(f), *std::get<1>(std::forward<E>(e)));
        }
      }
    };

    static constexpr bind_op bind{};
  };

  /// instance Applicative (Either e) where
  ///   pure          = Right
  ///   Left  e <*> _ = Left e
  ///   Right f <*> r = fmap f r
  template <class T1, class T2>
  struct applicative_traits<Either<T1, T2>> {
    /// pure :: a -> f a
    struct pure_op {
      template <class U>
      constexpr auto operator()(U&& u) const        //
        -> Either<T1, std::unwrap_ref_decay_t<U>> { //
        return make_right(std::forward<U>(u));
      }
    };

    /// seq_apply :: f (a -> b) -> f a -> f b
    struct seq_apply_op {
      template <isEither E1, isEither E2>
      constexpr auto operator()(E1&& e1, E2&& e2) const
        -> Either<
          std::common_type_t<
            alternative_value_t<0, E1>,
            alternative_value_t<0, decltype(mpc::fmap(*std::get<1>(std::forward<E1>(e1)), std::forward<E2>(e2)))>
          >,
          alternative_value_t<1, decltype(mpc::fmap(*std::get<1>(std::forward<E1>(e1)), std::forward<E2>(e2)))>
        > {
        if (e1.index() == 0) {
          return std::get<0>(std::forward<E1>(e1));
        } else {
          return mpc::fmap(*std::get<1>(std::forward<E1>(e1)), std::forward<E2>(e2));
        }
      }
    };

    static constexpr pure_op pure{};
    static constexpr seq_apply_op seq_apply{};
    static constexpr auto liftA2 = applicatives::liftA2;
    static constexpr auto discard2nd = applicatives::discard2nd;
    static constexpr auto discard1st = monads::discard1st;
  };
} // namespace mpc

// clang-format on
