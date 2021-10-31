/// @file tuple_like.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc::detail::tuple_like {
  template <auto>
  void get(auto&) = delete;
  template <auto>
  void get(const auto&) = delete;

  template <class T, class = std::make_index_sequence<std::tuple_size_v<T>>, class = void>
  struct has_tuple_element : std::false_type {};

  template <class T, std::size_t... Idx>
  struct has_tuple_element<T, std::index_sequence<Idx...>,
                           std::void_t<typename std::tuple_element<Idx, T>::type...>>
    : std::true_type {};

  template <class T, class = std::make_index_sequence<std::tuple_size_v<T>>, class = void>
  struct has_unqualified_get : std::false_type {};

  template <class T, std::size_t... Idx>
  struct has_unqualified_get<T, std::index_sequence<Idx...>,
                             std::void_t<decltype(get<Idx>(std::declval<T>()))...>>
    : std::true_type {};
} // namespace mpc::detail::tuple_like

namespace mpc {
  /// %is_tuple_like
  template <class T, class = void>
  struct is_tuple_like : std::false_type {};

  /// partial specialization of `is_tuple_like`
  template <class T>
  struct is_tuple_like<T, std::void_t<decltype(std::tuple_size<T>::value)>>
    : _and<detail::tuple_like::has_tuple_element<T>, detail::tuple_like::has_unqualified_get<T>> {};

  /// helper variable template for `is_tuple_like`
  template <class T>
  inline constexpr bool is_tuple_like_v = is_tuple_like<T>::value;

  /// tuple_like
  template <class T>
  concept tuple_like = is_tuple_like_v<std::remove_cvref_t<T>>;
} // namespace mpc
