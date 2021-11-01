/// @file applicative.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional/constant.hpp>
#include <mpc/functional/flip.hpp>
#include <mpc/functional/identity.hpp>
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/control/basic_traits.hpp>

namespace mpc {
  // clang-format off
  template <class F>
  struct applicative_traits {
  private:
    using F2 = std::remove_cvref_t<F>;

  public:
    // pure :: a -> f a
    static constexpr auto pure(auto&& a)
      noexcept(noexcept(basic_applicative_traits<F2>::pure(std::forward<decltype(a)>(a))))
      -> decltype(      basic_applicative_traits<F2>::pure(std::forward<decltype(a)>(a))) {
      return            basic_applicative_traits<F2>::pure(std::forward<decltype(a)>(a));
    }

    // pure = returns
    static constexpr auto pure(auto&& a)
      noexcept(noexcept(monad_traits<F2>::returns(std::forward<decltype(a)>(a))))
      -> decltype(      monad_traits<F2>::returns(std::forward<decltype(a)>(a)))
      requires (not requires { basic_applicative_traits<F2>::pure(std::forward<decltype(a)>(a)); }) {
      return            monad_traits<F2>::returns(std::forward<decltype(a)>(a));
    }

    // seq_apply :: f (a -> b) -> f a -> f b
    static constexpr auto seq_apply(auto&& a1, auto&& a2)
      noexcept(noexcept(basic_applicative_traits<F2>::seq_apply(std::forward<decltype(a1)>(a1),
                                                                std::forward<decltype(a2)>(a2))))
      -> decltype(      basic_applicative_traits<F2>::seq_apply(std::forward<decltype(a1)>(a1),
                                                                std::forward<decltype(a2)>(a2))) {
      return            basic_applicative_traits<F2>::seq_apply(std::forward<decltype(a1)>(a1),
                                                                std::forward<decltype(a2)>(a2));
    }

    // seq_apply a1 a2 = bind a1 (f -> bind a2 (x -> return (f x)))
    static constexpr auto seq_apply(auto&& a1, auto&& a2)
    requires (
      not requires { basic_applicative_traits<F2>::seq_apply(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2)); } and
          requires { monad_traits<F2>::bind; }
    ) {
      return monad_traits<F2>::bind(
        std::forward<decltype(a1)>(a1),
        [a2 = std::forward<decltype(a2)>(a2)](auto&& f) {
          return monad_traits<F2>::bind(
            std::forward<decltype(a2)>(a2),
            [f = std::forward<decltype(f)>(f)](auto&& x) {
              return std::invoke(std::forward<decltype(f)>(f),
                                  std::forward<decltype(x)>(x));
            }
          );
        }
      );
    }

