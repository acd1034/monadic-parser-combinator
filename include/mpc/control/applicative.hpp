/// @file applicative.hpp
#pragma once
#include <mpc/control/functor.hpp>
#include <mpc/functional/perfect_forward.hpp>
#include <mpc/prelude/constant.hpp>
#include <mpc/prelude/flip.hpp>
#include <mpc/prelude/id.hpp>
#include <mpc/type_traits.hpp>

// clang-format off

namespace mpc {
  // applicative
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html

  /// class Functor f => Applicative f where
  template <class>
  struct applicative_traits;

  template <class F>
  concept applicative_traits_specialized = requires {
    applicative_traits<std::remove_cvref_t<F>>::pure;
    applicative_traits<std::remove_cvref_t<F>>::seq_apply;
    applicative_traits<std::remove_cvref_t<F>>::liftA2;
    applicative_traits<std::remove_cvref_t<F>>::discard2nd;
    applicative_traits<std::remove_cvref_t<F>>::discard1st;
  };

  template <class F>
  concept applicative = functor<F> and applicative_traits_specialized<F>;

  // class requirements

  namespace detail {
    /// pure :: a -> f a
    template <class F>
    struct pure_op {
      template <class A>
      constexpr auto operator()(A&& a) const noexcept(
      noexcept(   applicative_traits<std::remove_cvref_t<F>>::pure(std::forward<A>(a))))
      -> decltype(applicative_traits<std::remove_cvref_t<F>>::pure(std::forward<A>(a)))
      { return    applicative_traits<std::remove_cvref_t<F>>::pure(std::forward<A>(a)); }
    };

    /// seq_apply :: f (a -> b) -> f a -> f b
    struct seq_apply_op {
      template <class Fab, class Fa>
      constexpr auto operator()(Fab&& fab, Fa&& fa) const noexcept(
      noexcept(   applicative_traits<std::remove_cvref_t<Fab>>::seq_apply(std::forward<Fab>(fab), std::forward<Fa>(fa))))
      -> decltype(applicative_traits<std::remove_cvref_t<Fab>>::seq_apply(std::forward<Fab>(fab), std::forward<Fa>(fa)))
      { return    applicative_traits<std::remove_cvref_t<Fab>>::seq_apply(std::forward<Fab>(fab), std::forward<Fa>(fa)); }
    };

