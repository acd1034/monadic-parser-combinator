/// @file monad.hpp
#pragma once
#include <mpc/control/basic_traits.hpp>
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  // clang-format off
  template <class M>
  struct monad_traits {
  private:
    using M2 = std::remove_cvref_t<M>;

  public:
    // return :: a -> m a
    static constexpr auto returns(auto&& a)
      noexcept(noexcept(basic_monad_traits<M2>::returns(std::forward<decltype(a)>(a))))
      -> decltype(      basic_monad_traits<M2>::returns(std::forward<decltype(a)>(a))) {
      return            basic_monad_traits<M2>::returns(std::forward<decltype(a)>(a));
    }

    // bind :: forall a b. m a -> (a -> m b) -> m b
    static constexpr auto bind(auto&& a1, auto&& a2)
      noexcept(noexcept(basic_monad_traits<M2>::bind(std::forward<decltype(a1)>(a1),
                                                     std::forward<decltype(a2)>(a2))))
      -> decltype(      basic_monad_traits<M2>::bind(std::forward<decltype(a1)>(a1),
                                                     std::forward<decltype(a2)>(a2))) {
      return            basic_monad_traits<M2>::bind(std::forward<decltype(a1)>(a1),
                                                     std::forward<decltype(a2)>(a2));
    }
  }; // struct monad_traits

  namespace detail {
    template <class M, class M2 = std::remove_cvref_t<M>>
    struct returns_op {
      constexpr auto operator()(auto&& a) const
        noexcept(noexcept(monad_traits<M2>::returns(std::forward<decltype(a)>(a))))
        -> decltype(      monad_traits<M2>::returns(std::forward<decltype(a)>(a))) {
        return            monad_traits<M2>::returns(std::forward<decltype(a)>(a));
      }
    };

    template<class M>
    struct returns_t : perfect_forward<returns_op<M>> {
      using mpc::perfect_forward<returns_op<M>>::perfect_forward;
    };

    template <class M, class M2 = std::remove_cvref_t<M>>
    struct bind_op {
      constexpr auto operator()(auto&& a1, auto&& a2) const
        noexcept(noexcept(monad_traits<M2>::bind(std::forward<decltype(a1)>(a1),
                                                 std::forward<decltype(a2)>(a2))))
        -> decltype(      monad_traits<M2>::bind(std::forward<decltype(a1)>(a1),
                                                 std::forward<decltype(a2)>(a2))) {
        return            monad_traits<M2>::bind(std::forward<decltype(a1)>(a1),
                                                 std::forward<decltype(a2)>(a2));
      }
    };

    template<class M>
    struct bind_t : perfect_forward<bind_op<M>> {
      using mpc::perfect_forward<bind_op<M>>::perfect_forward;
    };
  }
  // clang-format on

  inline namespace cpo {
    template <class M>
    inline constexpr detail::returns_t<M> returns;
    template <class M>
    inline constexpr detail::bind_t<M> bind;
  } // namespace cpo
} // namespace mpc
