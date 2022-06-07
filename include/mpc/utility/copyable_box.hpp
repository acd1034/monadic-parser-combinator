/// @file copyable_box.hpp
#pragma once
#include <memory>
#include <optional>
#include <mpc/stdfundamental.hpp>

namespace mpc {
  // copyable_box
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__ranges/copyable_box.h

  // copyable_box allows turning a type that is copy-constructible (but maybe not copy-assignable)
  // into a type that is both copy-constructible and copy-assignable. It does that by introducing an
  // empty state and basically doing destroy-then-copy-construct in the assignment operator. The
  // empty state is necessary to handle the case where the copy construction fails after destroying
  // the object.
  //
  // In some cases, we can completely avoid the use of an empty state; we provide a specialization
  // of copyable_box that does this, see below for the details.

  /// Requires copy_constructible and is_object.
  template <class T>
  concept copy_constructible_object = std::copy_constructible<T> and std::is_object_v<T>;

  // Primary template - uses std::optional and introduces an empty state in case assignment fails.

  /// Makes copy_constructible but not copy_assignable types copy_assignable.
  template <copy_constructible_object T>
  class copyable_box {
    [[no_unique_address]] std::optional<T> instance_;

  public:
    template <class... Args>
    requires std::is_constructible_v<T, Args...>
    constexpr explicit copyable_box(std::in_place_t, Args&&... args) //
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : instance_(std::in_place, std::forward<Args>(args)...) {}

    constexpr copyable_box()                               //
      noexcept(std::is_nothrow_default_constructible_v<T>) //
      requires std::default_initializable<T> : instance_(std::in_place) {}

    constexpr copyable_box(const copyable_box&) = default;
    constexpr copyable_box(copyable_box&&) = default;

    constexpr copyable_box&
    operator=(const copyable_box& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
      if (this != std::addressof(other)) {
        if (other.has_value())
          instance_.emplace(*other);
        else
          instance_.reset();
      }
      return *this;
    }

    constexpr copyable_box& operator=(copyable_box&&) requires std::movable<T>
    = default;

    constexpr copyable_box&
    operator=(copyable_box&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
      if (this != std::addressof(other)) {
        if (other.has_value())
          instance_.emplace(std::move(*other));
        else
          instance_.reset();
      }
      return *this;
    }

    constexpr const T& operator*() const noexcept {
      return *instance_;
    }
    constexpr T& operator*() noexcept {
      return *instance_;
    }

    constexpr const T* operator->() const noexcept {
      return instance_.operator->();
    }
    constexpr T* operator->() noexcept {
      return instance_.operator->();
    }

    constexpr bool has_value() const noexcept {
      return instance_.has_value();
    }
  };

  // This partial specialization implements an optimization for when we know we don't need to store
  // an empty state to represent failure to perform an assignment. For copy-assignment, this
  // happens:
  //
  // 1. If the type is copyable (which includes copy-assignment), we can use the type's own
  // assignment operator
  //    directly and avoid using std::optional.
  // 2. If the type is not copyable, but it is nothrow-copy-constructible, then we can implement
  // assignment as
  //    destroy-and-then-construct and we know it will never fail, so we don't need an empty state.
  //
  // The exact same reasoning can be applied for move-assignment, with copyable replaced by movable
  // and nothrow-copy-constructible replaced by nothrow-move-constructible. This specialization is
  // enabled whenever we can apply any of these optimizations for both the copy assignment and the
  // move assignment operator.

  /// @cond undocumented
  template <class T>
  concept doesnt_need_empty_state_for_copy =
    std::copyable<T> or std::is_nothrow_copy_constructible_v<T>;

  template <class T>
  concept doesnt_need_empty_state_for_move =
    std::movable<T> or std::is_nothrow_move_constructible_v<T>;
  /// @endcond undocumented

  /// @spec copyable_box
  template <copy_constructible_object T>
  requires doesnt_need_empty_state_for_copy<T> and doesnt_need_empty_state_for_move<T>
  class copyable_box<T> {
    [[no_unique_address]] T instance_;

  public:
    template <class... Args>
    requires std::is_constructible_v<T, Args...>
    constexpr explicit copyable_box(std::in_place_t, Args&&... args) //
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : instance_(std::forward<Args>(args)...) {}

    constexpr copyable_box()                               //
      noexcept(std::is_nothrow_default_constructible_v<T>) //
      requires std::default_initializable<T> : instance_() {}

    constexpr copyable_box(const copyable_box&) = default;
    constexpr copyable_box(copyable_box&&) = default;

    // Implementation of assignment operators in case we perform optimization (1)
    constexpr copyable_box& operator=(const copyable_box&) requires std::copyable<T>
    = default;
    constexpr copyable_box& operator=(copyable_box&&) requires std::movable<T>
    = default;

    // Implementation of assignment operators in case we perform optimization (2)
    constexpr copyable_box& operator=(const copyable_box& other) noexcept {
      static_assert(std::is_nothrow_copy_constructible_v<T>);
      if (this != std::addressof(other)) {
        std::destroy_at(std::addressof(instance_));
        std::construct_at(std::addressof(instance_), other.instance_);
      }
      return *this;
    }

    constexpr copyable_box& operator=(copyable_box&& other) noexcept {
      static_assert(std::is_nothrow_move_constructible_v<T>);
      if (this != std::addressof(other)) {
        std::destroy_at(std::addressof(instance_));
        std::construct_at(std::addressof(instance_), std::move(other.instance_));
      }
      return *this;
    }

    constexpr const T& operator*() const noexcept {
      return instance_;
    }
    constexpr T& operator*() noexcept {
      return instance_;
    }

    constexpr const T* operator->() const noexcept {
      return std::addressof(instance_);
    }
    constexpr T* operator->() noexcept {
      return std::addressof(instance_);
    }

    constexpr bool has_value() const noexcept {
      return true;
    }
  };
} // namespace mpc
