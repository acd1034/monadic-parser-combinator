/// @file functor.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/control/basic_traits.hpp>

namespace mpc {
  // clang-format off
  template <class F>
  struct functor_traits {
  private:
    using F2 = std::remove_cvref_t<F>;

  public:
    // fmap :: (a -> b) -> f a -> f b
    static constexpr auto fmap(auto&& a1, auto&& a2)
      noexcept(noexcept(basic_functor_traits<F2>::fmap(std::forward<decltype(a1)>(a1),
                                                       std::forward<decltype(a2)>(a2))))
      -> decltype(      basic_functor_traits<F2>::fmap(std::forward<decltype(a1)>(a1),
                                                       std::forward<decltype(a2)>(a2))) {
      return            basic_functor_traits<F2>::fmap(std::forward<decltype(a1)>(a1),
                                                       std::forward<decltype(a2)>(a2));
    }

    // fmap f x = bind x (y -> return (f y))
    static constexpr auto fmap(auto&& a1, auto&& a2)
    requires (
      not requires { basic_functor_traits<F2>::fmap(std::forward<decltype(a1)>(a1),
                                                    std::forward<decltype(a2)>(a2)); } and
          requires { monad_traits<F2>::returns; monad_traits<F2>::bind; }
    ) {
      return monad_traits<F2>::bind(
        std::forward<decltype(a2)>(a2),
        [a1 = std::forward<decltype(a1)>(a1)](auto&& b) {
          return monad_traits<F2>::returns(
            std::invoke(std::forward<decltype(a1)>(a1),
                        std::forward<decltype(b)>(b))
          );
        }
      );
    }

    // fmap f x = seq_apply (pure f) x
    static constexpr auto fmap(auto&& a1, auto&& a2)
    requires (
      not requires { basic_functor_traits<F2>::fmap(std::forward<decltype(a1)>(a1),
                                                    std::forward<decltype(a2)>(a2));  } and
      not requires { monad_traits<F2>::returns;    monad_traits<F2>::bind;            } and
          requires { applicative_traits<F2>::pure; applicative_traits<F2>::seq_apply; }
    ) {
      return applicative_traits<F2>::seq_apply(
        applicative_traits<F2>::pure(std::forward<decltype(a1)>(a1)),
        std::forward<decltype(a2)>(a2)
      );
    }
  }; // struct functor_traits

  namespace detail {
    template <class F, class F2 = std::remove_cvref_t<F>>
    struct fmap_op {
      constexpr auto operator()(auto&& a, auto&& b) const
        noexcept(noexcept(functor_traits<F2>::fmap(std::forward<decltype(a)>(a),
                                                   std::forward<decltype(b)>(b))))
        -> decltype(      functor_traits<F2>::fmap(std::forward<decltype(a)>(a),
                                                   std::forward<decltype(b)>(b))) {
        return            functor_traits<F2>::fmap(std::forward<decltype(a)>(a),
                                                   std::forward<decltype(b)>(b));
      }
    };

    template<class F>
    struct fmap_t : perfect_forward<fmap_op<F>> {
      using mpc::perfect_forward<fmap_op<F>>::perfect_forward;
    };
  } // namespace detail
  // clang-format on

  inline namespace cpo {
    template <class F>
    inline constexpr detail::fmap_t<F> fmap;
  } // namespace cpo
} // namespace mpc
