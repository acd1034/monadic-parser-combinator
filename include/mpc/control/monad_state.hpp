/// @file fname.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  template <class M>
  struct monad_state_traits;

  template <class M>
  concept monad_state_traits_specialized = requires {
    monad_state_traits<std::remove_cvref_t<M>>::gets;
    monad_state_traits<std::remove_cvref_t<M>>::puts;
    monad_state_traits<std::remove_cvref_t<M>>::states;
  };

  template <class M>
  concept monad_state = monad_state_traits_specialized<M>;

  namespace detail {
    template <class M>
    using puts_op = std::remove_cvref_t<decltype(monad_state_traits<std::remove_cvref_t<M>>::puts)>;

    template <class M>
    using states_op = std::remove_cvref_t<decltype(monad_state_traits<std::remove_cvref_t<M>>::states)>;
  }

  inline namespace cpo {
    template <class M>
    requires requires {
      monad_state_traits<std::remove_cvref_t<M>>::gets;
    }
    inline constexpr auto gets = monad_state_traits<std::remove_cvref_t<M>>::gets;

    template <class M>
    inline constexpr perfect_forwarded_t<detail::puts_op<std::remove_cvref_t<M>>> puts{};

    template <class M>
    inline constexpr perfect_forwarded_t<detail::states_op<std::remove_cvref_t<M>>> states{};
  } // namespace cpo
} // namespace mpc
