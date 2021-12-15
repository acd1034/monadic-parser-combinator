/// @file overloaded.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc {
  // overloaded
  // https://en.cppreference.com/w/cpp/utility/variant/visit

  /// %overloaded
  template <typename... Ts>
  struct overloaded : Ts... {
    using Ts::operator()...;
  };

  /// @dguide @ref overloaded
  template <typename... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;
} // namespace mpc
