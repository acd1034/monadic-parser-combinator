/// @file tuple_like.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

/// @cond undocumented
namespace mpc::detail::_tuple_like {
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
} // namespace mpc::detail::_tuple_like
/// @endcond undocumented

namespace mpc {
  /// %is_tuple_like
  template <class T, class = void>
  struct is_tuple_like : std::false_type {};

  /// @spec `is_tuple_like`
  template <class T>
  struct is_tuple_like<T, std::void_t<decltype(std::tuple_size<T>::value)>>
    : _and<detail::_tuple_like::has_tuple_element<T>, detail::_tuple_like::has_unqualified_get<T>> {};

  /// @ivar `is_tuple_like`
  template <class T>
  inline constexpr bool is_tuple_like_v = is_tuple_like<T>::value;

  /// Requires `std::tuple_size`, `std::tuple_element` and unqualified `get` is valid.
  template <class T>
  concept tuple_like = is_tuple_like_v<std::remove_cvref_t<T>>;
} // namespace mpc
