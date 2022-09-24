// Reference: https://github.com/acd1034/monadic-parser-combinator/commit/4a7ee617f1245ec5b5c0d653ec6dec9688d8ba4c
/// @file function.hpp
#pragma once
#include <functional>
// #include <memory>
#include <type_traits>
#include <utility>

template <typename T>
class unique_ptr {
public:
  // コンストラクタ
  constexpr unique_ptr() {}
  constexpr unique_ptr(T* pointer) : pointer_(pointer) {}

  // デストラクタ
  constexpr ~unique_ptr() {
    // デストラクタで管理対象の変数を delete
    if (pointer_) {
      delete pointer_;
    }
  }

  // コピーコンストラクタ (コピー禁止)
  unique_ptr(const unique_ptr&) = delete;

  // コピー代入演算子 (コピー禁止)
  unique_ptr& operator=(const unique_ptr&) = delete;

  // ムーブコンストラクタ
  constexpr unique_ptr(unique_ptr&& v) : pointer_(v.pointer_) { // 移譲元の参照を引き継ぐ
    v.pointer_ = nullptr; // 移譲元の参照ポインタをクリア
  }

  // ムーブ代入演算子
  constexpr unique_ptr& operator=(unique_ptr&& v) {
    if (&v == this) { // a = a; のような自己代入の対応 (何もしない)
      return *this;
    }

    auto* temp = pointer_; // 確実にコピー終了後に古いデータを削除するために一旦tempにコピー
    pointer_ = v.pointer_; // 移譲元の参照を引き継ぐ
    v.pointer_ = nullptr;  // 移譲元の参照ポインタをクリア
    delete temp;           // 代入前の古いデータは削除する

    return *this;
  }

  // 実体変数の値参照
  constexpr T& operator*() const {
    return *pointer_;
  }

  // 実体変数のポインタ参照
  constexpr T* operator->() const {
    return pointer_;
  }

  // getメソッド: 実体変数のポインタ取得
  constexpr T* get() const {
    return pointer_;
  }

  // releaseメソッド: 実体変数のポインタ取得 & 管理対象から外す
  constexpr T* release() {
    auto* temp = pointer_;
    pointer_ = nullptr;
    return temp;
  }

private:
  T* pointer_ = nullptr; // 実体変数へのポインタ
};

template <class R, class... Args>
struct function_impl_base {
  constexpr virtual ~function_impl_base() = default;
  constexpr virtual R operator()(Args...) const = 0;
};

template <class F, class R, class... Args>
struct function_impl : function_impl_base<R, Args...> {
  F f_;

  constexpr function_impl(const F& f) : f_(f) {}
  constexpr function_impl(F&& f) : f_(std::move(f)) {}
  constexpr function_impl(const function_impl&) = default;
  constexpr function_impl(function_impl&&) = default;
  constexpr ~function_impl() override = default;
  constexpr function_impl& operator=(const function_impl&) = default;
  constexpr function_impl& operator=(function_impl&&) = default;
  constexpr R operator()(Args... args) const override {
    return f_(std::forward<Args>(args)...);
  }
};

template <class>
struct function;

template <class R, class... Args>
struct function<R(Args...)> {
private:
  using F = function_impl_base<R, Args...>;

  template <class H>
  static constexpr F* copy_impl(F* f_ptr) {
    return new H{*reinterpret_cast<std::add_pointer_t<H>>(f_ptr)};
  }

  constexpr F* copied_instance() const {
    return copy_(instance_.get());
  }

public:
  unique_ptr<F> instance_{};
  std::add_pointer_t<F*(F*)> copy_{};

  template <class G>
  constexpr function(G&& g)
    : instance_{new function_impl<std::decay_t<G>, R, Args...>{std::forward<G>(g)}},
      copy_{&copy_impl<function_impl<std::decay_t<G>, R, Args...>>} {
  }

  constexpr function(const function& other) : instance_{other.copied_instance()}, copy_{other.copy_} {}

  constexpr function(function&&) = default;

  constexpr ~function() = default;

  constexpr function& operator=(const function& other) {
    instance_ = unique_ptr<F>{other.copied_instance()};
    copy_ = other.copy_;
    return *this;
  }

  constexpr function& operator=(function&&) = default;

  constexpr R operator()(Args... args) const {
    return instance_->operator()(std::forward<Args>(args)...);
  }
};
