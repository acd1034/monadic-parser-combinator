/// @file identity.hpp
#pragma once
#include <functional> // std::identity
#include <mpc/functional/partial.hpp>

namespace mpc {
  inline namespace cpo {
    /**
     * @brief %Identity mapping.
     */
    inline constexpr partial<std::identity> identity{};
  } // namespace cpo
} // namespace mpc
