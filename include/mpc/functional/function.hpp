/// @file function.hpp
#pragma once
#include <functional> // std::invoke
#include <memory>     // std::shared_ptr
#include <mpc/stdfundamental.hpp>

namespace mpc {
  // https://stackoverflow.com/questions/53977787/constexpr-version-of-stdfunction
  template <class Ret, class Arg>
  struct _function {
    constexpr virtual ~_function() = default;
    constexpr virtual Ret operator()(Arg&&) const = 0;
  };

  template <class F, class Ret, class Arg>
  struct _function_impl : _function<Ret, Arg> {
  private:
    F f_;

  public:
    constexpr _function_impl(const F& f) : f_(f) {}
    constexpr _function_impl(F&& f) : f_(std::move(f)) {}
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
    function() = default;
    template <class F>
    requires std::invocable<std::decay_t<F>&, Arg> and std::same_as<std::invoke_result_t<std::decay_t<F>&, Arg>, Ret>
    constexpr function(F&& f)
      : instance_(std::make_shared<_function_impl<std::decay_t<F>, Ret, Arg>>(
        std::forward<F>(f))) {}
    constexpr Ret operator()(const Arg& arg) const {
      auto tmp = arg;
      return instance_->operator()(std::move(tmp));
    }
    constexpr Ret operator()(Arg&& arg) const {
      return instance_->operator()(std::move(arg));
    }
    constexpr Ret operator%(const Arg& arg) const {
      auto tmp = arg;
      return instance_->operator()(std::move(tmp));
    }
    constexpr Ret operator%(Arg&& arg) const {
      return instance_->operator()(std::move(arg));
    }
  };
} // namespace mpc
