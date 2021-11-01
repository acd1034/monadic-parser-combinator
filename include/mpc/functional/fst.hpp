/// @file fst.hpp
#pragma once
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  namespace detail::_fst {
    template <auto>
    void get(auto&) = delete;
    template <auto>
    void get(const auto&) = delete;

    template <std::size_t Idx>
    struct get_op {
      template <class T>
      constexpr auto operator()(T&& t) const             //
        noexcept(noexcept(get<Idx>(std::forward<T>(t)))) //
        -> decltype(get<Idx>(std::forward<T>(t))) {
        return get<Idx>(std::forward<T>(t));
      }
    };
  } // namespace detail::_fst

  inline namespace cpo {
    inline constexpr perfect_forwarded_t<detail::_fst::get_op<0>> fst;
    inline constexpr perfect_forwarded_t<detail::_fst::get_op<1>> snd;
  } // namespace cpo
} // namespace mpc
