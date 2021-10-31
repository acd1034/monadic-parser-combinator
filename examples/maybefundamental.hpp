/// @file maybefundamental.hpp
#pragma once
#include <functional>
#include <variant>
#include <mpc/control/monad.hpp>
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/utility/single.hpp>

struct nothing_t {
  inline constexpr auto operator<=>(const nothing_t&) const = default;
};

inline constexpr nothing_t nothing;

template <class T>
using just_t = mpc::single<T, nothing_t>;

template <class T>
constexpr just_t<std::unwrap_ref_decay_t<T>> make_just(T&& t) {
  return std::forward<T>(t);
}

template <class T>
using maybe = std::variant<nothing_t, just_t<T>>;

namespace detail {
  template <class>
  struct is_maybe : std::false_type {};

  template <class T>
  struct is_maybe<maybe<T>> : std::true_type {};

  template <class T>
  concept Maybe = is_maybe<std::remove_cvref_t<T>>::value;

  struct fmap_op {
    template <class F, Maybe M>
    constexpr auto operator()(F&& f, M&& x) const //
      -> maybe<std::unwrap_ref_decay_t<decltype(std::invoke(std::forward<F>(f),
                                                            *std::get<1>(std::forward<M>(x))))>> {
      if (x.index() == 0) {
        return nothing;
      } else {
        return make_just(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x))));
      }
    }
  };

  struct replace2nd_op {
    template <class U, Maybe M>
    constexpr auto operator()(U&& u, M&& m) const //
      -> maybe<std::unwrap_ref_decay_t<U&&>> {
      if (m.index() == 0) {
        return nothing;
      } else {
        return make_just(std::forward<U>(u));
      }
    }
  };

  struct bind_op {
    template <Maybe M, class F>
    constexpr auto operator()(M&& x, F&& f) const //
      -> decltype(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x)))) {
      if (x.index() == 0) {
        return nothing;
      } else {
        return std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x)));
      }
    }
  };

  struct pure_op {
    template <class U>
    constexpr auto operator()(U&& u) const   //
      -> maybe<std::unwrap_ref_decay_t<U>> { //
      return make_just(std::forward<U>(u));
    }
  };

  struct seq_apply_op {
    template <Maybe M1, Maybe M2>
    constexpr auto operator()(M1&& f, M2&& x) const //
      -> decltype(mpc::fmap<M1>(*std::get<1>(std::forward<M1>(f)), std::forward<M2>(x))) {
      if (f.index() == 0) {
        return nothing;
      } else {
        return mpc::fmap<M1>(*std::get<1>(std::forward<M1>(f)), std::forward<M2>(x));
      }
    }
  };

  struct liftA2_op {
    template <class F, Maybe M1, Maybe M2>
    constexpr auto operator()(F&& f, M1&& m1, M2&& m2) const                                    //
      -> maybe<std::unwrap_ref_decay_t<decltype(std::invoke(std::forward<F>(f),                 //
                                                            *std::get<1>(std::forward<M1>(m1)), //
                                                            *std::get<1>(std::forward<M2>(m2))))>> {
      if (m1.index() == 1 and m2.index() == 1) {
        return make_just(std::invoke(std::forward<F>(f),                 //
                                     *std::get<1>(std::forward<M1>(m1)), //
                                     *std::get<1>(std::forward<M2>(m2))));
      } else {
        return nothing;
      }
    }
  };
} // namespace detail
