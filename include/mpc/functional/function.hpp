/// @file function.hpp
#pragma once
#include <functional> // std::invoke
#include <memory>     // std::shared_ptr
#include <mpc/stdfundamental.hpp>

namespace mpc {
  template <class Ret, class... Args>
  struct _function {
    constexpr virtual ~_function() = default;
    constexpr virtual Ret operator()(Args...) const = 0;
  };

  template <class F, class Ret, class... Args>
  struct _function_impl : _function<Ret, Args...> {
  private:
    F f_;

  public:
    constexpr _function_impl(const F& f) : f_(f) {}
    constexpr _function_impl(F&& f) : f_(std::move(f)) {}
    constexpr Ret operator()(Args... args) const override {
      return std::invoke(f_, std::forward<Args>(args)...);
    }
  };

  template <class>
  struct function;

  template <class Ret, class... Args>
  struct function<Ret(Args...)> {
  private:
    std::shared_ptr<_function<Ret, Args...>> instance_ = nullptr;

  public:
    template <class F>
    constexpr function(F&& f)
      : instance_(std::make_shared<_function_impl<std::decay_t<F>, Ret, Args...>>(
        std::forward<F>(f))) {}

    constexpr Ret operator()(Args... args) const {
      return instance_->operator()(std::forward<Args>(args)...);
    }
  };
} // namespace mpc
