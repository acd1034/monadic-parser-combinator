/// @file alternative_value_t.hpp
#pragma once
#include <variant>
#include <mpc/stdfundamental.hpp>

namespace mpc {
  /// alternative_value_t
  template <std::size_t Idx, class Variant>
  requires requires {
    typename std::variant_alternative_t<Idx, std::remove_cvref_t<Variant>>::value_type;
  }
  using alternative_value_t =
    typename std::variant_alternative_t<Idx, std::remove_cvref_t<Variant>>::value_type;
} // namespace mpc
