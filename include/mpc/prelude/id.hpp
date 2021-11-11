/// @file id.hpp
#pragma once
#include <functional> // std::identity
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  inline namespace cpo {
    inline constexpr perfect_forwarded_t<std::identity> id;
  } // namespace cpo
} // namespace mpc
