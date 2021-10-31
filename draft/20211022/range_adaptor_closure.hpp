/// @file range_adaptor_closure.hpp
#pragma once
#include <ranges>
#include <mpc/functional/compose.hpp>

namespace mpc {
  // clang-format off

  // CRTP base that one can derive from in order to be considered a range adaptor closure
  // by the library. When deriving from this class, a pipe operator will be provided to
  // make the following hold:
  // - `x | f` is equivalent to `f(x)`
  // - `f1 | f2` is an adaptor closure `g` such that `g(x)` is equivalent to `f2(f1(x))`
  template <class T>
  struct range_adaptor_closure;

  // Type that wraps an arbitrary function object and makes it into a range adaptor closure,
  // i.e. something that can be called via the `x | f` notation.
  template <class Fn>
  struct range_adaptor_closure_t : Fn, range_adaptor_closure<range_adaptor_closure_t<Fn>> {
    constexpr explicit range_adaptor_closure_t(Fn&& f) : Fn(std::move(f)) {}
  };

  template <class T>
  concept RangeAdaptorClosure = std::derived_from<std::remove_cvref_t<T>, range_adaptor_closure<std::remove_cvref_t<T>>>;

  template <class T>
  struct range_adaptor_closure {
    template <std::ranges::viewable_range View, RangeAdaptorClosure Closure>
      requires
        std::same_as<T, std::remove_cvref_t<Closure>> &&
        std::invocable<Closure, View>
    [[nodiscard]]
    friend constexpr decltype(auto) operator|(View&& view, Closure&& closure)
      noexcept(std::is_nothrow_invocable_v<Closure, View>)
    { return std::invoke(std::forward<Closure>(closure), std::forward<View>(view)); }

    template <RangeAdaptorClosure Closure, RangeAdaptorClosure OtherClosure>
      requires
        std::same_as<T, std::remove_cvref_t<Closure>> &&
        std::constructible_from<std::decay_t<Closure>, Closure> &&
        std::constructible_from<std::decay_t<OtherClosure>, OtherClosure>
    [[nodiscard]]
    friend constexpr auto operator|(Closure&& c1, OtherClosure&& c2)
      noexcept(
        std::is_nothrow_constructible_v<std::decay_t<Closure>, Closure> &&
        std::is_nothrow_constructible_v<std::decay_t<OtherClosure>, OtherClosure>)
    { return range_adaptor_closure_t(mpc::compose(std::forward<OtherClosure>(c2), std::forward<Closure>(c1))); }
  };
  // clang-format on
} // namespace mpc
