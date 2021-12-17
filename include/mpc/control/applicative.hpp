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
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html#t:Applicative

  /// class Functor f => Applicative f where
  template <class>
  struct applicative_traits;

  /// applicative_traits_specialized
  template <class F>
  concept applicative_traits_specialized = requires {
    applicative_traits<std::remove_cvref_t<F>>::pure;
    applicative_traits<std::remove_cvref_t<F>>::seq_apply;
    applicative_traits<std::remove_cvref_t<F>>::liftA2;
    applicative_traits<std::remove_cvref_t<F>>::discard2nd;
    applicative_traits<std::remove_cvref_t<F>>::discard1st;
  };

  /// Requires functor and pure, seq_apply, liftA2, discard2nd and discard1st is valid in @link mpc::applicative_traits applicative_traits @endlink.
  template <class F>
  concept applicative = functor<F> and applicative_traits_specialized<F>;

  // Methods required for the class definition.
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

    /**
     * @brief seq_apply :: f (a -> b) -> f a -> f b
     * @details (<*>) in Haskell
     */
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

    /**
     * @brief discard2nd :: f a -> f b -> f a
     * @details (<*) in Haskell
     */
    struct discard2nd_op {
      template <class Fa, class Fb>
      constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
      noexcept(   applicative_traits<std::remove_cvref_t<Fa>>::discard2nd(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(applicative_traits<std::remove_cvref_t<Fa>>::discard2nd(std::forward<Fa>(fa), std::forward<Fb>(fb)))
      { return    applicative_traits<std::remove_cvref_t<Fa>>::discard2nd(std::forward<Fa>(fa), std::forward<Fb>(fb)); }
    };

    /**
     * @brief discard1st :: f a -> f b -> f b
     * @details (*>) in Haskell
     */
    struct discard1st_op {
      template <class Fa, class Fb>
      constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
      noexcept(   applicative_traits<std::remove_cvref_t<Fa>>::discard1st(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(applicative_traits<std::remove_cvref_t<Fa>>::discard1st(std::forward<Fa>(fa), std::forward<Fb>(fb)))
      { return    applicative_traits<std::remove_cvref_t<Fa>>::discard1st(std::forward<Fa>(fa), std::forward<Fb>(fb)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// @copydoc mpc::detail::pure_op
    template <class F>
    inline constexpr perfect_forwarded_t<detail::pure_op<F>> pure{};

    /// @copydoc mpc::detail::seq_apply_op
    inline constexpr perfect_forwarded_t<detail::seq_apply_op> seq_apply{};

    /// @copydoc mpc::detail::liftA2_op
    inline constexpr perfect_forwarded_t<detail::liftA2_op> liftA2{};

    /// @copydoc mpc::detail::discard2nd_op
    inline constexpr perfect_forwarded_t<detail::discard2nd_op> discard2nd{};

    /// @copydoc mpc::detail::discard1st_op
    inline constexpr perfect_forwarded_t<detail::discard1st_op> discard1st{};
  } // namespace cpo

  /// Methods deducible from other methods of @link mpc::applicative applicative @endlink.
  namespace applicatives {
    namespace detail {
      /**
       * @copydoc mpc::detail::fmap_op
       * @details
       * ```
       * fmap f x = seq_apply (pure f) x
       * ```
       */
      struct fmap_op {
        template <class Fn, class Fa>
        constexpr auto operator()(Fn&& fn, Fa&& fa) const noexcept(
        noexcept(   mpc::seq_apply(mpc::pure<Fa>(std::forward<Fn>(fn)), std::forward<Fa>(fa))))
        -> decltype(mpc::seq_apply(mpc::pure<Fa>(std::forward<Fn>(fn)), std::forward<Fa>(fa)))
        { return    mpc::seq_apply(mpc::pure<Fa>(std::forward<Fn>(fn)), std::forward<Fa>(fa)); }
      };

      /**
       * @copydoc mpc::detail::liftA2_op
       * @details
       * ```
       * liftA2 f x y = seq_apply (fmap f x) y
       * ```
       */
      struct liftA2_op {
        template <class Fn, class Fa, class Fb>
        constexpr auto operator()(Fn&& fn, Fa&& fa, Fb&& fb) const noexcept(
        noexcept(   mpc::seq_apply(mpc::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)), std::forward<Fb>(fb))))
        -> decltype(mpc::seq_apply(mpc::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)), std::forward<Fb>(fb)))
        { return    mpc::seq_apply(mpc::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)), std::forward<Fb>(fb)); }
      };

      /**
       * @copydoc mpc::detail::discard1st_op
       * @details
       * ```
       * discard1st x y = (id <$ x) <*> y
       * ```
       */
      struct discard1st_opt_op {
        template <class Fa, class Fb>
        constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
        noexcept(   mpc::seq_apply(mpc::replace2nd(id, std::forward<Fa>(fa)), std::forward<Fb>(fb))))
        -> decltype(mpc::seq_apply(mpc::replace2nd(id, std::forward<Fa>(fa)), std::forward<Fb>(fb)))
        { return    mpc::seq_apply(mpc::replace2nd(id, std::forward<Fa>(fa)), std::forward<Fb>(fb)); }
      };
    } // namespace detail

    /// @copydoc mpc::applicatives::detail::fmap_op
    inline constexpr perfect_forwarded_t<detail::fmap_op> fmap{};

    /**
     * @copydoc mpc::detail::seq_apply_op
     * @details
     * ```
     * seq_apply = liftA2 id
     * ```
     */
    inline constexpr auto seq_apply = mpc::liftA2 % id;

    /// @copydoc mpc::applicatives::detail::liftA2_op
    inline constexpr perfect_forwarded_t<detail::liftA2_op> liftA2{};

    /**
     * @copydoc mpc::detail::discard2nd_op
     * @details
     * ```
     * discard2nd = liftA2 const
     * ```
     */
    inline constexpr auto discard2nd = mpc::liftA2 % constant;

    /**
     * @copydoc mpc::detail::discard1st_op
     * @details
     * ```
     * discard1st = liftA2 (flip const)
     * ```
     */
    inline constexpr auto discard1st = mpc::liftA2 % (flip % constant);

    /// @copydoc mpc::applicatives::detail::discard1st_opt_op
    inline constexpr perfect_forwarded_t<detail::discard1st_opt_op> discard1st_opt{};
  } // namespace applicatives

  // Grobal methods
  namespace detail {
    /// @cond undocumented
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
    /// @endcond undocumented

    /**
     * @brief liftA :: Applicative f => (a -> b -> ... -> z) -> f a -> f b -> ... -> f z
     * @details
     * ```
     * liftA f a b ... z = f `fmap` a `seq_apply` b `seq_apply` ... `seq_apply` z
     * ```
     */
    template <std::size_t N, class = make_reversed_index_sequence<N + 1>>
    struct liftA_op;

    /// @spec liftA
    template <std::size_t N, std::size_t... Idx>
    requires (N > 1)
    struct liftA_op <N, std::index_sequence<Idx...>> {
      // WORKAROUND: std::tuple_element (and therefore, std::get) is not SFINAE-friendly in Clang.
      template <class Bound, class = std::enable_if_t<sizeof...(Idx) == std::tuple_size_v<Bound>>>
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
    /// @copydoc mpc::detail::liftA_op
    template <std::size_t N>
    requires (N > 1)
    inline constexpr perfect_forwarded_t<detail::liftA_op<N>> liftA{};
  } // namespace cpo
} // namespace mpc

// clang-format on
