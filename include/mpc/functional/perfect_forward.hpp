/// @file perfect_forward.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc {
  // perfect_forward
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__functional/perfect_forward.h

  template <class, class, class...>
  struct perfect_forward_impl;

  // perfect_forward implements a perfect-forwarding call wrapper.
  template <class Op, class... Args>
  using perfect_forward = perfect_forward_impl<Op, std::index_sequence_for<Args...>, Args...>;

  template <class Op>
  struct perfect_forwarded_t : perfect_forward<Op> {
    using perfect_forward<Op>::perfect_forward;
  };

  template <class Op, std::size_t... Idx, class... Bound>
  struct perfect_forward_impl<Op, std::index_sequence<Idx...>, Bound...> {
  private:
    std::tuple<Bound...> bound_;

  public:
    // clang-format off
    template <class... BoundArgs,
              class = std::enable_if_t<std::is_constructible_v<std::tuple<Bound...>, BoundArgs&&...>>>
    explicit constexpr perfect_forward_impl(BoundArgs&&... bound)
      : bound_(std::forward<BoundArgs>(bound)...) {}

    perfect_forward_impl(perfect_forward_impl const&) = default;
    perfect_forward_impl(perfect_forward_impl&&) = default;

    perfect_forward_impl& operator=(perfect_forward_impl const&) = default;
    perfect_forward_impl& operator=(perfect_forward_impl&&) = default;

    // operator()
    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, Bound&..., Args...>>>
    constexpr auto operator()(Args&&... args) & noexcept(
      noexcept(   Op()(std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(Op()(std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      Op()(std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op, Bound&..., Args...>>>
    constexpr auto operator()(Args&&... args) & noexcept(
      noexcept(   perfect_forward<Op, Bound..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(perfect_forward<Op, Bound..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      perfect_forward<Op, Bound..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, Bound const&..., Args...>>>
    constexpr auto operator()(Args&&... args) const& noexcept(
      noexcept(   Op()(std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(Op()(std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      Op()(std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args,
              class = std::enable_if_t<!std::is_invocable_v<Op, Bound const&..., Args...>>>
    constexpr auto operator()(Args&&... args) const& noexcept(
      noexcept(   perfect_forward<Op, Bound const..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(perfect_forward<Op, Bound const..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      perfect_forward<Op, Bound const..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, Bound..., Args...>>>
    constexpr auto operator()(Args&&... args) && noexcept(
      noexcept(   Op()(std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...)))
      -> decltype(Op()(std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...)) {
      return      Op()(std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op, Bound..., Args...>>>
    constexpr auto operator()(Args&&... args) && noexcept(
      noexcept(   perfect_forward<Op, Bound..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(perfect_forward<Op, Bound..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      perfect_forward<Op, Bound..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<Op, Bound const..., Args...>>>
    constexpr auto operator()(Args&&... args) const&& noexcept(
      noexcept(   Op()(std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...)))
      -> decltype(Op()(std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...)) {
      return      Op()(std::get<Idx>(std::move(bound_))..., std::forward<Args>(args)...);
    }

    template <class... Args, class = std::enable_if_t<!std::is_invocable_v<Op, Bound const..., Args...>>>
    constexpr auto operator()(Args&&... args) const&& noexcept(
      noexcept(   perfect_forward<Op, Bound const..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...)))
      -> decltype(perfect_forward<Op, Bound const..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...)) {
      return      perfect_forward<Op, Bound const..., Args...>(std::get<Idx>(bound_)..., std::forward<Args>(args)...);
    }

    // operator%
    template <class Arg, class = std::enable_if_t<std::is_invocable_v<Op, Bound&..., Arg>>>
    constexpr auto operator%(Arg&& arg) & noexcept(
      noexcept(   Op()(std::get<Idx>(bound_)..., std::forward<Arg>(arg))))
      -> decltype(Op()(std::get<Idx>(bound_)..., std::forward<Arg>(arg))) {
      return      Op()(std::get<Idx>(bound_)..., std::forward<Arg>(arg));
    }

    template <class Arg, class = std::enable_if_t<!std::is_invocable_v<Op, Bound&..., Arg>>>
    constexpr auto operator%(Arg&& arg) & noexcept(
      noexcept(   perfect_forward<Op, Bound..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg))))
      -> decltype(perfect_forward<Op, Bound..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg))) {
      return      perfect_forward<Op, Bound..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg));
    }

    template <class Arg, class = std::enable_if_t<std::is_invocable_v<Op, Bound const&..., Arg>>>
    constexpr auto operator%(Arg&& arg) const& noexcept(
      noexcept(   Op()(std::get<Idx>(bound_)..., std::forward<Arg>(arg))))
      -> decltype(Op()(std::get<Idx>(bound_)..., std::forward<Arg>(arg))) {
      return      Op()(std::get<Idx>(bound_)..., std::forward<Arg>(arg));
    }

    template <class Arg,
              class = std::enable_if_t<!std::is_invocable_v<Op, Bound const&..., Arg>>>
    constexpr auto operator%(Arg&& arg) const& noexcept(
      noexcept(   perfect_forward<Op, Bound const..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg))))
      -> decltype(perfect_forward<Op, Bound const..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg))) {
      return      perfect_forward<Op, Bound const..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg));
    }

    template <class Arg, class = std::enable_if_t<std::is_invocable_v<Op, Bound..., Arg>>>
    constexpr auto operator%(Arg&& arg) && noexcept(
      noexcept(   Op()(std::get<Idx>(std::move(bound_))..., std::forward<Arg>(arg))))
      -> decltype(Op()(std::get<Idx>(std::move(bound_))..., std::forward<Arg>(arg))) {
      return      Op()(std::get<Idx>(std::move(bound_))..., std::forward<Arg>(arg));
    }

    template <class Arg, class = std::enable_if_t<!std::is_invocable_v<Op, Bound..., Arg>>>
    constexpr auto operator%(Arg&& arg) && noexcept(
      noexcept(   perfect_forward<Op, Bound..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg))))
      -> decltype(perfect_forward<Op, Bound..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg))) {
      return      perfect_forward<Op, Bound..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg));
    }

    template <class Arg, class = std::enable_if_t<std::is_invocable_v<Op, Bound const..., Arg>>>
    constexpr auto operator%(Arg&& arg) const&& noexcept(
      noexcept(   Op()(std::get<Idx>(std::move(bound_))..., std::forward<Arg>(arg))))
      -> decltype(Op()(std::get<Idx>(std::move(bound_))..., std::forward<Arg>(arg))) {
      return      Op()(std::get<Idx>(std::move(bound_))..., std::forward<Arg>(arg));
    }

    template <class Arg, class = std::enable_if_t<!std::is_invocable_v<Op, Bound const..., Arg>>>
    constexpr auto operator%(Arg&& arg) const&& noexcept(
      noexcept(   perfect_forward<Op, Bound const..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg))))
      -> decltype(perfect_forward<Op, Bound const..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg))) {
      return      perfect_forward<Op, Bound const..., Arg>(std::get<Idx>(bound_)..., std::forward<Arg>(arg));
    }
    // clang-format on
  };
} // namespace mpc
