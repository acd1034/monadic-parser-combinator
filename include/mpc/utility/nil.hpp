/// @file nil.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc {
  using nil_t = std::tuple<>;
  inline constexpr nil_t nil;
} // namespace mpc
