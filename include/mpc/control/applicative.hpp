/// @file applicative.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/control/basic_traits.hpp>
#include <mpc/control/functor.hpp>
#include <mpc/functional/constant.hpp>
#include <mpc/functional/flip.hpp>
#include <mpc/functional/id.hpp>
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  namespace detail {
    template <class F>
    using pure_op = std::remove_cvref_t<decltype(applicative_traits<std::remove_cvref_t<F>>::pure)>;

    template <class F>
    using seq_apply_op =
      std::remove_cvref_t<decltype(applicative_traits<std::remove_cvref_t<F>>::seq_apply)>;

    template <class F>
    using liftA2_op =
      std::remove_cvref_t<decltype(applicative_traits<std::remove_cvref_t<F>>::liftA2)>;

    template <class F>
    using discard2nd_op =
      std::remove_cvref_t<decltype(applicative_traits<std::remove_cvref_t<F>>::discard2nd)>;

    template <class F>
    using discard1st_op =
      std::remove_cvref_t<decltype(applicative_traits<std::remove_cvref_t<F>>::discard1st)>;

    template <class F>
    struct pure_t : perfect_forward<pure_op<F>> {
      using perfect_forward<pure_op<F>>::perfect_forward;
    };

    template <class F>
    struct seq_apply_t : perfect_forward<seq_apply_op<F>> {
      using perfect_forward<seq_apply_op<F>>::perfect_forward;
    };

    template <class F>
    struct liftA2_t : perfect_forward<liftA2_op<F>> {
      using perfect_forward<liftA2_op<F>>::perfect_forward;
    };

    template <class F>
    struct discard2nd_t : perfect_forward<discard2nd_op<F>> {
      using perfect_forward<discard2nd_op<F>>::perfect_forward;
    };

    template <class F>
    struct discard1st_t : perfect_forward<discard1st_op<F>> {
      using perfect_forward<discard1st_op<F>>::perfect_forward;
    };
  } // namespace detail

  inline namespace cpo {
    /// pure   :: a -> f a
    template <class F>
    inline constexpr detail::pure_t<std::remove_cvref_t<F>> pure{};

    /// (<*>)  :: f (a -> b) -> f a -> f b -- infixl 4
    template <class F>
    inline constexpr detail::seq_apply_t<std::remove_cvref_t<F>> seq_apply{};

    /// liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    template <class F>
    inline constexpr detail::liftA2_t<std::remove_cvref_t<F>> liftA2{};

    /// ( *>)  :: f a -> f b -> f b -- infixl 4
    template <class F>
    inline constexpr detail::discard2nd_t<std::remove_cvref_t<F>> discard2nd{};

    /// (<* )  :: f a -> f b -> f a -- infixl 4
    template <class F>
    inline constexpr detail::discard1st_t<std::remove_cvref_t<F>> discard1st{};
  } // namespace cpo

  namespace applicatives {
    namespace detail {
      // clang-format off
      /// fmap :: (a -> b) -> f a -> f b
      /// fmap f x = seq_apply (pure f) x
      template<applicative_traits_specialized F>
      struct fmap_op {
        template<class A1, class A2>
        constexpr auto operator()(A1&& f, A2&& x) const
        noexcept(noexcept(mpc::seq_apply<F>(mpc::pure<F>(std::forward<A1>(f)), std::forward<A2>(x))))
        -> decltype(      mpc::seq_apply<F>(mpc::pure<F>(std::forward<A1>(f)), std::forward<A2>(x)))
        { return          mpc::seq_apply<F>(mpc::pure<F>(std::forward<A1>(f)), std::forward<A2>(x)); }
      };

      /// liftA2 :: (a -> b -> c) -> f a -> f b -> f c
      /// liftA2 f x y = seq_apply (fmap f x) y
      template<class F>
      requires requires {
        functor_traits<std::remove_cvref_t<F>>::fmap;
        applicative_traits<std::remove_cvref_t<F>>::seq_apply;
      }
      struct liftA2_op {
        template<class A1, class A2, class A3>
        constexpr auto operator()(A1&& f, A2&& x, A3&& y) const
        noexcept(noexcept(mpc::seq_apply<F>(mpc::fmap<F>(std::forward<A1>(f), std::forward<A2>(x)), std::forward<A3>(y))))
        -> decltype(      mpc::seq_apply<F>(mpc::fmap<F>(std::forward<A1>(f), std::forward<A2>(x)), std::forward<A3>(y)))
        { return          mpc::seq_apply<F>(mpc::fmap<F>(std::forward<A1>(f), std::forward<A2>(x)), std::forward<A3>(y)); }
      };

      /// discard1st_opt :: f a -> f b -> f b
      /// discard1st_opt x y = (id <$ x) <*> y
      template<class F>
      requires requires {
        functor_traits<std::remove_cvref_t<F>>::replace2nd;
        applicative_traits<std::remove_cvref_t<F>>::seq_apply;
      }
      struct discard1st_opt_op {
        template<class A1, class A2>
        constexpr auto operator()(A1&& x, A2&& y) const
        noexcept(noexcept(mpc::seq_apply<F>(mpc::replace2nd<F>(id, std::forward<A1>(x)), std::forward<A2>(y))))
        -> decltype(      mpc::seq_apply<F>(mpc::replace2nd<F>(id, std::forward<A1>(x)), std::forward<A2>(y)))
        { return          mpc::seq_apply<F>(mpc::replace2nd<F>(id, std::forward<A1>(x)), std::forward<A2>(y)); }
      };

      template <applicative_traits_specialized F>
      struct fmap_t : perfect_forward<fmap_op<F>> {
        using perfect_forward<fmap_op<F>>::perfect_forward;
      };

      template <class F>
      requires requires {
        functor_traits<std::remove_cvref_t<F>>::fmap;
        applicative_traits<std::remove_cvref_t<F>>::seq_apply;
      }
      struct liftA2_t : perfect_forward<liftA2_op<F>> {
        using perfect_forward<liftA2_op<F>>::perfect_forward;
      };

      template <class F>
      requires requires {
        functor_traits<std::remove_cvref_t<F>>::replace2nd;
        applicative_traits<std::remove_cvref_t<F>>::seq_apply;
      }
      struct discard1st_opt_t : perfect_forward<discard1st_opt_op<F>> {
        using perfect_forward<discard1st_opt_op<F>>::perfect_forward;
      };
      // clang-format on
    } // namespace detail

    /// @brief fmap f x = seq_apply (pure f) x
    /// @details If you fully specialize `applicative_traits<F>`, you can deduce `fmap`.
    template <applicative_traits_specialized F>
    inline constexpr detail::fmap_t<std::remove_cvref_t<F>> fmap{};

    /// @brief seq_apply = liftA2 id
    /// @details If you define `applicative_traits<F>::liftA2`, you can deduce `seq_apply`.
    template <class F>
    requires requires {
      applicative_traits<std::remove_cvref_t<F>>::liftA2;
    }
    inline constexpr auto seq_apply = mpc::liftA2<F> % id;

    /// @brief liftA2 f x y = seq_apply (fmap f x) y
    /// @details If you define `applicative_traits<F>::seq_apply`, you can deduce `liftA2`.
    template <class F>
    requires requires {
      functor_traits<std::remove_cvref_t<F>>::fmap;
      applicative_traits<std::remove_cvref_t<F>>::seq_apply;
    }
    inline constexpr detail::liftA2_t<std::remove_cvref_t<F>> liftA2{};

    /// @brief discard2nd = liftA2 const
    /// @details If you define `applicative_traits<F>::seq_apply` and
    /// `applicative_traits<F>::liftA2`, you can deduce `discard2nd`.
    template <class F>
    requires requires {
      applicative_traits<std::remove_cvref_t<F>>::liftA2;
    }
    inline constexpr auto discard2nd = mpc::liftA2<F> % constant;

    /// @brief discard1st = liftA2 (flip const)
    /// @details If you define `applicative_traits<F>::seq_apply` and
    /// `applicative_traits<F>::liftA2`, you can deduce `discard1st`.
    template <class F>
    requires requires {
      applicative_traits<std::remove_cvref_t<F>>::liftA2;
    }
    inline constexpr auto discard1st = mpc::liftA2<F> % (flip % constant);

    /// @brief discard1st_opt x y = seq_apply (id <$ x) y
    /// @details If you optimize `functor_traits<F>::replace2nd`, you can deduce optimized
    /// `discard1st`.
    template <class F>
    requires requires {
      functor_traits<std::remove_cvref_t<F>>::replace2nd;
      applicative_traits<std::remove_cvref_t<F>>::seq_apply;
    }
    inline constexpr detail::discard1st_opt_t<std::remove_cvref_t<F>> discard1st_opt{};
  } // namespace applicatives

  // Grobal methods

  namespace detail {
    // clang-format off
    template<applicative F>
    struct liftA3_op {
      /// liftA3 :: Applicative f => (a -> b -> c -> d) -> f a -> f b -> f c -> f d
      /// liftA3 f a b c = liftA2 f a b <*> c
      template<class A1, class A2, class A3, class A4>
      constexpr auto operator()(A1&& f, A2&& a, A3&& b, A4&& c) const
      noexcept(noexcept(mpc::seq_apply<F>(mpc::liftA2<F>(std::forward<A1>(f), std::forward<A2>(a), std::forward<A3>(b)), std::forward<A4>(c))))
      -> decltype(      mpc::seq_apply<F>(mpc::liftA2<F>(std::forward<A1>(f), std::forward<A2>(a), std::forward<A3>(b)), std::forward<A4>(c)))
      { return          mpc::seq_apply<F>(mpc::liftA2<F>(std::forward<A1>(f), std::forward<A2>(a), std::forward<A3>(b)), std::forward<A4>(c)); }
    };
    // clang-format on
  } // namespace detail

  inline namespace cpo {
    /// liftA3 f a b c = liftA2 f a b <*> c
    template <applicative F>
    inline constexpr perfect_forwarded_t<detail::liftA3_op<std::remove_cvref_t<F>>> liftA3{};

    // liftA4 f a b c d = liftA3 f a b c <*> d
    // liftA5 f a b c d e = liftA4 f a b c d <*> e
  } // namespace cpo
} // namespace mpc
