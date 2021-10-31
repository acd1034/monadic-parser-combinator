/// @file bind_back.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {
  // bind_back
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__functional/bind_back.h

  // clang-format off
  namespace detail {
    template <size_t NBound, class = std::make_index_sequence<NBound>>
    struct bind_backed_fn_op;

    template <size_t NBound, size_t... Idx>
    struct bind_backed_fn_op<NBound, std::index_sequence<Idx...>> {
      template <class Fn, class Bound, class... Args>
      constexpr auto operator()(Fn&& f, Bound&& bound, Args&& ...args) const
        noexcept(noexcept(std::invoke(std::forward<Fn>(f), std::forward<Args>(args)..., std::get<Idx>(std::forward<Bound>(bound))...)))
        -> decltype(      std::invoke(std::forward<Fn>(f), std::forward<Args>(args)..., std::get<Idx>(std::forward<Bound>(bound))...))
        { return          std::invoke(std::forward<Fn>(f), std::forward<Args>(args)..., std::get<Idx>(std::forward<Bound>(bound))...); }
    };

    template <class Fn, class BoundArgs>
    struct bind_backed_fn_t : perfect_forward<bind_backed_fn_op<std::tuple_size_v<BoundArgs>>, Fn, BoundArgs> {
      using perfect_forward<bind_backed_fn_op<std::tuple_size_v<BoundArgs>>, Fn, BoundArgs>::perfect_forward;
    };
  } // namespace detail

  /// @brief bind_back
  /// @note `bind_back` does NOT support partial application because it cannot deduce # of arguments to bind.
  template <class Fn, class ...Args, class = std::enable_if_t<
    _and_v<
      std::is_constructible<std::decay_t<Fn>, Fn>,
      std::is_move_constructible<std::decay_t<Fn>>,
      std::is_constructible<std::decay_t<Args>, Args>...,
      std::is_move_constructible<std::decay_t<Args>>...
    >
  >>
  constexpr auto bind_back(Fn&& f, Args&&... args)
    noexcept(noexcept(detail::bind_backed_fn_t<std::decay_t<Fn>, std::tuple<std::decay_t<Args>...>>(std::forward<Fn>(f), std::forward_as_tuple(std::forward<Args>(args)...))))
    -> decltype(      detail::bind_backed_fn_t<std::decay_t<Fn>, std::tuple<std::decay_t<Args>...>>(std::forward<Fn>(f), std::forward_as_tuple(std::forward<Args>(args)...)))
    { return          detail::bind_backed_fn_t<std::decay_t<Fn>, std::tuple<std::decay_t<Args>...>>(std::forward<Fn>(f), std::forward_as_tuple(std::forward<Args>(args)...)); }
  // clang-format on
} // namespace mpc
