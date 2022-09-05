/// @file function.hpp
#pragma once
#include <functional> // std::invoke
#include <memory>     // std::shared_ptr
#include <mpc/stdfundamental.hpp>

namespace mpc {
  template <class Ret, class Arg>
  struct _function {
    constexpr virtual ~_function() = default;
    constexpr virtual Ret operator()(const Arg&) const = 0;
    constexpr virtual Ret operator()(Arg&&) const = 0;
  };

  template <class F, class Ret, class Arg>
  struct _function_impl : _function<Ret, Arg> {
  private:
    F f_;

  public:
    constexpr _function_impl(const F& f) : f_(f) {}
    constexpr _function_impl(F&& f) : f_(std::move(f)) {}
    constexpr Ret operator()(const Arg& arg) const override {
      return std::invoke(f_, arg);
    }
    constexpr Ret operator()(Arg&& arg) const override {
      return std::invoke(f_, std::move(arg));
    }
  };

  template <class>
  struct function;

  template <class Ret, class Arg>
  requires (not std::is_reference_v<Arg>)
  struct function<Ret(Arg)> {
  private:
    std::shared_ptr<_function<Ret, Arg>> instance_ = nullptr;

  public:
    template <class F>
    constexpr function(F&& f)
      : instance_(std::make_shared<_function_impl<std::decay_t<F>, Ret, Arg>>(
        std::forward<F>(f))) {}
    constexpr Ret operator()(const Arg& arg) const {
      return instance_->operator()(arg);
    }
    constexpr Ret operator()(Arg&& arg) const {
      return instance_->operator()(std::move(arg));
    }
    constexpr Ret operator%(const Arg& arg) const {
      return instance_->operator()(arg);
    }
    constexpr Ret operator%(Arg&& arg) const {
      return instance_->operator()(std::move(arg));
    }
  };
} // namespace mpc
