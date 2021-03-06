/// @file tagged_union.hpp
#pragma once
#include <variant>
#include <mpc/utility/single.hpp>

namespace mpc {
  template <class T, std::size_t Idx>
  using nth_element_t = single<T, index_constant<Idx>>;

  namespace detail {
    template <class, class...>
    struct tagged_union_impl;

    template <std::size_t... Idx, class... Args>
    struct tagged_union_impl<std::index_sequence<Idx...>, Args...> {
      using type = std::variant<nth_element_t<Args, Idx>...>;
    };
  } // namespace detail

  template <class... Args>
  using tagged_union =
    typename detail::tagged_union_impl<std::index_sequence_for<Args...>, Args...>::type;

  template <std::size_t Idx, class T>
  constexpr nth_element_t<T, Idx> make_nth_element(T&& t) {
    return std::forward<T>(t);
  }
} // namespace mpc
