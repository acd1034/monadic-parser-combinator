/// @file function.hpp
#pragma once
#include <memory>
#include <type_traits>
#include <utility>

namespace mpc {
  template <class Ret, class... Args>
  struct _function_impl {
    constexpr virtual ~_function_impl() = default;
    constexpr virtual Ret operator()(Args...) const = 0;
    // constexpr virtual Ret operator()(Args...) = 0;
  };

  template <class F, class Ret, class... Args>
  struct _function : _function_impl<Ret, Args...> {
    F f_;
    constexpr _function(const F& f) : f_(f) {}
    constexpr _function(F&& f) : f_(std::move(f)) {}
    constexpr Ret operator()(Args... args) const override {
      return f_(std::forward<Args>(args)...);
    }
    /* constexpr Ret operator()(Args... args) override {
      return f_(std::forward<Args>(args)...);
    } */
  };

  template <class>
  struct function;

  template <class Ret, class... Args>
  struct function<Ret(Args...)> {
    std::shared_ptr<_function_impl<Ret, Args...>> instance_ = nullptr;

    template <class F>
    constexpr function(F&& f)
      : instance_(std::make_shared<_function<std::decay_t<F>, Ret, Args...>>(std::forward<F>(f))) {}

    constexpr Ret operator()(Args... args) const {
      return instance_->operator()(std::forward<Args>(args)...);
    }
    /* constexpr Ret operator()(Args... args) {
      return instance_->operator()(std::forward<Args>(args)...);
    } */
  };
} // namespace mpc