    // seq_apply = liftA2 id
    static constexpr auto seq_apply(auto&& a1, auto&& a2)
    requires (
      not requires { basic_applicative_traits<F2>::seq_apply(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2)); } and
      not requires { monad_traits<F2>::bind; } and
          requires { basic_applicative_traits<F2>::liftA2; }
    ) {
      return basic_applicative_traits<F2>::liftA2(Identity,
                                                  std::forward<decltype(a1)>(a1),
                                                  std::forward<decltype(a2)>(a2));
    }

    // liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    static constexpr auto liftA2(auto&& a1, auto&& a2, auto&& a3)
      noexcept(noexcept(basic_applicative_traits<F2>::liftA2(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2),
                                                             std::forward<decltype(a3)>(a3))))
      -> decltype(      basic_applicative_traits<F2>::liftA2(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2),
                                                             std::forward<decltype(a3)>(a3))) {
      return            basic_applicative_traits<F2>::liftA2(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2),
                                                             std::forward<decltype(a3)>(a3));
    }

    // liftA2 f x y = seq_apply (fmap f x) y
    static constexpr auto liftA2(auto&& a1, auto&& a2, auto&& a3)
    requires (
      not requires { basic_applicative_traits<F2>::liftA2(std::forward<decltype(a1)>(a1),
                                                          std::forward<decltype(a2)>(a2),
                                                          std::forward<decltype(a3)>(a3)); } and
      // Use `applicative_traits` instead of `basic_applicative_traits` because `seq_apply` may be defined by `monad_traits`
          requires { applicative_traits<F2>::seq_apply; }
    ) {
      return applicative_traits<F2>::seq_apply(functor_traits<F2>::fmap(std::forward<decltype(a1)>(a1),
                                                                        std::forward<decltype(a2)>(a2)),
                                                std::forward<decltype(a3)>(a3));
    }

    // discard2nd :: f a -> f b -> f a
    static constexpr auto discard2nd(auto&& a1, auto&& a2)
      noexcept(noexcept(basic_applicative_traits<F2>::discard2nd(std::forward<decltype(a1)>(a1),
                                                                 std::forward<decltype(a2)>(a2))))
      -> decltype(      basic_applicative_traits<F2>::discard2nd(std::forward<decltype(a1)>(a1),
                                                                 std::forward<decltype(a2)>(a2))) {
      return            basic_applicative_traits<F2>::discard2nd(std::forward<decltype(a1)>(a1),
                                                                 std::forward<decltype(a2)>(a2));
    }

    // discard2nd = liftA2 const
    static constexpr auto discard2nd(auto&& a1, auto&& a2)
    requires (
      not requires { basic_applicative_traits<F2>::discard2nd(std::forward<decltype(a1)>(a1),
                                                              std::forward<decltype(a2)>(a2)); } and
          requires { applicative_traits<F2>::liftA2; }
    ) {
      return applicative_traits<F2>::liftA2(constant,
                                            std::forward<decltype(a1)>(a1),
                                            std::forward<decltype(a2)>(a2));
    }

    // discard1st :: f a -> f b -> f b
    static constexpr auto discard1st(auto&& a1, auto&& a2)
      noexcept(noexcept(basic_applicative_traits<F2>::discard1st(std::forward<decltype(a1)>(a1),
                                                                 std::forward<decltype(a2)>(a2))))
      -> decltype(      basic_applicative_traits<F2>::discard1st(std::forward<decltype(a1)>(a1),
                                                                 std::forward<decltype(a2)>(a2))) {
      return            basic_applicative_traits<F2>::discard1st(std::forward<decltype(a1)>(a1),
                                                                 std::forward<decltype(a2)>(a2));
    }

    // TODO: discard1st a1 a2 = (id <$ a1) <*> a2 if (<$) is optimized???

    // discard1st = liftA2 (flip const)
    static constexpr auto discard1st(auto&& a1, auto&& a2)
    requires (
      not requires { basic_applicative_traits<F2>::discard1st(std::forward<decltype(a1)>(a1),
                                                              std::forward<decltype(a2)>(a2)); } and
          requires { applicative_traits<F2>::liftA2; }
    ) {
      return applicative_traits<F2>::liftA2(flip(constant),
                                            std::forward<decltype(a1)>(a1),
                                            std::forward<decltype(a2)>(a2));
    }
  }; // struct applicable_traits

  namespace detail {
    template <class F, class F2 = std::remove_cvref_t<F>>
    struct pure_op {
      constexpr auto operator()(auto&& a) const
        noexcept(noexcept(applicative_traits<F2>::pure(std::forward<decltype(a)>(a))))
        -> decltype(      applicative_traits<F2>::pure(std::forward<decltype(a)>(a))) {
        return            applicative_traits<F2>::pure(std::forward<decltype(a)>(a));
      }
    };

    template <class F>
    struct pure_t : perfect_forward<pure_op<F>> {
      using mpc::perfect_forward<pure_op<F>>::perfect_forward;
    };

    template <class F, class F2 = std::remove_cvref_t<F>>
    struct seq_apply_op {
      constexpr auto operator()(auto&& a1, auto&& a2) const
        noexcept(noexcept(applicative_traits<F2>::seq_apply(std::forward<decltype(a1)>(a1),
                                                            std::forward<decltype(a2)>(a2))))
        -> decltype(      applicative_traits<F2>::seq_apply(std::forward<decltype(a1)>(a1),
                                                            std::forward<decltype(a2)>(a2))) {
        return            applicative_traits<F2>::seq_apply(std::forward<decltype(a1)>(a1),
                                                            std::forward<decltype(a2)>(a2));
      }
    };

    template <class F>
    struct seq_apply_t : perfect_forward<seq_apply_op<F>> {
      using mpc::perfect_forward<seq_apply_op<F>>::perfect_forward;
    };

    template <class F, class F2 = std::remove_cvref_t<F>>
    struct liftA2_op {
      constexpr auto operator()(auto&& a1, auto&& a2, auto&& a3) const
        noexcept(noexcept(applicative_traits<F2>::liftA2(std::forward<decltype(a1)>(a1),
                                                         std::forward<decltype(a2)>(a2),
                                                         std::forward<decltype(a3)>(a3))))
        -> decltype(      applicative_traits<F2>::liftA2(std::forward<decltype(a1)>(a1),
                                                         std::forward<decltype(a2)>(a2),
                                                         std::forward<decltype(a3)>(a3))) {
        return            applicative_traits<F2>::liftA2(std::forward<decltype(a1)>(a1),
                                                         std::forward<decltype(a2)>(a2),
                                                         std::forward<decltype(a3)>(a3));
      }
    };

    template <class F>
    struct liftA2_t : perfect_forward<liftA2_op<F>> {
      using mpc::perfect_forward<liftA2_op<F>>::perfect_forward;
    };

    template <class F, class F2 = std::remove_cvref_t<F>>
    struct discard2nd_op {
      constexpr auto operator()(auto&& a1, auto&& a2) const
        noexcept(noexcept(applicative_traits<F2>::discard2nd(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2))))
        -> decltype(      applicative_traits<F2>::discard2nd(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2))) {
        return            applicative_traits<F2>::discard2nd(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2));
      }
    };

    template <class F>
    struct discard2nd_t : perfect_forward<discard2nd_op<F>> {
      using mpc::perfect_forward<discard2nd_op<F>>::perfect_forward;
    };

    template <class F, class F2 = std::remove_cvref_t<F>>
    struct discard1st_op {
      constexpr auto operator()(auto&& a1, auto&& a2) const
        noexcept(noexcept(applicative_traits<F2>::discard1st(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2))))
        -> decltype(      applicative_traits<F2>::discard1st(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2))) {
        return            applicative_traits<F2>::discard1st(std::forward<decltype(a1)>(a1),
                                                             std::forward<decltype(a2)>(a2));
      }
    };

    template <class F>
    struct discard1st_t : perfect_forward<discard1st_op<F>> {
      using mpc::perfect_forward<discard1st_op<F>>::perfect_forward;
    };
  } // namespace detail
  // clang-format on

  inline namespace cpo {
    template<class F>
    inline constexpr detail::pure_t<F> pure;
    template<class F>
    inline constexpr detail::seq_apply_t<F> seq_apply;
    template<class F>
    inline constexpr detail::liftA2_t<F> liftA2;
    template<class F>
    inline constexpr detail::discard2nd_t<F> discard2nd;
    template<class F>
    inline constexpr detail::discard1st_t<F> discard1st;
  }
} // namespace mpc
