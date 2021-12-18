/// @file unit.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc {
  /// The type of an empty tuple.
  using unit_t = std::tuple<>;

  /// An entity of an empty tuple.
  inline constexpr unit_t unit;
} // namespace mpc
