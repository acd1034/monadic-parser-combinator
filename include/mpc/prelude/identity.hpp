/// @file identity.hpp
#pragma once
#include <functional> // std::identity
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  inline namespace cpo {
    /**
     * @brief %Identity mapping.
     */
    inline constexpr partially_applicable<std::identity> identity{};
  } // namespace cpo
} // namespace mpc