    /// liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    struct liftA2_op {
      template <class Fn, class Fa, class Fb>
      constexpr auto operator()(Fn&& fn, Fa&& fa, Fb&& fb) const noexcept(
      noexcept(   applicative_traits<std::remove_cvref_t<Fa>>::liftA2(std::forward<Fn>(fn), std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(applicative_traits<std::remove_cvref_t<Fa>>::liftA2(std::forward<Fn>(fn), std::forward<Fa>(fa), std::forward<Fb>(fb)))
      { return    applicative_traits<std::remove_cvref_t<Fa>>::liftA2(std::forward<Fn>(fn), std::forward<Fa>(fa), std::forward<Fb>(fb)); }
    };

    /// discard2nd :: f a -> f b -> f a
    struct discard2nd_op {
      template <class Fa, class Fb>
      constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
      noexcept(   applicative_traits<std::remove_cvref_t<Fa>>::discard2nd(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(applicative_traits<std::remove_cvref_t<Fa>>::discard2nd(std::forward<Fa>(fa), std::forward<Fb>(fb)))
      { return    applicative_traits<std::remove_cvref_t<Fa>>::discard2nd(std::forward<Fa>(fa), std::forward<Fb>(fb)); }
    };

    /// discard1st :: f a -> f b -> f b
    struct discard1st_op {
      template <class Fa, class Fb>
      constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
      noexcept(   applicative_traits<std::remove_cvref_t<Fa>>::discard1st(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(applicative_traits<std::remove_cvref_t<Fa>>::discard1st(std::forward<Fa>(fa), std::forward<Fb>(fb)))
      { return    applicative_traits<std::remove_cvref_t<Fa>>::discard1st(std::forward<Fa>(fa), std::forward<Fb>(fb)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// pure :: a -> f a
    template <class F>
    inline constexpr perfect_forwarded_t<detail::pure_op<F>> pure{};

    /// seq_apply :: f (a -> b) -> f a -> f b
    inline constexpr perfect_forwarded_t<detail::seq_apply_op> seq_apply{};

    /// liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    inline constexpr perfect_forwarded_t<detail::liftA2_op> liftA2{};

    /// discard2nd :: f a -> f b -> f a
    inline constexpr perfect_forwarded_t<detail::discard2nd_op> discard2nd{};

    /// discard1st :: f a -> f b -> f b
    inline constexpr perfect_forwarded_t<detail::discard1st_op> discard1st{};
  } // namespace cpo

  // Deducibles

  namespace applicatives {
    namespace detail {
      /// fmap :: (a -> b) -> f a -> f b
      /// fmap f x = seq_apply (pure f) x
      struct fmap_op {
        template <class Fn, class Fa>
        constexpr auto operator()(Fn&& fn, Fa&& fa) const noexcept(
        noexcept(   mpc::seq_apply(mpc::pure<Fa>(std::forward<Fn>(fn)), std::forward<Fa>(fa))))
        -> decltype(mpc::seq_apply(mpc::pure<Fa>(std::forward<Fn>(fn)), std::forward<Fa>(fa)))
        { return    mpc::seq_apply(mpc::pure<Fa>(std::forward<Fn>(fn)), std::forward<Fa>(fa)); }
      };

      /// liftA2 :: (a -> b -> c) -> f a -> f b -> f c
      /// liftA2 f x y = seq_apply (fmap f x) y
      struct liftA2_op {
        template <class Fn, class Fa, class Fb>
        constexpr auto operator()(Fn&& fn, Fa&& fa, Fb&& fb) const noexcept(
        noexcept(   mpc::seq_apply(mpc::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)), std::forward<Fb>(fb))))
        -> decltype(mpc::seq_apply(mpc::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)), std::forward<Fb>(fb)))
        { return    mpc::seq_apply(mpc::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)), std::forward<Fb>(fb)); }
      };

      /// discard1st_opt :: f a -> f b -> f b
      /// discard1st_opt x y = (id <$ x) <*> y
      struct discard1st_opt_op {
        template <class Fa, class Fb>
        constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
        noexcept(   mpc::seq_apply(mpc::replace2nd(id, std::forward<Fa>(fa)), std::forward<Fb>(fb))))
        -> decltype(mpc::seq_apply(mpc::replace2nd(id, std::forward<Fa>(fa)), std::forward<Fb>(fb)))
        { return    mpc::seq_apply(mpc::replace2nd(id, std::forward<Fa>(fa)), std::forward<Fb>(fb)); }
      };
    } // namespace detail

    /// @brief fmap f x = seq_apply (pure f) x
    /// @details If you fully specialize `applicative_traits<F>`, you can deduce `fmap`.
    inline constexpr perfect_forwarded_t<detail::fmap_op> fmap{};

    /// @brief seq_apply = liftA2 id
    /// @details If you define `applicative_traits<F>::liftA2`, you can deduce `seq_apply`.
    inline constexpr auto seq_apply = mpc::liftA2 % id;

    /// @brief liftA2 f x y = seq_apply (fmap f x) y
    /// @details If you define `applicative_traits<F>::seq_apply`, you can deduce `liftA2`.
    inline constexpr perfect_forwarded_t<detail::liftA2_op> liftA2{};

    /// @brief discard2nd = liftA2 const
    /// @details If you define `applicative_traits<F>::seq_apply` and
    /// `applicative_traits<F>::liftA2`, you can deduce `discard2nd`.
    inline constexpr auto discard2nd = mpc::liftA2 % constant;

    /// @brief discard1st = liftA2 (flip const)
    /// @details If you define `applicative_traits<F>::seq_apply` and
    /// `applicative_traits<F>::liftA2`, you can deduce `discard1st`.
    inline constexpr auto discard1st = mpc::liftA2 % (flip % constant);

    /// @brief discard1st_opt x y = seq_apply (id <$ x) y
    /// @details If you optimize `functor_traits<F>::replace2nd`, you can deduce optimized
    /// `discard1st`.
    inline constexpr perfect_forwarded_t<detail::discard1st_opt_op> discard1st_opt{};
  } // namespace applicatives

  // Grobal methods

  namespace detail {
    template <class Fn, class Fa, class Fb>
    constexpr auto reversed_liftA(Fb&& fb, Fa&& fa, Fn&& fn) noexcept(
    noexcept(   mpc::liftA2(std::forward<Fn>(fn), std::forward<Fa>(fa), std::forward<Fb>(fb))))
    -> decltype(mpc::liftA2(std::forward<Fn>(fn), std::forward<Fa>(fa), std::forward<Fb>(fb)))
    { return    mpc::liftA2(std::forward<Fn>(fn), std::forward<Fa>(fa), std::forward<Fb>(fb)); }

    template <class Fz, class... Args>
    requires (sizeof...(Args) > 2)
    constexpr auto reversed_liftA(Fz&& fz, Args&&... args) noexcept(
    noexcept(   mpc::seq_apply(reversed_liftA(std::forward<Args>(args)...), std::forward<Fz>(fz))))
    -> decltype(mpc::seq_apply(reversed_liftA(std::forward<Args>(args)...), std::forward<Fz>(fz)))
    { return    mpc::seq_apply(reversed_liftA(std::forward<Args>(args)...), std::forward<Fz>(fz)); }

    template <std::size_t N, class = make_reversed_index_sequence<N + 1>>
    struct liftA_op;

    template <std::size_t N, std::size_t... Idx>
    requires (N > 2)
    struct liftA_op<N, std::index_sequence<Idx...>> {
      template <class Bound>
      constexpr auto operator()(Bound&& bound) const noexcept(
      noexcept(   reversed_liftA(std::get<Idx>(std::forward<Bound>(bound))...)))
      -> decltype(reversed_liftA(std::get<Idx>(std::forward<Bound>(bound))...))
      { return    reversed_liftA(std::get<Idx>(std::forward<Bound>(bound))...); }

      template <class... Args>
      requires (sizeof...(Args) == N + 1)
      constexpr auto operator()(Args&&... args) const noexcept(
      noexcept(   this->operator()(std::forward_as_tuple(std::forward<Args>(args)...))))
      -> decltype(this->operator()(std::forward_as_tuple(std::forward<Args>(args)...)))
      { return    this->operator()(std::forward_as_tuple(std::forward<Args>(args)...)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// liftA :: Applicative f => (a -> b -> .. -> z) -> f a -> f b -> .. -> f z
    template <std::size_t N>
    requires (N > 1)
    inline constexpr perfect_forwarded_t<detail::liftA_op<N>> liftA{};
  } // namespace cpo
} // namespace mpc

// clang-format on
