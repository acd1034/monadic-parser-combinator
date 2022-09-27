/// @file single.hpp
#pragma once
#include <mpc/type_traits.hpp>
#include <mpc/utility/tuple_like.hpp>

// clang-format off

namespace mpc {
  /**
   * @brief A class that holds a single value.
   * @details To prepare multiple classes with different types but the same
   * properties, the tag type can be assigned to the second type parameter.
   */
  template <class T, class Tag = void>
  struct single {
  private:
    T instance_{};

  public:
    // methods:
    // [x] value_type
    // [x] ctors
    // [x] assignment ops
    // [x] operator<=>
    // [x] swap
    // [x] operator*
    // [x] operator->

    using value_type = T;

    // ctors

    // (1)
    explicit(not detail::is_implicitly_default_constructible_v<T>)
    constexpr single()
      requires std::default_initializable<T> =default;

    // (2)
    single(const single&) = default;

    // (3)
    single(single&&) = default;

    // (4)
    explicit(not std::is_convertible_v<const T&, T>)
    constexpr single(const T& rhs)
      requires std::copy_constructible<T>
      : instance_(rhs) {}

    // (5)
    template <class U = T>
    requires std::constructible_from<T, U&&>
    explicit(not std::is_convertible_v<U, T>)
    constexpr single(U&& rhs)
      : instance_(std::forward<U>(rhs)) {}

    // (6)
    template <tuple_like Tuple>
    requires (
      not std::constructible_from<T, Tuple&&> and
      std::constructible_from<T, detail::copy_reference_t<Tuple, std::tuple_element_t<0, Tuple>>&&>
    )
    explicit(not std::is_convertible_v<std::tuple_element_t<0, Tuple>, T>)
    constexpr single(Tuple&& rhs)
      : instance_(get<0>(std::forward<Tuple>(rhs))) {}

    // (7)
    template <class... Args>
    requires std::constructible_from<T, Args&&...>
    constexpr single(std::in_place_t, Args&&... args)
      : instance_(std::forward<Args>(args)...) {}

    // (8)
    template <class U, class... Args>
    requires std::constructible_from<T, std::initializer_list<U>, Args&&...>
    constexpr single(std::in_place_t, std::initializer_list<U> il, Args&&... args)
      : instance_(il, std::forward<Args>(args)...) {}

    // assignment ops.

    // (1)
    constexpr single& operator=(const single& rhs)
      requires std::is_copy_assignable_v<T> {
      instance_ = *rhs;
      return *this;
    }

    // (2)
    constexpr single& operator=(single&& rhs)
      noexcept(std::is_nothrow_move_assignable_v<T>)
      requires std::is_move_assignable_v<T> {
      instance_ = *std::move(rhs);
      return *this;
    }

    // (3)
    template <std::convertible_to<T> U = T>
    single& operator=(U&& rhs) {
      instance_ = std::forward<U>(rhs);
      return *this;
    }

    // (4)
    template <tuple_like Tuple>
    requires (
      not std::convertible_to<Tuple, T> and
      std::convertible_to<std::tuple_element_t<0, Tuple>, T>
    )
    constexpr single& operator=(Tuple&& rhs) {
      instance_ = get<0>(std::forward<Tuple>(rhs));
      return *this;
    }

    // operator <=>

    auto operator<=>(const single&) const
      requires std::equality_comparable<T> = default;

    // swap

    void swap(single& other)
      noexcept(std::is_nothrow_swappable_v<T>)
      requires std::swappable<T> {
      using std::swap;
      swap(instance_, other.instance_);
    }

    // operator*

    constexpr decltype(auto) operator*() & {
      return instance_;
    }

    constexpr decltype(auto) operator*() const& {
      return instance_;
    }

    constexpr decltype(auto) operator*() && {
      return std::move(instance_);
    }

    constexpr decltype(auto) operator*() const&& {
      return std::move(instance_);
    }

    // operator->

    constexpr auto operator->() {
      return std::addressof(instance_);
    }

    constexpr auto operator->() const {
      return std::addressof(instance_);
    }
  }; // struct single

  // grobal methods:
  // [x] deduction guides
  // [x] swap
  // [x] get

  /// @dguide @ref single
  template <class T, class Tag = void>
  single(T) -> single<T>;

  /// swap for @ref single
  template <std::swappable T, class Tag>
  void swap(single<T, Tag>& lhs, single<T, Tag>& rhs)
    noexcept(std::is_nothrow_swappable_v<T>) {
    lhs.swap(rhs);
  }

  /// get for @ref single
  template <std::size_t Idx, class T, class Tag>
  requires (Idx < 1)
  constexpr decltype(auto) get(single<T, Tag>& s) {
    return *s;
  }

  /// @overl get
  template <std::size_t Idx, class T, class Tag>
  requires (Idx < 1)
  constexpr decltype(auto) get(const single<T, Tag>& s) {
    return *s;
  }

  /// @overl get
  template <std::size_t Idx, class T, class Tag>
  requires (Idx < 1)
  constexpr decltype(auto) get(single<T, Tag>&& s) {
    return *std::move(s);
  }

  /// @overl get
  template <std::size_t Idx, class T, class Tag>
  requires (Idx < 1)
  constexpr decltype(auto) get(const single<T, Tag>&& s) {
    return *std::move(s);
  }
} // namespace mpc

// tuple-like methods:
// [x] std::tuple_size
// [x] std::tuple_element

/// @spec std::tuple_size for mpc::single
template <class T, class Tag>
struct std::tuple_size<mpc::single<T, Tag>> : mpc::index_constant<1> {};

/// @spec std::tuple_element for mpc::single
template <std::size_t Idx, class T, class Tag>
requires (Idx < 1)
struct std::tuple_element<Idx, mpc::single<T, Tag>> {
  using type = T;
};

// clang-format on
