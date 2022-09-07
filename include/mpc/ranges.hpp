/// @file ranges.hpp
#pragma once
#include <iterator>

#if defined(__cpp_lib_ranges)

#include <ranges>

namespace mpc::ranges::inline cpo {
  // clang-format off
  using std::ranges::begin,
    std::ranges::end,
    std::ranges::range,
    std::ranges::input_range,
    std::ranges::iterator_t,std::ranges::sentinel_t,
    std::ranges::range_difference_t,
    std::ranges::range_value_t,
    std::ranges::range_reference_t,
    std::ranges::range_rvalue_reference_t;
  // clang-format on
}

#else

namespace mpc::ranges::detail {
  using std::begin, std::end;

  struct begin_fn {
    template <class C>
    constexpr auto operator()(C&& c) noexcept(noexcept(begin(c))) -> decltype(begin(c)) {
      return begin(c);
    }
  };

  struct end_fn {
    template <class C>
    constexpr auto operator()(C&& c) noexcept(noexcept(end(c))) -> decltype(end(c)) {
      return end(c);
    }
  };
} // namespace mpc::ranges::detail

namespace mpc::ranges::inline cpo {
  inline constexpr detail::begin_fn begin{};
  inline constexpr detail::end_fn end{};
} // namespace mpc::ranges::inline cpo

namespace mpc::ranges {
  template <class T>
  concept range = requires(T& t) {
    ranges::begin(t); // equality-preserving for forward iterators
    ranges::end(t);
  };

  template <class T>
  using iterator_t = decltype(ranges::begin(std::declval<T&>()));

  template <ranges::range R>
  using sentinel_t = decltype(ranges::end(std::declval<R&>()));

  template <ranges::range R>
  using range_difference_t = std::iter_difference_t<ranges::iterator_t<R>>;

  template <ranges::range R>
  using range_value_t = std::iter_value_t<ranges::iterator_t<R>>;

  template <ranges::range R>
  using range_reference_t = std::iter_reference_t<ranges::iterator_t<R>>;

  template <ranges::range R>
  using range_rvalue_reference_t = std::iter_rvalue_reference_t<ranges::iterator_t<R>>;

  template <class T>
  concept input_range = ranges::range<T> and std::input_iterator<ranges::iterator_t<T>>;
} // namespace mpc::ranges

#endif
