/// @file Maybe.hpp
#pragma once
#include <functional>
#include <variant>
#include <mpc/control/alternative.hpp>
#include <mpc/control/monad.hpp>
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/utility/alternative_value_t.hpp>
#include <mpc/utility/single.hpp>

namespace mpc {
  struct nothing_t {
    inline constexpr auto operator<=>(const nothing_t&) const = default;
  }; // struct nothing_t

  inline constexpr nothing_t nothing;

  template <class T>
  using just_t = single<T, nothing_t>;

  template <class T>
  constexpr just_t<std::unwrap_ref_decay_t<T>> make_just(T&& t) {
    return std::forward<T>(t);
  }

  /// data Maybe a = Nothing | Just a
  template <class T>
  using Maybe = std::variant<nothing_t, just_t<T>>;

  namespace detail {
    template <class>
    struct is_Maybe : std::false_type {};

    template <class T>
    struct is_Maybe<Maybe<T>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept isMaybe = detail::is_Maybe<std::remove_cvref_t<T>>::value;

  // https://hackage.haskell.org/package/base-4.15.0.0/docs/src/GHC-Maybe.html#Maybe

  /// instance Functor Maybe where
  ///     fmap _ Nothing  = Nothing
  ///     fmap f (Just a) = Just (f a)
  ///
  ///     _ <$ Nothing = Nothing
  ///     a <$ Just _  = Just a
  template <class T1>
  struct functor_traits<Maybe<T1>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class F, isMaybe M>
      constexpr auto operator()(F&& f, M&& x) const //
        -> Maybe<std::unwrap_ref_decay_t<decltype(std::invoke(std::forward<F>(f),
                                                              *std::get<1>(std::forward<M>(x))))>> {
        if (x.index() == 0) {
          return nothing;
        } else {
          return make_just(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x))));
        }
      }
    };

    // (<$ ) :: a -> f b -> f a
    struct replace2nd_op {
      template <class U, isMaybe M>
      constexpr auto operator()(U&& u, M&& m) const //
        -> Maybe<std::unwrap_ref_decay_t<U&&>> {
        if (m.index() == 0) {
          return nothing;
        } else {
          return make_just(std::forward<U>(u));
        }
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr replace2nd_op replace2nd{};
  };

  /// instance Monad Maybe where
  ///     (Just x) >>= k = k x
  ///     Nothing  >>= _ = Nothing
  template <class T1>
  struct monad_traits<Maybe<T1>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b -- infixl 1
    struct bind_op {
      template <isMaybe M, class F>
      constexpr auto operator()(M&& x, F&& f) const //
        -> decltype(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x)))) {
        if (x.index() == 0) {
          return nothing;
        } else {
          return std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x)));
        }
      }
    };

    static constexpr bind_op bind{};
  };

  /// instance Applicative Maybe where
  ///     pure = Just
  ///
  ///     Nothing <*> _m      = Nothing
  ///     Just f  <*> m       = fmap f m
  ///
  ///     liftA2 f (Just x) (Just y) = Just (f x y)
  ///     liftA2 _ _ _ = Nothing
  template <class T1>
  struct applicative_traits<Maybe<T1>> {
    /// pure   :: a -> f a
    struct pure_op {
      template <class U>
      constexpr auto operator()(U&& u) const   //
        -> Maybe<std::unwrap_ref_decay_t<U>> { //
        return make_just(std::forward<U>(u));
      }
    };

    /// (<*>)  :: f (a -> b) -> f a -> f b -- infixl 4
    struct seq_apply_op {
      template <isMaybe M1, isMaybe M2>
      constexpr auto operator()(M1&& f, M2&& x) const //
        -> decltype(mpc::fmap<Maybe<T1>>(*std::get<1>(std::forward<M1>(f)), std::forward<M2>(x))) {
        if (f.index() == 0) {
          return nothing;
        } else {
          return mpc::fmap<Maybe<T1>>(*std::get<1>(std::forward<M1>(f)), std::forward<M2>(x));
        }
      }
    };

    /// liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    struct liftA2_op {
      template <class F, isMaybe M1, isMaybe M2>
      constexpr auto operator()(F&& f, M1&& m1, M2&& m2) const //
        -> Maybe<
          std::unwrap_ref_decay_t<decltype(std::invoke(std::forward<F>(f),                 //
                                                       *std::get<1>(std::forward<M1>(m1)), //
                                                       *std::get<1>(std::forward<M2>(m2))))>> {
        if (m1.index() == 1 and m2.index() == 1) {
          return make_just(std::invoke(std::forward<F>(f),                 //
                                       *std::get<1>(std::forward<M1>(m1)), //
                                       *std::get<1>(std::forward<M2>(m2))));
        } else {
          return nothing;
        }
      }
    };

    static constexpr pure_op pure{};
    static constexpr seq_apply_op seq_apply{};
    static constexpr liftA2_op liftA2{};
    static constexpr auto discard2nd = applicatives::discard2nd<Maybe<T1>>;
    static constexpr auto discard1st = applicatives::discard1st_opt<Maybe<T1>>;
  };

  /// instance Alternative Maybe where
  ///     empty = Nothing
  ///     Nothing <|> r = r
  ///     l       <|> _ = l
  template <class T1>
  struct alternative_traits<Maybe<T1>> {
    struct combine_op {
      template <isMaybe M1, isMaybe M2>
      requires std::same_as<std::remove_cvref_t<M1>, std::remove_cvref_t<M2>>
      constexpr auto operator()(M1&& m1, M2&& m2) const //
        -> std::remove_cvref_t<M1> {
        return (m1.index() == 1 ? m1 : m2);
      }
    };

    static constexpr auto empty = nothing;
    static constexpr combine_op combine{};
  };
} // namespace mpc
