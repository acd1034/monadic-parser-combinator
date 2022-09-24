/// @file list.hpp
#pragma once
#include <algorithm>  // std::transform
#include <functional> // std::invoke
#include <iterator>
#include <list>
#include <mpc/control/holding.hpp>
#include <mpc/control/monad.hpp>
#include <mpc/functional/partial.hpp>
#include <mpc/prelude.hpp> // identity
#include <mpc/ranges.hpp>

namespace mpc {
  // List operations
  // https://hackage.haskell.org/package/base-4.17.0.0/docs/Prelude.html#g:13

  namespace detail {
    template <class>
    struct is_list_impl : std::false_type {};

    template <class T, class Alloc>
    struct is_list_impl<std::list<T, Alloc>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept is_list = detail::is_list_impl<std::remove_cvref_t<T>>::value;

  // cons, foldr
  namespace detail {
    struct cons_op {
      template <class T, is_list L>
      constexpr auto operator()(T&& t, L&& l) const {
        auto ret = std::forward<L>(l);
        ret.emplace_front(std::forward<T>(t));
        return ret;
      }

      template <class T, class L>
      requires std::same_as<std::remove_cvref_t<L>, std::string>
      constexpr auto operator()(T&& t, L&& l) const {
        return std::forward<T>(t) + std::forward<L>(l);
      }
    };

    struct foldr_op {
      template <class Fn, std::movable T, std::input_iterator I, std::sentinel_for<I> S>
      constexpr auto operator()(Fn op, T&& init, I first, S last) const -> T {
        if (first == last) {
          return std::forward<T>(init);
        } else {
          auto tmp = first++;
          return std::invoke(op, *tmp, this->operator()(op, std::forward<T>(init), first, last));
        }
      }

      template <class Fn, std::movable T, mpc::ranges::input_range R>
      constexpr auto operator()(Fn&& op, T&& init, R&& r) const -> T {
        return this->operator()(std::forward<Fn>(op), std::forward<T>(init), mpc::ranges::begin(r),
                                mpc::ranges::end(r));
      }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr partial<detail::cons_op> cons;

    inline constexpr partial<detail::foldr_op> foldr;
  } // namespace cpo

  // instances
  template <class T>
  struct functor_traits<std::list<T>> {
    struct fmap_op {
      template <class Fn, is_list L>
      constexpr auto operator()(Fn f, L&& l) const {
        using U = std::remove_cvref_t<std::invoke_result_t<Fn&, mpc::ranges::range_reference_t<L>>>;
        std::list<U> ret(l.size());
        std::transform(l.begin(), l.end(), ret.begin(), std::move(f));
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
      template <class Fn, is_list L1, is_list L2>
      constexpr auto operator()(Fn f, L1&& l1, L2&& l2) const {
        using U = std::remove_cvref_t<std::invoke_result_t<Fn&, mpc::ranges::range_reference_t<L1>,
                                                           mpc::ranges::range_reference_t<L2>>>;
        const auto n = std::min(l1.size(), l2.size());
        std::list<U> ret(n);
        std::transform(l1.begin(), l1.end(), l2.begin(), l2.end(), ret.begin(), std::move(f));
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
  // https://hackage.haskell.org/package/base-4.17.0.0/docs/src/Data.Traversable.html#traverse
  // https://hackage.haskell.org/package/base-4.17.0.0/docs/src/Data.Traversable.html#Traversable
  // traverse :: Applicative f => (a -> f b) -> t a -> f (t b)
  // traverse f = sequenceA . fmap f
  //
  // sequenceA :: Applicative f => t (f a) -> f (t a)
  // sequenceA = traverse id
  //
  // instance Traversable [] where
  //   traverse f = List.foldr cons_f (pure [])
  //     where cons_f x ys = liftA2 (:) (f x) ys
  namespace detail {
    struct sequence_op {
      // FIXME: 本来 applicative T
      template <monad T>
      constexpr auto operator()(const std::list<T>& l) const {
        // FIXME: 不正な方法で monad の value_type を取得している
        using U = holding_or_t<T, std::remove_cvref_t<decltype(mpc::bind(l.front(), identity))>>;
        if constexpr (false)
          return foldr(mpc::liftA2 % cons, mpc::returns<T> % std::string{}, l);
        else
          return foldr(mpc::liftA2 % cons, mpc::returns<T> % std::list<U>{}, l);
      }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr partial<detail::sequence_op> sequence;
  } // namespace cpo
} // namespace mpc
