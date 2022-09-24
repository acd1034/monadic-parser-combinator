/// @file forward_like.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc {
  template <class T, class U>
  using __override_ref_t =
    std::conditional_t<std::is_rvalue_reference_v<T>, std::remove_reference_t<U>&&, U&>;

  template <class T, class U>
  using __copy_const_t =
    std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, U const, U>;

  template <class T, class U>
  using forward_like_t = __override_ref_t<T&&, __copy_const_t<T, std::remove_reference_t<U>>>;

  template <typename T>
  [[nodiscard]] constexpr auto forward_like(auto&& x) noexcept -> forward_like_t<T, decltype(x)> {
    return static_cast<forward_like_t<T, decltype(x)>>(x);
  }
} // namespace mpc
