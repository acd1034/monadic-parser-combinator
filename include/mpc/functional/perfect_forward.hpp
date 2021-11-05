/// @file perfect_forward.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/stdfundamental.hpp>
#include <mpc/utility/copyable_box.hpp>

// clang-format off

namespace mpc {
  // perfect_forward
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__functional/perfect_forward.h

  template <class, class, class...>
  struct perfect_forward_impl;

  // perfect_forward implements a perfect-forwarding call wrapper.
  template <class Op, class... Args>
  using perfect_forward = perfect_forward_impl<Op, std::index_sequence_for<Args...>, Args...>;

  template <class Op, class... Args>
  struct perfect_forwarded_t : perfect_forward<Op, Args...> {
    using perfect_forward<Op, Args...>::perfect_forward;
  };

  template <copy_constructible_object Op, std::size_t... Idx, class... Bound>
  struct perfect_forward_impl<Op, std::index_sequence<Idx...>, Bound...> {
  private:
    copyable_box<Op> op_{};
    std::tuple<Bound...> bound_{};

  public:
    explicit constexpr perfect_forward_impl()
    requires std::default_initializable<Op> and (sizeof...(Bound) == 0) = default;

    template <class... BoundArgs,
              class = std::enable_if_t<std::is_constructible_v<std::tuple<Bound...>, BoundArgs&&...>>>
    explicit constexpr perfect_forward_impl(Op const& op, BoundArgs&&... bound)
      : op_(std::in_place, op), bound_(std::forward<BoundArgs>(bound)...) {}

    template <class... BoundArgs,
              class = std::enable_if_t<std::is_constructible_v<std::tuple<Bound...>, BoundArgs&&...>>>
    explicit constexpr perfect_forward_impl(Op&& op, BoundArgs&&... bound)
      : op_(std::in_place, std::move(op)), bound_(std::forward<BoundArgs>(bound)...) {}

    template <class... BoundArgs,
              class = std::enable_if_t<std::is_constructible_v<std::tuple<Bound...>, BoundArgs&&...>>>
    explicit constexpr perfect_forward_impl(copyable_box<Op> const& op, BoundArgs&&... bound)
      : op_(op), bound_(std::forward<BoundArgs>(bound)...) {}

    template <class... BoundArgs,
              class = std::enable_if_t<std::is_constructible_v<std::tuple<Bound...>, BoundArgs&&...>>>
    explicit constexpr perfect_forward_impl(copyable_box<Op>&& op, BoundArgs&&... bound)
      : op_(std::move(op)), bound_(std::forward<BoundArgs>(bound)...) {}

    perfect_forward_impl(perfect_forward_impl const&) = default;
    perfect_forward_impl(perfect_forward_impl&&) = default;

    perfect_forward_impl& operator=(perfect_forward_impl const&) = default;
    perfect_forward_impl& operator=(perfect_forward_impl&&) = default;

    // operator()
    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, Bound&..., Args...>>>
    constexpr auto operator()(Args&&... args) & noexcept(
      noexcept(   std::invoke(*op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(std::invoke(*op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      std::invoke(*op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op, Bound&..., Args...>>>
    constexpr auto operator()(Args&&... args) & noexcept(
      noexcept(   perfect_forward<Op, Bound..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(perfect_forward<Op, Bound..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      perfect_forward<Op, Bound..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op const, Bound const&..., Args...>>>
    constexpr auto operator()(Args&&... args) const& noexcept(
      noexcept(   std::invoke(*op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(std::invoke(*op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      std::invoke(*op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args,
              class = std::enable_if_t<!std::is_invocable_v<Op const, Bound const&..., Args...>>>
    constexpr auto operator()(Args&&... args) const& noexcept(
      noexcept(   perfect_forward<Op, Bound const..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(perfect_forward<Op, Bound const..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      perfect_forward<Op, Bound const..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, Bound..., Args...>>>
    constexpr auto operator()(Args&&... args) && noexcept(
      noexcept(   std::invoke(*op_, std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...)))
      -> decltype(std::invoke(*op_, std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...)) {
      return      std::invoke(*op_, std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op, Bound..., Args...>>>
    constexpr auto operator()(Args&&... args) && noexcept(
      noexcept(   perfect_forward<Op, Bound..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(perfect_forward<Op, Bound..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      perfect_forward<Op, Bound..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op const, Bound const..., Args...>>>
    constexpr auto operator()(Args&&... args) const&& noexcept(
      noexcept(   std::invoke(*op_, std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...)))
      -> decltype(std::invoke(*op_, std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...)) {
      return      std::invoke(*op_, std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op const, Bound const..., Args...>>>
    constexpr auto operator()(Args&&... args) const&& noexcept(
      noexcept(   perfect_forward<Op, Bound const..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(perfect_forward<Op, Bound const..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      perfect_forward<Op, Bound const..., Args...>(op_, std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    // operator%
    template <class Arg>
    constexpr auto operator%(Arg&& arg) & noexcept(
      noexcept(   this->operator()(std::forward<Arg>(arg))))
      -> decltype(this->operator()(std::forward<Arg>(arg))) {
      return      this->operator()(std::forward<Arg>(arg));
    }

    template <class Arg>
    constexpr auto operator%(Arg&& arg) const& noexcept(
      noexcept(   this->operator()(std::forward<Arg>(arg))))
      -> decltype(this->operator()(std::forward<Arg>(arg))) {
      return      this->operator()(std::forward<Arg>(arg));
    }

    template <class Arg>
    constexpr auto operator%(Arg&& arg) && noexcept(
      noexcept(   this->operator()(std::forward<Arg>(arg))))
      -> decltype(this->operator()(std::forward<Arg>(arg))) {
      return      this->operator()(std::forward<Arg>(arg));
    }

    template <class Arg>
    constexpr auto operator%(Arg&& arg) const&& noexcept(
      noexcept(   this->operator()(std::forward<Arg>(arg))))
      -> decltype(this->operator()(std::forward<Arg>(arg))) {
      return      this->operator()(std::forward<Arg>(arg));
    }
  };
} // namespace mpc

// clang-format on
