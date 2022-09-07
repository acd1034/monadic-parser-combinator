/// @file holding.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc {
  template <class>
  struct holding;

  template <class T>
  using holding_t = typename holding<T>::type;

  template <class T, class U>
  struct holding_or : std::type_identity<U> {};

  template <class T, class U>
  requires requires { typename holding<std::remove_cvref_t<T>>::type; }
  struct holding_or<T, U> : holding<std::remove_cvref_t<T>> {};

  template <class T, class U>
  using holding_or_t = typename holding_or<T, U>::type;
} // namespace mpc
