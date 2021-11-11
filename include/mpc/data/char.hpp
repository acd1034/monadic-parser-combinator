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
    inline constexpr perfect_forwarded_t<detail::isalnum_op> isalnum{};
    inline constexpr perfect_forwarded_t<detail::isalpha_op> isalpha{};
    inline constexpr perfect_forwarded_t<detail::islower_op> islower{};
    inline constexpr perfect_forwarded_t<detail::isupper_op> isupper{};
    inline constexpr perfect_forwarded_t<detail::isdigit_op> isdigit{};
    inline constexpr perfect_forwarded_t<detail::isxdigit_op> isxdigit{};
    inline constexpr perfect_forwarded_t<detail::iscntrl_op> iscntrl{};
    inline constexpr perfect_forwarded_t<detail::isgraph_op> isgraph{};
    inline constexpr perfect_forwarded_t<detail::isspace_op> isspace{};
    inline constexpr perfect_forwarded_t<detail::isblank_op> isblank{};
    inline constexpr perfect_forwarded_t<detail::isprint_op> isprint{};
    inline constexpr perfect_forwarded_t<detail::ispunct_op> ispunct{};
  } // namespace cpo

  template <class charT>
  std::basic_string<charT> quoted(charT s, charT delim = charT('\'')) {
    return {delim, s, delim};
  }

  template <class charT>
  auto quoted(const charT* s, charT delim = charT('"')) {
    return std::basic_string<charT>{delim} + s + delim;
  }

  template <class charT, class traits>
  auto quoted(std::basic_string_view<charT, traits> s, charT delim = charT('"')) {
    return delim + std::basic_string<charT, traits>{s} + delim;
  }

  template <class charT, class traits, class Allocator>
  auto quoted(const std::basic_string<charT, traits, Allocator>& s, charT delim = charT('"')) {
    return delim + s + delim;
  }

  template <class charT, class traits, class Allocator>
  auto quoted(std::basic_string<charT, traits, Allocator>&& s, charT delim = charT('"')) {
    return delim + std::move(s) + delim;
  }
} // namespace mpc
