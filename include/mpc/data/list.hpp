/// @file list.hpp
#pragma once
#include <algorithm>  // std::ranges::transform
#include <functional> // std::invoke
#include <iterator>
#include <list>
#include <ranges> // std::ranges::input_range, etc.
#include <mpc/control/monad.hpp>
#include <mpc/prelude.hpp> // identity

namespace mpc {
  // cons, foldr
  namespace detail {
    struct cons_op {
      template <class T, class U>
      constexpr auto operator()(T&& t, const std::list<U>& l) const {
        auto ret = l;
        ret.emplace_front(std::forward<T>(t));
        return ret;
      }

      template <class T, class U>
      constexpr auto operator()(T&& t, std::list<U>&& l) const {
        auto ret = std::move(l);
        ret.emplace_front(std::forward<T>(t));
        return ret;
      }
    };

    struct foldr_op {
      template <class Fn, std::movable T, std::input_iterator I, std::sentinel_for<I> S>
      constexpr auto operator()(Fn op, T&& init, I first, S last) const -> T {
        if (first == last) {
          return std::forward<T>(init);
        } else {
          return std::invoke(
            op, *first,
            this->operator()(op, std::forward<T>(init), std::ranges::next(first), last));
        }
      }

      template <class Fn, std::movable T, std::ranges::input_range R>
      constexpr auto operator()(Fn&& op, T&& init, R&& r) const -> T {
        return this->operator()(std::forward<Fn>(op), std::forward<T>(init), std::ranges::begin(r),
                                std::ranges::end(r));
      }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr perfect_forwarded_t<detail::cons_op> cons;

    inline constexpr perfect_forwarded_t<detail::foldr_op> foldr;
  } // namespace cpo

  // instances
  template <class T>
  struct functor_traits<std::list<T>> {
    struct fmap_op {
      template <class Fn, class U>
      constexpr auto operator()(Fn&& f, const std::list<U>& l) const {
        std::list<std::invoke_result_t<Fn, U>> ret(std::ranges::size(l));
        std::ranges::transform(l, std::begin(ret), std::forward<Fn>(f));
        return ret;
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd;
  };

  template <class T>
  struct applicative_traits<std::list<T>> {
    struct pure_op {
      template <class U>
      constexpr auto operator()(U&& u) const {
        return std::list{std::forward<U>(u)};
      }
    };

    struct liftA2_op {
      template <class Fn, class U1, class U2>
      constexpr auto operator()(Fn&& f, const std::list<U1>& l1, const std::list<U2>& l2) const {
        const auto n = std::min(std::ranges::size(l1), std::ranges::size(l2));
        std::list<std::invoke_result_t<Fn, U1, U2>> ret(n);
        std::ranges::transform(l1, l2, std::begin(ret), std::forward<Fn>(f));
        return ret;
      }
    };

    static constexpr pure_op pure{};
    static constexpr liftA2_op liftA2{};
    static constexpr auto seq_apply = applicatives::seq_apply;
    static constexpr auto discard2nd = applicatives::discard2nd;
    static constexpr auto discard1st = applicatives::discard1st;
  };

  // sequence
  namespace detail {
    struct sequence_op {
      // FIXME: 本来 applicative T
      template <monad T>
      constexpr auto operator()(const std::list<T>& l) const {
        // FIXME: 不正な方法で monad の value_type を取得している
        using U = std::remove_cvref_t<decltype(mpc::bind(l.front(), identity))>;
        return foldr(mpc::liftA2 % cons, mpc::returns<T> % std::list<U>{}, l);
      }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr perfect_forwarded_t<detail::sequence_op> sequence;
  } // namespace cpo
} // namespace mpc
