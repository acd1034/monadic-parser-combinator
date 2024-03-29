/// @file partial.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/utility/copyable_box.hpp>
#include <mpc/utility/forward_like.hpp>

// clang-format off

namespace mpc {
  // partial
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__functional/perfect_forward.h

  /// Implements a perfect-forwarding call wrapper.
  template <copy_constructible_object Op, class... Bound>
  struct partial;

  template <class T>
  inline constexpr bool is_partial_v = false;

  template <copy_constructible_object Op, class... Bound>
  inline constexpr bool is_partial_v<partial<Op, Bound...>> = true;

  template <class Op, class Tuple, std::size_t... Idx, class... Args>
  constexpr auto make_partial_impl(Op&& op, Tuple&& tuple, std::index_sequence<Idx...>, Args&&... args) noexcept(
    noexcept(   partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)))
    -> decltype(partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)) {
    return      partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...);
  }

  template <class Op, class... Args,
            class = std::enable_if_t<is_partial_v<std::remove_cvref_t<Op>>>>
  constexpr auto make_partial(Op&& op, Args&&... args) noexcept(
    noexcept(   make_partial_impl(forward_like<Op>(op.op_), forward_like<Op>(op.bound_), std::make_index_sequence<std::tuple_size_v<decltype(op.bound_)>>(), std::forward<Args>(args)...)))
    -> decltype(make_partial_impl(forward_like<Op>(op.op_), forward_like<Op>(op.bound_), std::make_index_sequence<std::tuple_size_v<decltype(op.bound_)>>(), std::forward<Args>(args)...)) {
    return      make_partial_impl(forward_like<Op>(op.op_), forward_like<Op>(op.bound_), std::make_index_sequence<std::tuple_size_v<decltype(op.bound_)>>(), std::forward<Args>(args)...);
  }

  template <class Op, class... Args,
            class = std::enable_if_t<!is_partial_v<std::remove_cvref_t<Op>>>>
  constexpr auto make_partial(Op&& op, Args&&... args) noexcept(
    noexcept(   partial(std::forward<Op>(op), std::forward<Args>(args)...)))
    -> decltype(partial(std::forward<Op>(op), std::forward<Args>(args)...)) {
    return      partial(std::forward<Op>(op), std::forward<Args>(args)...);
  }

  template <class Op, class Tuple, std::size_t... Idx, class... Args,
            class = std::enable_if_t<std::is_invocable_v<Op, forward_like_t<Tuple, std::tuple_element_t<Idx, std::remove_cvref_t<Tuple>>>..., Args...>>>
  constexpr auto __call(Op&& op, Tuple&& tuple, std::index_sequence<Idx...>, Args&&... args) noexcept(
    noexcept(   std::invoke(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)))
    -> decltype(std::invoke(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)) {
    return      std::invoke(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...);
  }

  template <class Op, class Tuple, std::size_t... Idx, class... Args,
            class = std::enable_if_t<!std::is_invocable_v<Op, forward_like_t<Tuple, std::tuple_element_t<Idx, std::remove_cvref_t<Tuple>>>..., Args...>>>
  constexpr auto __call(Op&& op, Tuple&& tuple, std::index_sequence<Idx...>, Args&&... args) noexcept(
    noexcept(   partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)))
    -> decltype(partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)) {
    return      partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...);
  }

  template <copy_constructible_object Op, class... Bound>
  struct partial {
  private:
    copyable_box<Op> op_{};
    std::tuple<Bound...> bound_{};

    template <class Op2, class... Args, class>
    friend constexpr auto make_partial(Op2&&, Args&&...);

  public:
    constexpr explicit partial()
    requires std::default_initializable<Op> and (std::default_initializable<Bound> and ...) = default;

    template <class... BoundArgs,
              class = std::enable_if_t<std::is_constructible_v<std::tuple<Bound...>, BoundArgs&&...>>>
    constexpr explicit partial(Op const& op, BoundArgs&&... bound)
      : op_(std::in_place, op), bound_(std::forward<BoundArgs>(bound)...) {}

    template <class... BoundArgs,
              class = std::enable_if_t<std::is_constructible_v<std::tuple<Bound...>, BoundArgs&&...>>>
    constexpr explicit partial(Op&& op, BoundArgs&&... bound)
      : op_(std::in_place, std::move(op)), bound_(std::forward<BoundArgs>(bound)...) {}

    partial(partial const&) = default;
    partial(partial&&) = default;

    partial& operator=(partial const&) = default;
    partial& operator=(partial&&) = default;

    // operator()
    template <class... Args>
    constexpr auto operator()(Args&&... args) & noexcept(
      noexcept(   __call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)))
      -> decltype(__call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)) {
      return      __call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) const& noexcept(
      noexcept(   __call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)))
      -> decltype(__call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)) {
      return      __call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) && noexcept(
      noexcept(   __call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)))
      -> decltype(__call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)) {
      return      __call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) const&& noexcept(
      noexcept(   __call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)))
      -> decltype(__call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)) {
      return      __call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...);
    }

    // operator%
    template <class Arg>
    constexpr auto operator%(Arg&& arg) & noexcept(
      noexcept(   (*this)(std::forward<Arg>(arg))))
      -> decltype((*this)(std::forward<Arg>(arg))) {
      return      (*this)(std::forward<Arg>(arg));
    }

    template <class Arg>
    constexpr auto operator%(Arg&& arg) const& noexcept(
      noexcept(   (*this)(std::forward<Arg>(arg))))
      -> decltype((*this)(std::forward<Arg>(arg))) {
      return      (*this)(std::forward<Arg>(arg));
    }

    template <class Arg>
    constexpr auto operator%(Arg&& arg) && noexcept(
      noexcept(   std::move(*this)(std::forward<Arg>(arg))))
      -> decltype(std::move(*this)(std::forward<Arg>(arg))) {
      return      std::move(*this)(std::forward<Arg>(arg));
    }

    template <class Arg>
    constexpr auto operator%(Arg&& arg) const&& noexcept(
      noexcept(   std::move(*this)(std::forward<Arg>(arg))))
      -> decltype(std::move(*this)(std::forward<Arg>(arg))) {
      return      std::move(*this)(std::forward<Arg>(arg));
    }
  };

  /// @dguide partial
  template <class Op, class... Args>
  partial(Op, Args...) -> partial<Op, Args...>;
} // namespace mpc

// clang-format on
