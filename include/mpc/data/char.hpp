/// @file char.hpp
#pragma once
#include <cctype>
#include <string>
#include <mpc/prelude.hpp>

namespace mpc {
  namespace detail {
    // See also:
    // https://en.cppreference.com/w/cpp/string/byte/isalnum

    struct isalnum_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isalnum(c))) {
        return std::isalnum(c);
      }
    };

    struct isalpha_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isalpha(c))) {
        return std::isalpha(c);
      }
    };

    struct islower_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::islower(c))) {
        return std::islower(c);
      }
    };

    struct isupper_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isupper(c))) {
        return std::isupper(c);
      }
    };

    struct isdigit_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isdigit(c))) {
        return std::isdigit(c);
      }
    };

    struct isxdigit_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isxdigit(c))) {
        return std::isxdigit(c);
      }
    };

    struct iscntrl_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::iscntrl(c))) {
        return std::iscntrl(c);
      }
    };

    struct isgraph_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isgraph(c))) {
        return std::isgraph(c);
      }
    };

    struct isspace_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isspace(c))) {
        return std::isspace(c);
      }
    };

    struct isblank_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isblank(c))) {
        return std::isblank(c);
      }
    };

    struct isprint_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isprint(c))) {
        return std::isprint(c);
      }
    };

    struct ispunct_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::ispunct(c))) {
        return std::ispunct(c);
      }
    };
  } // namespace detail

  inline namespace cpo {
    /// Partially applicable std::isalnum.
    inline constexpr partially_applicable<detail::isalnum_op> isalnum{};
    /// Partially applicable std::isalpha.
    inline constexpr partially_applicable<detail::isalpha_op> isalpha{};
    /// Partially applicable std::islower.
    inline constexpr partially_applicable<detail::islower_op> islower{};
    /// Partially applicable std::isupper.
    inline constexpr partially_applicable<detail::isupper_op> isupper{};
    /// Partially applicable std::isdigit.
    inline constexpr partially_applicable<detail::isdigit_op> isdigit{};
    /// Partially applicable std::isxdigit.
    inline constexpr partially_applicable<detail::isxdigit_op> isxdigit{};
    /// Partially applicable std::iscntrl.
    inline constexpr partially_applicable<detail::iscntrl_op> iscntrl{};
    /// Partially applicable std::isgraph.
    inline constexpr partially_applicable<detail::isgraph_op> isgraph{};
    /// Partially applicable std::isspace.
    inline constexpr partially_applicable<detail::isspace_op> isspace{};
    /// Partially applicable std::isblank.
    inline constexpr partially_applicable<detail::isblank_op> isblank{};
    /// Partially applicable std::isprint.
    inline constexpr partially_applicable<detail::isprint_op> isprint{};
    /// Partially applicable std::ispunct.
    inline constexpr partially_applicable<detail::ispunct_op> ispunct{};
  } // namespace cpo

  /// Surrounds a string with delimiter.
  template <class charT>
  inline std::basic_string<charT> quoted(charT s, charT delim = charT('\'')) {
    return {delim, s, delim};
  }

  /// @overl quoted
  template <class charT>
  inline auto quoted(const charT* s, charT delim = charT('"')) {
    return std::basic_string<charT>{delim} + s + delim;
  }

  /// @overl quoted
  template <class charT, class traits>
  inline auto quoted(std::basic_string_view<charT, traits> s, charT delim = charT('"')) {
    return delim + std::basic_string<charT, traits>{s} + delim;
  }

  /// @overl quoted
  template <class charT, class traits, class Allocator>
  inline auto quoted(const std::basic_string<charT, traits, Allocator>& s,
                     charT delim = charT('"')) {
    return delim + s + delim;
  }

  /// @overl quoted
  template <class charT, class traits, class Allocator>
  inline auto quoted(std::basic_string<charT, traits, Allocator>&& s, charT delim = charT('"')) {
    return delim + std::move(s) + delim;
  }
} // namespace mpc
