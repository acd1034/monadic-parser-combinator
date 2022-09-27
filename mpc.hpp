#ifndef RICH_SINGLE_HEADER_INCLUDED
#define RICH_SINGLE_HEADER_INCLUDED
#include <functional> // std::invoke
#include <memory>
#include <optional>
#include <cassert> // assert
#include <compare>
#include <concepts>
#include <cstddef>          // size_t, ptrdiff_t, nullptr_t
#include <cstdint>          // int32_t
#include <initializer_list> // initializer_list
#include <tuple>            // tuple
#include <type_traits>      // enable_if_t, void_t, true_type, invoke_result, etc.
#include <cctype>
#include <string>
#include <variant>
#include <algorithm>  // std::transform
#include <iterator>
#include <list>
#include <ranges>
#include <iostream>
#include <string_view>

//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/mpc.hpp
/// @file mpc.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control.hpp
/// @file control.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/alternative.hpp
/// @file alternative.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/applicative.hpp
/// @file applicative.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/functor.hpp
/// @file functor.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/functional/partial.hpp
/// @file partial.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/copyable_box.hpp
/// @file copyable_box.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/stdfundamental.hpp
/// @file stdfundamental.hpp
// #include <utility>          // move, forward, pair, swap, exchange, declval

namespace mpc {
  /// always_false
  template <class...>
  inline constexpr bool always_false = false;

  /// always_true_type
  template <class...>
  using always_true_type = std::true_type;

  /// index_constant
  template <std::size_t N>
  using index_constant = std::integral_constant<std::size_t, N>;

  /// _and
  template <class... Bn>
  using _and = std::conjunction<Bn...>;

  /// _or
  template <class... Bn>
  using _or = std::disjunction<Bn...>;

  /// _not
  template <class B>
  using _not = std::negation<B>;

  /// _and_v
  template <class... Bn>
  inline constexpr bool _and_v = _and<Bn...>::value;

  /// _or_v
  template <class... Bn>
  inline constexpr bool _or_v = _or<Bn...>::value;

  /// _not_v
  template <class B>
  inline constexpr bool _not_v = _not<B>::value;
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/stdfundamental.hpp

namespace mpc {
  // copyable_box
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__ranges/copyable_box.h

  // copyable_box allows turning a type that is copy-constructible (but maybe not copy-assignable)
  // into a type that is both copy-constructible and copy-assignable. It does that by introducing an
  // empty state and basically doing destroy-then-copy-construct in the assignment operator. The
  // empty state is necessary to handle the case where the copy construction fails after destroying
  // the object.
  //
  // In some cases, we can completely avoid the use of an empty state; we provide a specialization
  // of copyable_box that does this, see below for the details.

  /// Requires copy_constructible and is_object.
  template <class T>
  concept copy_constructible_object = std::copy_constructible<T> and std::is_object_v<T>;

  // Primary template - uses std::optional and introduces an empty state in case assignment fails.

  /// Makes copy_constructible but not copy_assignable types copy_assignable.
  template <copy_constructible_object T>
  class copyable_box {
    [[no_unique_address]] std::optional<T> instance_ = std::nullopt;

  public:
    template <class... Args>
    requires std::is_constructible_v<T, Args...>
    constexpr explicit copyable_box(std::in_place_t, Args&&... args) //
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : instance_(std::in_place, std::forward<Args>(args)...) {}

    constexpr copyable_box()                               //
      noexcept(std::is_nothrow_default_constructible_v<T>) //
      requires std::default_initializable<T> : instance_(std::in_place) {}

    constexpr copyable_box(const copyable_box&) = default;
    constexpr copyable_box(copyable_box&&) = default;

    constexpr copyable_box&
    operator=(const copyable_box& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
      if (this != std::addressof(other)) {
        if (other.has_value())
          instance_.emplace(*other);
        else
          instance_.reset();
      }
      return *this;
    }

    constexpr copyable_box& operator=(copyable_box&&) requires std::movable<T>
    = default;

    constexpr copyable_box&
    operator=(copyable_box&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
      if (this != std::addressof(other)) {
        if (other.has_value())
          instance_.emplace(std::move(*other));
        else
          instance_.reset();
      }
      return *this;
    }

    constexpr const T& operator*() const& noexcept {
      return *instance_;
    }
    constexpr const T&& operator*() const&& noexcept {
      return std::move(*instance_);
    }
    constexpr T& operator*() & noexcept {
      return *instance_;
    }
    constexpr T&& operator*() && noexcept {
      return std::move(*instance_);
    }

    constexpr const T* operator->() const noexcept {
      return instance_.operator->();
    }
    constexpr T* operator->() noexcept {
      return instance_.operator->();
    }

    constexpr bool has_value() const noexcept {
      return instance_.has_value();
    }
  };

  // This partial specialization implements an optimization for when we know we don't need to store
  // an empty state to represent failure to perform an assignment. For copy-assignment, this
  // happens:
  //
  // 1. If the type is copyable (which includes copy-assignment), we can use the type's own
  // assignment operator
  //    directly and avoid using std::optional.
  // 2. If the type is not copyable, but it is nothrow-copy-constructible, then we can implement
  // assignment as
  //    destroy-and-then-construct and we know it will never fail, so we don't need an empty state.
  //
  // The exact same reasoning can be applied for move-assignment, with copyable replaced by movable
  // and nothrow-copy-constructible replaced by nothrow-move-constructible. This specialization is
  // enabled whenever we can apply any of these optimizations for both the copy assignment and the
  // move assignment operator.

  /// @cond undocumented
  template <class T>
  concept doesnt_need_empty_state_for_copy =
    std::copyable<T> or std::is_nothrow_copy_constructible_v<T>;

  template <class T>
  concept doesnt_need_empty_state_for_move =
    std::movable<T> or std::is_nothrow_move_constructible_v<T>;
  /// @endcond undocumented

  /// @spec copyable_box
  template <copy_constructible_object T>
  requires doesnt_need_empty_state_for_copy<T> and doesnt_need_empty_state_for_move<T>
  class copyable_box<T> {
    [[no_unique_address]] T instance_{};

  public:
    template <class... Args>
    requires std::is_constructible_v<T, Args...>
    constexpr explicit copyable_box(std::in_place_t, Args&&... args) //
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : instance_(std::forward<Args>(args)...) {}

    constexpr copyable_box()                               //
      noexcept(std::is_nothrow_default_constructible_v<T>) //
      requires std::default_initializable<T> : instance_{} {}

    constexpr copyable_box(const copyable_box&) = default;
    constexpr copyable_box(copyable_box&&) = default;

    // Implementation of assignment operators in case we perform optimization (1)
    constexpr copyable_box& operator=(const copyable_box&) requires std::copyable<T>
    = default;
    constexpr copyable_box& operator=(copyable_box&&) requires std::movable<T>
    = default;

    // Implementation of assignment operators in case we perform optimization (2)
    constexpr copyable_box& operator=(const copyable_box& other) noexcept {
      static_assert(std::is_nothrow_copy_constructible_v<T>);
      if (this != std::addressof(other)) {
        std::destroy_at(std::addressof(instance_));
        std::construct_at(std::addressof(instance_), other.instance_);
      }
      return *this;
    }

    constexpr copyable_box& operator=(copyable_box&& other) noexcept {
      static_assert(std::is_nothrow_move_constructible_v<T>);
      if (this != std::addressof(other)) {
        std::destroy_at(std::addressof(instance_));
        std::construct_at(std::addressof(instance_), std::move(other.instance_));
      }
      return *this;
    }

    constexpr const T& operator*() const& noexcept {
      return instance_;
    }
    constexpr const T&& operator*() const&& noexcept {
      return std::move(instance_);
    }
    constexpr T& operator*() & noexcept {
      return instance_;
    }
    constexpr T&& operator*() && noexcept {
      return std::move(instance_);
    }

    constexpr const T* operator->() const noexcept {
      return std::addressof(instance_);
    }
    constexpr T* operator->() noexcept {
      return std::addressof(instance_);
    }

    constexpr bool has_value() const noexcept {
      return true;
    }
  };
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/copyable_box.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/forward_like.hpp
/// @file forward_like.hpp

namespace mpc {
  template <class T, class U>
  using __override_ref_t =
    std::conditional_t<std::is_rvalue_reference_v<T>, std::remove_reference_t<U>&&, U&>;

  template <class T, class U>
  using __copy_const_t =
    std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, U const, U>;

  template <class T, class U>
  using forward_like_t = __override_ref_t<T&&, __copy_const_t<T, std::remove_reference_t<U>>>;

  template <typename T>
  [[nodiscard]] constexpr auto forward_like(auto&& x) noexcept -> forward_like_t<T, decltype(x)> {
    return static_cast<forward_like_t<T, decltype(x)>>(x);
  }
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/forward_like.hpp

// clang-format off

namespace mpc {
  // partial
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__functional/perfect_forward.h

  /// Implements a perfect-forwarding call wrapper.
  template <copy_constructible_object Op, class... Bound>
  struct partial;

  template <class T>
  inline constexpr bool is_partial_v = false;

  template <copy_constructible_object Op, class... Bound>
  inline constexpr bool is_partial_v<partial<Op, Bound...>> = true;

  template <class Op, class Tuple, std::size_t... Idx, class... Args>
  constexpr auto make_partial_impl(Op&& op, Tuple&& tuple, std::index_sequence<Idx...>, Args&&... args) noexcept(
    noexcept(   partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)))
    -> decltype(partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)) {
    return      partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...);
  }

  template <class Op, class... Args,
            class = std::enable_if_t<is_partial_v<std::remove_cvref_t<Op>>>>
  constexpr auto make_partial(Op&& op, Args&&... args) noexcept(
    noexcept(   make_partial_impl(forward_like<Op>(op.op_), forward_like<Op>(op.bound_), std::make_index_sequence<std::tuple_size_v<decltype(op.bound_)>>(), std::forward<Args>(args)...)))
    -> decltype(make_partial_impl(forward_like<Op>(op.op_), forward_like<Op>(op.bound_), std::make_index_sequence<std::tuple_size_v<decltype(op.bound_)>>(), std::forward<Args>(args)...)) {
    return      make_partial_impl(forward_like<Op>(op.op_), forward_like<Op>(op.bound_), std::make_index_sequence<std::tuple_size_v<decltype(op.bound_)>>(), std::forward<Args>(args)...);
  }

  template <class Op, class... Args,
            class = std::enable_if_t<!is_partial_v<std::remove_cvref_t<Op>>>>
  constexpr auto make_partial(Op&& op, Args&&... args) noexcept(
    noexcept(   partial(std::forward<Op>(op), std::forward<Args>(args)...)))
    -> decltype(partial(std::forward<Op>(op), std::forward<Args>(args)...)) {
    return      partial(std::forward<Op>(op), std::forward<Args>(args)...);
  }

  template <class Op, class Tuple, std::size_t... Idx, class... Args,
            class = std::enable_if_t<std::is_invocable_v<Op, forward_like_t<Tuple, std::tuple_element_t<Idx, std::remove_cvref_t<Tuple>>>..., Args...>>>
  constexpr auto __call(Op&& op, Tuple&& tuple, std::index_sequence<Idx...>, Args&&... args) noexcept(
    noexcept(   std::invoke(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)))
    -> decltype(std::invoke(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)) {
    return      std::invoke(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...);
  }

  template <class Op, class Tuple, std::size_t... Idx, class... Args,
            class = std::enable_if_t<!std::is_invocable_v<Op, forward_like_t<Tuple, std::tuple_element_t<Idx, std::remove_cvref_t<Tuple>>>..., Args...>>>
  constexpr auto __call(Op&& op, Tuple&& tuple, std::index_sequence<Idx...>, Args&&... args) noexcept(
    noexcept(   partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)))
    -> decltype(partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...)) {
    return      partial(std::forward<Op>(op), std::get<Idx>(std::forward<Tuple>(tuple))..., std::forward<Args>(args)...);
  }

  template <copy_constructible_object Op, class... Bound>
  struct partial {
  private:
    copyable_box<Op> op_{};
    std::tuple<Bound...> bound_{};

    template <class Op2, class... Args, class>
    friend constexpr auto make_partial(Op2&&, Args&&...);

  public:
    constexpr explicit partial()
    requires std::default_initializable<Op> and (std::default_initializable<Bound> and ...) = default;

    template <class... BoundArgs,
              class = std::enable_if_t<std::is_constructible_v<std::tuple<Bound...>, BoundArgs&&...>>>
    constexpr explicit partial(Op const& op, BoundArgs&&... bound)
      : op_(std::in_place, op), bound_(std::forward<BoundArgs>(bound)...) {}

    template <class... BoundArgs,
              class = std::enable_if_t<std::is_constructible_v<std::tuple<Bound...>, BoundArgs&&...>>>
    constexpr explicit partial(Op&& op, BoundArgs&&... bound)
      : op_(std::in_place, std::move(op)), bound_(std::forward<BoundArgs>(bound)...) {}

    partial(partial const&) = default;
    partial(partial&&) = default;

    partial& operator=(partial const&) = default;
    partial& operator=(partial&&) = default;

    // operator()
    template <class... Args>
    constexpr auto operator()(Args&&... args) & noexcept(
      noexcept(   __call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)))
      -> decltype(__call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)) {
      return      __call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) const& noexcept(
      noexcept(   __call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)))
      -> decltype(__call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)) {
      return      __call(*op_, bound_, std::index_sequence_for<Bound...>(), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) && noexcept(
      noexcept(   __call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)))
      -> decltype(__call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)) {
      return      __call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr auto operator()(Args&&... args) const&& noexcept(
      noexcept(   __call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)))
      -> decltype(__call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...)) {
      return      __call(*std::move(op_), std::move(bound_), std::index_sequence_for<Bound...>(), std::forward<Args>(args)...);
    }

    // operator%
    template <class Arg>
    constexpr auto operator%(Arg&& arg) & noexcept(
      noexcept(   (*this)(std::forward<Arg>(arg))))
      -> decltype((*this)(std::forward<Arg>(arg))) {
      return      (*this)(std::forward<Arg>(arg));
    }

    template <class Arg>
    constexpr auto operator%(Arg&& arg) const& noexcept(
      noexcept(   (*this)(std::forward<Arg>(arg))))
      -> decltype((*this)(std::forward<Arg>(arg))) {
      return      (*this)(std::forward<Arg>(arg));
    }

    template <class Arg>
    constexpr auto operator%(Arg&& arg) && noexcept(
      noexcept(   std::move(*this)(std::forward<Arg>(arg))))
      -> decltype(std::move(*this)(std::forward<Arg>(arg))) {
      return      std::move(*this)(std::forward<Arg>(arg));
    }

    template <class Arg>
    constexpr auto operator%(Arg&& arg) const&& noexcept(
      noexcept(   std::move(*this)(std::forward<Arg>(arg))))
      -> decltype(std::move(*this)(std::forward<Arg>(arg))) {
      return      std::move(*this)(std::forward<Arg>(arg));
    }
  };

  /// @dguide partial
  template <class Op, class... Args>
  partial(Op, Args...) -> partial<Op, Args...>;
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/functional/partial.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/compose.hpp
/// @file compose.hpp

// clang-format off

namespace mpc {
  // compose
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__functional/compose.h

  namespace detail {
    struct compose_op {
      struct closure {
        template<class Fn1, class Fn2, class... Args>
        constexpr auto operator()(Fn1&& f1, Fn2&& f2, Args&&... args) const noexcept(
          noexcept(   std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...))))
          -> decltype(std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...)))
          { return    std::invoke(std::forward<Fn1>(f1), std::invoke(std::forward<Fn2>(f2), std::forward<Args>(args)...)); }
      };

      // NOTE: You cannot write as `partial<closure>{}(std::forward<Fn1>(f1), std::forward<Fn2>(f2))`.
      template<class Fn1, class Fn2>
      constexpr auto operator()(Fn1&& f1, Fn2&& f2) const noexcept(
        noexcept(   partial(closure{}, std::forward<Fn1>(f1), std::forward<Fn2>(f2))))
        -> decltype(partial(closure{}, std::forward<Fn1>(f1), std::forward<Fn2>(f2)))
        { return    partial(closure{}, std::forward<Fn1>(f1), std::forward<Fn2>(f2)); }
    };
  } // namespace detail

  inline namespace cpo {
    /**
     * @brief Function composition.
     */
    inline constexpr partial<detail::compose_op> compose;
  }
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/compose.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/constant.hpp
/// @file constant.hpp

namespace mpc {
  namespace detail {
    struct constant_op {
      template <class T, class U>
      constexpr auto operator()(T&& t, U&&) const //
        noexcept(noexcept(std::forward<T>(t)))    //
        -> decltype(std::forward<T>(t)) {
        return std::forward<T>(t);
      }
    };
  } // namespace detail

  inline namespace cpo {
    /**
     * @brief Returns a unary function always returning the first input.
     */
    inline constexpr partial<detail::constant_op> constant;
  } // namespace cpo
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/constant.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/flip.hpp
/// @file flip.hpp

// clang-format off

namespace mpc {
  namespace detail {
    struct flip_op {
      struct closure {
        template<class Fn, class T, class U, class... Args>
        constexpr auto operator()(Fn&& f, T&& t, U&& u, Args&&... args) const noexcept(
          noexcept(   std::invoke(std::forward<Fn>(f), std::forward<U>(u), std::forward<T>(t), std::forward<Args>(args)...)))
          -> decltype(std::invoke(std::forward<Fn>(f), std::forward<U>(u), std::forward<T>(t), std::forward<Args>(args)...))
          { return    std::invoke(std::forward<Fn>(f), std::forward<U>(u), std::forward<T>(t), std::forward<Args>(args)...); }
      };

      // NOTE: You cannot write as `partial<closure>{}(std::forward<Fn>(f))`.
      template<class Fn>
      constexpr auto operator()(Fn&& f) const noexcept(
        noexcept(   partial(closure{}, std::forward<Fn>(f))))
        -> decltype(partial(closure{}, std::forward<Fn>(f)))
        { return    partial(closure{}, std::forward<Fn>(f)); }
    };
  } // namespace detail

  inline namespace cpo {
    /**
     * @brief Returns a binary function which flips the first and second argument.
     */
    inline constexpr partial<detail::flip_op> flip;
  }
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/flip.hpp

namespace mpc {
  // functor
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html#t:Functor

  /// class Functor f where
  template <class>
  struct functor_traits;

  /// Requires fmap and replace2nd is valid in @link mpc::functor_traits functor_traits @endlink.
  template <class F>
  concept functor = requires {
    functor_traits<std::remove_cvref_t<F>>::fmap;
    functor_traits<std::remove_cvref_t<F>>::replace2nd;
  };

  // Methods required for the class definition.

  namespace detail {
    /// fmap :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class Fn, class Fa>
      constexpr auto operator()(Fn&& fn, Fa&& fa) const noexcept(
      noexcept(   functor_traits<std::remove_cvref_t<Fa>>::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa))))
      -> decltype(functor_traits<std::remove_cvref_t<Fa>>::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)))
      { return    functor_traits<std::remove_cvref_t<Fa>>::fmap(std::forward<Fn>(fn), std::forward<Fa>(fa)); }
    };

    /**
     * @brief replace2nd :: a -> f b -> f a
     * @details (<$) in Haskell
     */
    struct replace2nd_op {
      template <class A, class Fb>
      constexpr auto operator()(A&& a, Fb&& fb) const noexcept(
      noexcept(   functor_traits<std::remove_cvref_t<Fb>>::replace2nd(std::forward<A>(a), std::forward<Fb>(fb))))
      -> decltype(functor_traits<std::remove_cvref_t<Fb>>::replace2nd(std::forward<A>(a), std::forward<Fb>(fb)))
      { return    functor_traits<std::remove_cvref_t<Fb>>::replace2nd(std::forward<A>(a), std::forward<Fb>(fb)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// @copydoc mpc::detail::fmap_op
    inline constexpr partial<detail::fmap_op> fmap{};

    /// @copydoc mpc::detail::replace2nd_op
    inline constexpr partial<detail::replace2nd_op> replace2nd{};
  } // namespace cpo

  /// Methods deducible from other methods of @link mpc::functor functor @endlink.
  namespace functors {
    /**
     * @copydoc mpc::detail::replace2nd_op
     * @details
     * ```
     * replace2nd = fmap . constant
     * ```
     */
    inline constexpr auto replace2nd = compose(mpc::fmap, constant);
  } // namespace functors

  // Grobal methods

  inline namespace cpo {
    /**
     * @brief replace1st :: Functor f => f a -> b -> f b
     * @details ($>) in Haskell
     * ```
     * replace1st = flip replace2nd
     * ```
     */
    inline constexpr auto replace1st = flip % mpc::replace2nd;
  } // namespace cpo
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/functor.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/identity.hpp
/// @file identity.hpp

namespace mpc {
  inline namespace cpo {
    /**
     * @brief %Identity mapping.
     */
    inline constexpr partial<std::identity> identity{};
  } // namespace cpo
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/identity.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/type_traits.hpp
/// @file type_traits.hpp

/// Implementation details are here
namespace mpc::detail {
  // make_reversed_index_sequence
  // https://stackoverflow.com/questions/51408771/c-reversed-integer-sequence-implementation

  /// @cond undocumented
  template <std::size_t... Idx>
  constexpr auto reversed_index_sequence_impl(std::index_sequence<Idx...> const&)
    -> decltype(std::index_sequence<sizeof...(Idx) - 1U - Idx...>{});
  /// @endcond undocumented

  /// make_reversed_index_sequence
  template <std::size_t N>
  using make_reversed_index_sequence = decltype(reversed_index_sequence_impl(std::make_index_sequence<N>{}));

  // is_implicitly_default_constructible
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/type_traits#L3025

  /// @cond undocumented
  template <class T>
  void test_implicitly_default_constructible_impl(T);

  template <class T, class = void>
  struct is_implicitly_default_constructible_impl : std::false_type {};

  template <class T>
  struct is_implicitly_default_constructible_impl<
    T, decltype(test_implicitly_default_constructible_impl<const T&>({}))> : std::true_type {};
  /// @endcond undocumented

  /// %is_implicitly_default_constructible
  template <class T>
  struct is_implicitly_default_constructible
    : _and<std::is_default_constructible<T>, is_implicitly_default_constructible_impl<T>> {};

  /// @ivar is_implicitly_default_constructible
  template <class T>
  inline constexpr bool is_implicitly_default_constructible_v =
    is_implicitly_default_constructible<T>::value;

  // is_explicitly_convertible
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__ranges/counted.h#L39

  /// %is_explicitly_convertible
  template <class, class, class = void>
  struct is_explicitly_convertible : std::false_type {};

  /// @spec is_explicitly_convertible
  template <class From, class To>
  struct is_explicitly_convertible<From, To, std::void_t<decltype(To(std::declval<From>()))>>
    : std::true_type {};

  /// @ivar is_explicitly_convertible
  template <class From, class To>
  inline constexpr bool is_explicitly_convertible_v = is_explicitly_convertible<From, To>::value;

  // is_implicitly_convertible
  // https://github.com/llvm/llvm-project/blob/main/clang/test/Analysis/gtest.cpp#L43

  /// %is_implicitly_convertible
  template <class From, class To>
  struct is_implicitly_convertible {
  private:
    using yes = char[1];
    using no = char[2];
    static std::add_lvalue_reference_t<From> make_from();
    static yes& test(To);
    static no& test(...);

  public:
    static const bool value = sizeof(test(is_implicitly_convertible::make_from())) == sizeof(yes);
  };

  /// @ivar is_implicitly_convertible
  template <class From, class To>
  inline constexpr bool is_implicitly_convertible_v = is_implicitly_convertible<From, To>::value;

  // Type modification traits
  // https://github.com/vreverdy/type-utilities/blob/master/include/type_utilities.hpp

  /// Copies the const qualifier
  template <class From, class To>
  struct copy_const {
    using type = std::conditional_t<std::is_const_v<From>, std::add_const_t<To>, To>;
  };

  /// Clones the const qualifier
  template <class From, class To>
  struct clone_const {
    using type = typename copy_const<From, std::remove_const_t<To>>::type;
  };

  /// Copies the volatile qualifier
  template <class From, class To>
  struct copy_volatile {
    using type = std::conditional_t<std::is_volatile_v<From>, std::add_volatile_t<To>, To>;
  };

  /// Clones the volatile qualifier
  template <class From, class To>
  struct clone_volatile {
    using type = typename copy_volatile<From, std::remove_volatile_t<To>>::type;
  };

  /// Copies cv qualifiers
  template <class From, class To>
  struct copy_cv {
    using type = typename copy_const<From, typename copy_volatile<From, To>::type>::type;
  };

  /// Clones cv qualifiers
  template <class From, class To>
  struct clone_cv {
    using type = typename copy_cv<From, std::remove_cv_t<To>>::type;
  };

  /// Copies the reference qualifier
  // clang-format off
  template <class From, class To>
  struct copy_reference {
    using type = std::conditional_t<
      std::is_rvalue_reference_v<From>,
      std::add_rvalue_reference_t<To>,
      std::conditional_t<
        std::is_lvalue_reference_v<From>,
        std::add_lvalue_reference_t<To>,
        To
      >
    >;
  };
  // clang-format on

  /// Clones the reference qualifier
  template <class From, class To>
  struct clone_reference {
    using type = typename copy_reference<From, std::remove_reference_t<To>>::type;
  };

  /// Copies cv and reference qualifiers
  // clang-format off
  template <class From, class To>
  struct copy_cvref {
    using type = typename copy_reference<
      From,
      typename copy_reference<
        To,
        typename copy_cv<
          std::remove_reference_t<From>,
          std::remove_reference_t<To>
        >::type
      >::type
    >::type;
  };
  // clang-format on

  /// Clones cv and reference qualifiers
  template <class From, class To>
  struct clone_cvref {
    using type = typename copy_cvref<From, typename std::remove_cvref<To>::type>::type;
  };

  /// @alias copy_const
  template <class From, class To>
  using copy_const_t = typename copy_const<From, To>::type;

  /// @alias clone_const
  template <class From, class To>
  using clone_const_t = typename clone_const<From, To>::type;

  /// @alias copy_volatile
  template <class From, class To>
  using copy_volatile_t = typename copy_volatile<From, To>::type;

  /// @alias clone_volatile
  template <class From, class To>
  using clone_volatile_t = typename clone_volatile<From, To>::type;

  /// @alias copy_cv
  template <class From, class To>
  using copy_cv_t = typename copy_cv<From, To>::type;

  /// @alias clone_cv
  template <class From, class To>
  using clone_cv_t = typename clone_cv<From, To>::type;

  /// @alias copy_reference
  template <class From, class To>
  using copy_reference_t = typename copy_reference<From, To>::type;

  /// @alias clone_reference
  template <class From, class To>
  using clone_reference_t = typename clone_reference<From, To>::type;

  /// @alias copy_cvref
  template <class From, class To>
  using copy_cvref_t = typename copy_cvref<From, To>::type;

  /// @alias clone_cvref
  template <class From, class To>
  using clone_cvref_t = typename clone_cvref<From, To>::type;
} // namespace mpc::detail
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/type_traits.hpp

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
    inline constexpr partial<detail::pure_op<F>> pure{};

    /// @copydoc mpc::detail::seq_apply_op
    inline constexpr partial<detail::seq_apply_op> seq_apply{};

    /// @copydoc mpc::detail::liftA2_op
    inline constexpr partial<detail::liftA2_op> liftA2{};

    /// @copydoc mpc::detail::discard2nd_op
    inline constexpr partial<detail::discard2nd_op> discard2nd{};

    /// @copydoc mpc::detail::discard1st_op
    inline constexpr partial<detail::discard1st_op> discard1st{};
  } // namespace cpo

  /// Methods deducible from other methods of @link mpc::applicative applicative @endlink.
  namespace applicatives {
    namespace detail {
      /**
       * @copydoc mpc::detail::fmap_op
       * @details
       * ```
       * fmap f x = (pure f) `seq_apply` x
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
       * liftA2 f x y = f `fmap` x `seq_apply` y
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
       * @details Use this method if you have an optimized replace2nd.
       * ```
       * discard1st x y = identity `replace2nd` x `seq_apply` y
       * ```
       */
      struct discard1st_opt_op {
        template <class Fa, class Fb>
        constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
        noexcept(   mpc::seq_apply(mpc::replace2nd(identity, std::forward<Fa>(fa)), std::forward<Fb>(fb))))
        -> decltype(mpc::seq_apply(mpc::replace2nd(identity, std::forward<Fa>(fa)), std::forward<Fb>(fb)))
        { return    mpc::seq_apply(mpc::replace2nd(identity, std::forward<Fa>(fa)), std::forward<Fb>(fb)); }
      };
    } // namespace detail

    /// @copydoc mpc::applicatives::detail::fmap_op
    inline constexpr partial<detail::fmap_op> fmap{};

    /**
     * @copydoc mpc::detail::seq_apply_op
     * @details
     * ```
     * seq_apply = liftA2 identity
     * ```
     */
    inline constexpr auto seq_apply = mpc::liftA2 % identity;

    /// @copydoc mpc::applicatives::detail::liftA2_op
    inline constexpr partial<detail::liftA2_op> liftA2{};

    /**
     * @copydoc mpc::detail::discard2nd_op
     * @details
     * ```
     * discard2nd = liftA2 constant
     * ```
     */
    inline constexpr auto discard2nd = mpc::liftA2 % constant;

    /**
     * @copydoc mpc::detail::discard1st_op
     * @details
     * ```
     * discard1st = liftA2 (flip constant)
     * ```
     */
    inline constexpr auto discard1st = mpc::liftA2 % (flip % constant);

    /// @copydoc mpc::applicatives::detail::discard1st_opt_op
    inline constexpr partial<detail::discard1st_opt_op> discard1st_opt{};
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
    inline constexpr partial<detail::liftA_op<N>> liftA{};
  } // namespace cpo
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/applicative.hpp

// clang-format off

namespace mpc {
  // alternative
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html#t:Alternative

  /// class Applicative f => Alternative f where
  template <class>
  struct alternative_traits;

  namespace detail {
    /// has_alternative_traits_empty
    template <class F>
    concept has_alternative_traits_empty = requires {
      alternative_traits<std::remove_cvref_t<F>>::empty();
    };

    /// has_alternative_traits_combine
    template <class F>
    concept has_alternative_traits_combine = requires {
      alternative_traits<std::remove_cvref_t<F>>::combine;
    };
  } // namespace detail

  /// alternative_traits_specialized
  template <class F>
  concept alternative_traits_specialized =
    detail::has_alternative_traits_empty<F> and detail::has_alternative_traits_combine<F>;

  /// Requires applicative and empty and combine is valid in @link mpc::alternative_traits alternative_traits @endlink.
  template <class F>
  concept alternative = applicative<F> and alternative_traits_specialized<F>;

  // Methods required for the class definition.

  namespace detail {
    /**
     * @brief empty :: f a
     * @details Use operator* to access the value.
     */
    template <class F>
    struct empty_op {
      constexpr auto operator*() const noexcept(
      noexcept(   alternative_traits<std::remove_cvref_t<F>>::empty()))
      -> decltype(alternative_traits<std::remove_cvref_t<F>>::empty())
      { return    alternative_traits<std::remove_cvref_t<F>>::empty(); }
    };

    /**
     * @brief combine :: f a -> f a -> f a
     * @details (<|>) in Haskell
     */
    struct combine_op {
      template <class Fa, class Fb>
      requires std::same_as<std::remove_cvref_t<Fa>, std::remove_cvref_t<Fb>>
      constexpr auto operator()(Fa&& fa, Fb&& fb) const noexcept(
      noexcept(   alternative_traits<std::remove_cvref_t<Fa>>::combine(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(alternative_traits<std::remove_cvref_t<Fa>>::combine(std::forward<Fa>(fa), std::forward<Fb>(fb)))
      { return    alternative_traits<std::remove_cvref_t<Fa>>::combine(std::forward<Fa>(fa), std::forward<Fb>(fb)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// @copydoc mpc::detail::empty_op
    template <class F>
    inline constexpr detail::empty_op<F> empty{};

    /// @copydoc mpc::detail::combine_op
    inline constexpr partial<detail::combine_op> combine{};
  } // namespace cpo

  // Grobal methods

  namespace operators::alternatives {
    /// @copydoc mpc::detail::combine_op
    template <class Fa, class Fb>
    requires std::same_as<std::remove_cvref_t<Fa>, std::remove_cvref_t<Fb>>
    inline constexpr auto operator||(Fa&& fa, Fb&& fb)
      noexcept(noexcept(mpc::combine(std::forward<Fa>(fa), std::forward<Fb>(fb))))
      -> decltype(      mpc::combine(std::forward<Fa>(fa), std::forward<Fb>(fb))) {
      return            mpc::combine(std::forward<Fa>(fa), std::forward<Fb>(fb));
    }
  } // namespace operators::alternatives
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/alternative.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/holding.hpp
/// @file holding.hpp

namespace mpc {
  template <class>
  struct holding;

  template <class T>
  using holding_t = typename holding<std::remove_cvref_t<T>>::type;

  template <class T, class U>
  struct holding_or : std::type_identity<U> {};

  template <class T, class U>
  requires requires { typename holding<std::remove_cvref_t<T>>::type; }
  struct holding_or<T, U> : holding<std::remove_cvref_t<T>> {};

  template <class T, class U>
  using holding_or_t = typename holding_or<T, U>::type;
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/holding.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/monad.hpp
/// @file monad.hpp

// clang-format off

namespace mpc {
  // monad
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html#t:Monad

  /// class Applicative m => Monad m where
  template <class>
  struct monad_traits;

  /// monad_traits_specialized
  template <class M>
  concept monad_traits_specialized = requires {
    monad_traits<std::remove_cvref_t<M>>::bind;
  };

  /// Requires applicative and bind is valid in @link mpc::monad_traits monad_traits @endlink.
  template <class M>
  concept monad = applicative<M> and monad_traits_specialized<M>;

  // Methods required for the class definition.

  namespace detail {
    /**
     * @brief bind :: forall a b. m a -> (a -> m b) -> m b
     * @details (>>=) in Haskell
     */
    struct bind_op {
      template <class Ma, class Fn>
      constexpr auto operator()(Ma&& ma, Fn&& fn) const noexcept(
      noexcept(   monad_traits<std::remove_cvref_t<Ma>>::bind(std::forward<Ma>(ma), std::forward<Fn>(fn))))
      -> decltype(monad_traits<std::remove_cvref_t<Ma>>::bind(std::forward<Ma>(ma), std::forward<Fn>(fn)))
      { return    monad_traits<std::remove_cvref_t<Ma>>::bind(std::forward<Ma>(ma), std::forward<Fn>(fn)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// @copydoc mpc::detail::bind_op
    inline constexpr partial<detail::bind_op> bind{};
  } // namespace cpo

  /// Methods deducible from other methods of @link mpc::monad monad @endlink.
  namespace monads {
    namespace detail {
      /**
       * @copydoc mpc::detail::fmap_op
       * ```
       * fmap f xs = xs `bind` (returns . f)
       * ```
       */
      struct fmap_op {
        template <class Fn, class Ma>
        constexpr auto operator()(Fn&& fn, Ma&& ma) const noexcept(
        noexcept(   mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn)))))
        -> decltype(mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn))))
        { return    mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn))); }
      };

      /**
       * @copydoc mpc::detail::seq_apply_op
       * ```
       * seq_apply mf xs = mf `bind` (\f -> xs `bind` (returns . f))
       * ```
       */
      struct seq_apply_op {
        struct closure {
          template<class Ma, class Fn>
          constexpr auto operator()(Ma&& ma, Fn&& fn) const noexcept(
          noexcept(   mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn)))))
          -> decltype(mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn))))
          { return    mpc::bind(std::forward<Ma>(ma), compose(mpc::pure<Ma>, std::forward<Fn>(fn))); }
        };

        template<class Mab, class Ma>
        constexpr auto operator()(Mab&& mab, Ma&& ma) const noexcept(
        noexcept(   mpc::bind(std::forward<Mab>(mab), partial(closure{}, std::forward<Ma>(ma)))))
        -> decltype(mpc::bind(std::forward<Mab>(mab), partial(closure{}, std::forward<Ma>(ma))))
        { return    mpc::bind(std::forward<Mab>(mab), partial(closure{}, std::forward<Ma>(ma))); }
      };

      /**
       * @copydoc mpc::detail::discard1st_op
       * ```
       * discard1st m1 m2 = m1 `bind` (constant m2)
       * ```
       */
      struct discard1st_op {
        template<class Ma, class Mb>
        constexpr auto operator()(Ma&& ma, Mb&& mb) const noexcept(
        noexcept(   mpc::bind(std::forward<Ma>(ma), constant % std::forward<Mb>(mb))))
        -> decltype(mpc::bind(std::forward<Ma>(ma), constant % std::forward<Mb>(mb)))
        { return    mpc::bind(std::forward<Ma>(ma), constant % std::forward<Mb>(mb)); }
      };
    } // namespace detail

    /// @copydoc mpc::monads::detail::fmap_op
    inline constexpr partial<detail::fmap_op> fmap{};

    /// @copydoc mpc::monads::detail::seq_apply_op
    inline constexpr partial<detail::seq_apply_op> seq_apply{};

    /// @copydoc mpc::monads::detail::discard1st_op
    inline constexpr partial<detail::discard1st_op> discard1st{};
  } // namespace monads

  // Grobal methods

  namespace detail {
    /**
     * @brief karrow :: Monad m => (a -> m b) -> (b -> m c) -> (a -> m c)
     * @details (>=>) in Haskell
     * ```
     * karrow f g x = f x `bind` g
     * ```
     */
    struct karrow_op {
      template <class Fn, class Gn, class A>
      constexpr auto operator()(Fn&& fn, Gn&& gn, A&& a) const noexcept(
        noexcept(   mpc::bind(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)), std::forward<Gn>(gn))))
        -> decltype(mpc::bind(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)), std::forward<Gn>(gn)))
        { return    mpc::bind(std::invoke(std::forward<Fn>(fn), std::forward<A>(a)), std::forward<Gn>(gn)); }
    };
  } // namespace detail

  inline namespace cpo {
    /**
     * @brief returns :: a -> m a
     * @details
     * ```
     * returns = pure
     * ```
     */
    template <class M>
    requires requires {
      applicative_traits<std::remove_cvref_t<M>>::pure;
    }
    inline constexpr auto returns = mpc::pure<M>;

    /// @copydoc mpc::detail::karrow_op
    inline constexpr partial<detail::karrow_op> karrow{};
  } // namespace cpo
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/monad.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/state.hpp
/// @file state.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/state/class.hpp
/// @file class.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/unit.hpp
/// @file unit.hpp

namespace mpc {
  /// The type of an empty tuple.
  using unit_t = std::tuple<>;

  /// An entity of an empty tuple.
  inline constexpr unit_t unit;
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/unit.hpp

// clang-format off

namespace mpc {
  // monad_state
  // https://hackage.haskell.org/package/mtl-2.2.2/docs/Control-Monad-State-Class.html

  template <class>
  struct monad_state_traits;

  template <class ST>
  concept monad_state = requires {
    monad_state_traits<std::remove_cvref_t<ST>>::state;
    monad_state_traits<std::remove_cvref_t<ST>>::gets();
    monad_state_traits<std::remove_cvref_t<ST>>::put;
  };

  // Methods required for the class definition.

  namespace detail {
    /// state :: (s -> (a, s)) -> m a
    template <class ST>
    struct state_op {
      template <class Fn>
      constexpr auto operator()(Fn&& fn) const noexcept(
      noexcept(   monad_state_traits<std::remove_cvref_t<ST>>::state(std::forward<Fn>(fn))))
      -> decltype(monad_state_traits<std::remove_cvref_t<ST>>::state(std::forward<Fn>(fn)))
      { return    monad_state_traits<std::remove_cvref_t<ST>>::state(std::forward<Fn>(fn)); }
    };

    /**
     * @brief gets :: m s
     * @details Use operator* to access the value.
     */
    template <class ST>
    struct gets_op {
      constexpr auto operator*() const noexcept(
      noexcept(   monad_state_traits<std::remove_cvref_t<ST>>::gets()))
      -> decltype(monad_state_traits<std::remove_cvref_t<ST>>::gets())
      { return    monad_state_traits<std::remove_cvref_t<ST>>::gets(); }
    };

    /// put :: s -> m ()
    template <class ST>
    struct put_op {
      template <class S>
      constexpr auto operator()(S&& s) const noexcept(
      noexcept(   monad_state_traits<std::remove_cvref_t<ST>>::put(std::forward<S>(s))))
      -> decltype(monad_state_traits<std::remove_cvref_t<ST>>::put(std::forward<S>(s)))
      { return    monad_state_traits<std::remove_cvref_t<ST>>::put(std::forward<S>(s)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// state :: (s -> (a, s)) -> m a
    template <class ST>
    inline constexpr partial<detail::state_op<ST>> state{};

    /// gets :: m s
    template <class ST>
    inline constexpr detail::gets_op<ST> gets{};

    /// put :: s -> m ()
    template <class ST>
    inline constexpr partial<detail::put_op<ST>> put{};
  } // namespace cpo

  // Deducibles
  // [ ] state
  // [x] gets
  // [x] put

  namespace states {
    namespace detail {
      /// state :: (s -> (a, s)) -> m a
      /// state f = do
      ///   s <- get
      ///   let ~(a, s') = f s
      ///   put s'
      ///   return a

      /// gets :: m s
      /// gets = state (s -> (s, s))
      template <class ST>
      struct gets_op {
        struct closure {
          template <class T>
          constexpr auto operator()(T&& t) const noexcept(
            noexcept(   std::make_pair(t, t)))
            -> decltype(std::make_pair(t, t)) {
            auto t2 = t;
            return      std::make_pair(std::forward<T>(t), std::move(t2));
          }
        };

        constexpr auto operator()() const noexcept(
          noexcept(   mpc::state<ST>(closure{})))
          -> decltype(mpc::state<ST>(closure{})) {
          return      mpc::state<ST>(closure{});
        }
      };

      /// put :: s -> m ()
      /// put s = state (_ -> ((), s))
      template <class ST>
      struct put_op {
        struct closure {
          template <class T, class A>
          constexpr auto operator()(T&& t, A&&) const noexcept(
            noexcept(   std::make_pair(unit, std::forward<T>(t))))
            -> decltype(std::make_pair(unit, std::forward<T>(t))) {
            return      std::make_pair(unit, std::forward<T>(t));
          }
        };

        template <class T>
        constexpr auto operator()(T&& t) const noexcept(
          noexcept(   mpc::state<ST>(partial(closure{}, std::forward<T>(t)))))
          -> decltype(mpc::state<ST>(partial(closure{}, std::forward<T>(t)))) {
          return      mpc::state<ST>(partial(closure{}, std::forward<T>(t)));
        }
      };
    } // namespace detail

    // /// state :: (s -> (a, s)) -> m a
    // template <class ST>
    // inline constexpr partial<detail::state_op<ST>> state{};

    /// gets :: m s
    template <class ST>
    requires requires {
      monad_state_traits<std::remove_cvref_t<ST>>::state;
    }
    inline constexpr detail::gets_op<ST> gets{};

    /// put :: s -> m ()
    template <class ST>
    requires requires {
      monad_state_traits<std::remove_cvref_t<ST>>::state;
    }
    inline constexpr partial<detail::put_op<ST>> put{};
  } // namespace states

  // Grobal methods
  // [x] modify
  // [ ] modify'
  // [x] getss

  // modify, getss
  namespace detail {
    /// modify :: MonadState s m => (s -> s) -> m ()
    template <class ST>
    struct modify_op {
      struct closure {
        template <class Fn, class T>
        constexpr auto operator()(Fn&& f, T&& t) const noexcept(
          noexcept(   std::make_pair(unit, std::invoke(std::forward<Fn>(f), std::forward<T>(t)))))
          -> decltype(std::make_pair(unit, std::invoke(std::forward<Fn>(f), std::forward<T>(t)))) {
          return      std::make_pair(unit, std::invoke(std::forward<Fn>(f), std::forward<T>(t)));
        }
      };

      template <class Fn>
      constexpr auto operator()(Fn&& f) const noexcept(
        noexcept(   mpc::state<ST>(partial(closure{}, std::forward<Fn>(f)))))
        -> decltype(mpc::state<ST>(partial(closure{}, std::forward<Fn>(f)))) {
        return      mpc::state<ST>(partial(closure{}, std::forward<Fn>(f)));
      }
    };

    /// getss :: MonadState s m => (s -> a) -> m a
    template <class ST>
    struct getss_op {
      struct closure {
        template <class Fn, class T>
        constexpr auto operator()(Fn&& f, T&& t) const noexcept(
          noexcept(   std::make_pair(std::invoke(std::forward<Fn>(f), t), t)))
          -> decltype(std::make_pair(std::invoke(std::forward<Fn>(f), t), t)) {
          auto t2 = t;
          return      std::make_pair(std::invoke(std::forward<Fn>(f), std::forward<T>(t)), std::move(t2));
        }
      };

      template <class Fn>
      constexpr auto operator()(Fn&& f) const noexcept(
        noexcept(   mpc::state<ST>(partial(closure{}, std::forward<Fn>(f)))))
        -> decltype(mpc::state<ST>(partial(closure{}, std::forward<Fn>(f)))) {
        return      mpc::state<ST>(partial(closure{}, std::forward<Fn>(f)));
      }
    };
  } // namespace detail

  inline namespace cpo {
    /// modify :: MonadState s m => (s -> s) -> m ()
    template <class ST>
    inline constexpr partial<detail::modify_op<std::remove_cvref_t<ST>>> modify{};

    /// getss :: MonadState s m => (s -> a) -> m a
    template <class ST>
    inline constexpr partial<detail::getss_op<std::remove_cvref_t<ST>>> getss{};
  } // namespace cpo
}

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/state/class.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/state/state.hpp
/// @file state.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/state/stateT.hpp
/// @file stateT.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/trans/class.hpp
/// @file class.hpp

namespace mpc {
  // monad_trans
  // https://hackage.haskell.org/package/transformers-0.6.0.2/docs/Control-Monad-Trans-Class.html

  /// class (forall m. Monad m => Monad (t m)) => MonadTrans t where
  template <class>
  struct monad_trans_traits;

  template <class TR>
  concept monad_trans = requires {
    monad_trans_traits<std::remove_cvref_t<TR>>::lift;
  };

  // Methods required for the class definition.

  namespace detail {
    /// lift :: (Monad m) => m a -> t m a
    template <class TR>
    struct lift_op {
      template <class M>
      constexpr auto operator()(M&& m) const noexcept(
      noexcept(   monad_trans_traits<std::remove_cvref_t<TR>>::lift(std::forward<M>(m))))
      -> decltype(monad_trans_traits<std::remove_cvref_t<TR>>::lift(std::forward<M>(m)))
      { return    monad_trans_traits<std::remove_cvref_t<TR>>::lift(std::forward<M>(m)); }
    };
  } // namespace detail

  inline namespace cpo {
    /// lift :: (Monad m) => m a -> t m a
    template <class TR>
    inline constexpr partial<detail::lift_op<TR>> lift{};
  } // namespace cpo
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/trans/class.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/functor/identity.hpp
/// @file identity.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude.hpp
/// @file prelude.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/fst.hpp
/// @file fst.hpp

namespace mpc {
  /// @cond undocumented
  namespace detail::_fst {
    template <auto>
    void get(auto&) = delete;
    template <auto>
    void get(const auto&) = delete;

    template <std::size_t Idx>
    struct get_op {
      template <class T>
      constexpr auto operator()(T&& t) const             //
        noexcept(noexcept(get<Idx>(std::forward<T>(t)))) //
        -> decltype(get<Idx>(std::forward<T>(t))) {
        return get<Idx>(std::forward<T>(t));
      }
    };
  } // namespace detail::_fst
  /// @endcond undocumented

  inline namespace cpo {
    /**
     * @brief Returns the first element of the given tuple-like object.
     */
    inline constexpr partial<detail::_fst::get_op<0>> fst;

    /**
     * @brief Returns the second element of the given tuple-like object.
     */
    inline constexpr partial<detail::_fst::get_op<1>> snd;
  } // namespace cpo
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/fst.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/infix.hpp
/// @file infix.hpp

// clang-format off

namespace mpc {
  /**
   * @brief Implements the right-associative infix notation.
   * @details
   * ```
   * infixr(a1, op, a2) = op(a1, a2)
   * infixr(a1, op, args...) = op(a1, infixr(args...))
   * ```
   */
  template<class A1, class Op, class A2>
  constexpr auto infixr(A1&& a1, Op&& op, A2&& a2)
  noexcept(noexcept(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2))))
  -> decltype(      std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)))
  { return          std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)); }

  /// @overl infixr
  template<class A1, class Op, class... Args>
  constexpr auto infixr(A1&& a1, Op&& op, Args&&... args)
  noexcept(noexcept(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), infixr(std::forward<Args>(args)...))))
  -> decltype(      std::invoke(std::forward<Op>(op), std::forward<A1>(a1), infixr(std::forward<Args>(args)...)))
  { return          std::invoke(std::forward<Op>(op), std::forward<A1>(a1), infixr(std::forward<Args>(args)...)); }

  /**
   * @brief Implements the left-associative infix notation.
   * @details
   * ```
   * infixl(a1, op, a2) = op(a1, a2)
   * infixl(a1, op, a2, args...) = infixl(op(a1, a2), args...)
   * ```
   */
  template<class A1, class Op, class A2>
  constexpr auto infixl(A1&& a1, Op&& op, A2&& a2)
  noexcept(noexcept(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2))))
  -> decltype(      std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)))
  { return          std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)); }

  /// @overl infixl
  template<class A1, class Op, class A2, class... Args>
  constexpr auto infixl(A1&& a1, Op&& op, A2&& a2, Args&&... args)
  noexcept(noexcept(infixl(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)), std::forward<Args>(args)...)))
  -> decltype(      infixl(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)), std::forward<Args>(args)...))
  { return          infixl(std::invoke(std::forward<Op>(op), std::forward<A1>(a1), std::forward<A2>(a2)), std::forward<Args>(args)...); }
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/infix.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/operations.hpp
/// @file operations.hpp

namespace mpc {
  /// Customization point objects are here.
  inline namespace cpo {
    // Arithmetic operations
    /// Partially applicable std::plus<>.
    inline constexpr partial<std::plus<>> plus;
    /// Partially applicable std::minus<>.
    inline constexpr partial<std::minus<>> minus;
    /// Partially applicable std::multiplies<>.
    inline constexpr partial<std::multiplies<>> multiplies;
    /// Partially applicable std::divides<>.
    inline constexpr partial<std::divides<>> divides;
    /// Partially applicable std::modulus<>.
    inline constexpr partial<std::modulus<>> modulus;
    /// Partially applicable std::negate<>.
    inline constexpr partial<std::negate<>> negate;

    // Comparisons
    /// Partially applicable std::ranges::equal_to.
    inline constexpr partial<std::ranges::equal_to> equal_to;
    /// Partially applicable std::ranges::not_equal_to.
    inline constexpr partial<std::ranges::not_equal_to> not_equal_to;
    /// Partially applicable std::ranges::greater.
    inline constexpr partial<std::ranges::greater> greater;
    /// Partially applicable std::ranges::less.
    inline constexpr partial<std::ranges::less> less;
    /// Partially applicable std::ranges::greater_equal.
    inline constexpr partial<std::ranges::greater_equal> greater_equal;
    /// Partially applicable std::ranges::less_equal.
    inline constexpr partial<std::ranges::less_equal> less_equal;
    // WORKAROUND: LLVM 13.0.0 has not implemented std::compare_three_way
    // /// Partially applicable std::compare_three_way.
    // inline constexpr partial<std::compare_three_way> compare_three_way;

    // Logical operations
    /// Partially applicable std::logical_and<>.
    inline constexpr partial<std::logical_and<>> logical_and;
    /// Partially applicable std::logical_or<>.
    inline constexpr partial<std::logical_or<>> logical_or;
    /// Partially applicable std::logical_not<>.
    inline constexpr partial<std::logical_not<>> logical_not;

    // Bitwise operations
    /// Partially applicable std::bit_and<>.
    inline constexpr partial<std::bit_and<>> bit_and;
    /// Partially applicable std::bit_or<>.
    inline constexpr partial<std::bit_or<>> bit_or;
    /// Partially applicable std::bit_xor<>.
    inline constexpr partial<std::bit_xor<>> bit_xor;
    /// Partially applicable std::bit_not<>.
    inline constexpr partial<std::bit_not<>> bit_not;
  } // namespace cpo
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude/operations.hpp
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/prelude.hpp

namespace mpc {
  // Identity
  // https://hackage.haskell.org/package/base-4.16.0.0/docs/Data-Functor-Identity.html

  /// newtype Identity a = Identity { runIdentity :: a }
  template <copy_constructible_object T>
  struct Identity {
  private:
    copyable_box<T> instance_{};

  public:
    using value_type = T;

    constexpr Identity()                                   //
      noexcept(std::is_nothrow_default_constructible_v<T>) //
      requires std::default_initializable<T>               //
      : instance_(std::in_place) {}

    template <class U = T>
    requires std::constructible_from<T, U&&>
    constexpr explicit Identity(U&& u) //
      noexcept(std::is_nothrow_constructible_v<T, U&&>)
      : instance_(std::in_place, std::forward<U>(u)) {}

    constexpr const T& operator*() const& noexcept {
      return *instance_;
    }
    constexpr const T&& operator*() const&& noexcept {
      return std::move(*instance_);
    }
    constexpr T& operator*() & noexcept {
      return *instance_;
    }
    constexpr T&& operator*() && noexcept {
      return std::move(*instance_);
    }

    constexpr const T* operator->() const noexcept {
      return instance_.operator->();
    }
    constexpr T* operator->() noexcept {
      return instance_.operator->();
    }
  };

  template <class T>
  Identity(T) -> Identity<T>;

  namespace detail {
    template <class>
    struct is_Identity_impl : std::false_type {};

    template <copy_constructible_object T>
    struct is_Identity_impl<Identity<T>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept is_Identity = detail::is_Identity_impl<std::remove_cvref_t<T>>::value;

  namespace detail {
    struct make_Identity_op {
      template <copy_constructible_object U>
      constexpr auto operator()(U&& u) const {
        return Identity<std::decay_t<U>>(std::forward<U>(u));
      }
    };

    struct run_Identity_op {
      template <is_Identity I>
      constexpr auto operator()(I&& x) const noexcept -> decltype(*std::forward<I>(x)) {
        return *std::forward<I>(x);
      }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr partial<detail::make_Identity_op> make_Identity{};

    inline constexpr partial<detail::run_Identity_op> run_Identity{};
  } // namespace cpo

  // clang-format off

  /// instance Monad Identity where
  ///     m >>= k  = k (runIdentity m)
  template <copy_constructible_object T>
  struct monad_traits<Identity<T>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b
    struct bind_op {
      template <is_Identity I, class F>
      constexpr auto operator()(I&& x, F&& f) const
        noexcept(noexcept(std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x))))
        -> decltype(      std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x))) {
        return            std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x));
      }
    };

    static constexpr bind_op bind{};
  };

  template <copy_constructible_object T>
  struct functor_traits<Identity<T>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class F, is_Identity I>
      constexpr auto operator()(F&& f, I&& x) const
        noexcept(noexcept(make_Identity(std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x)))))
        -> decltype(      make_Identity(std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x)))) {
        return            make_Identity(std::invoke(std::forward<F>(f), run_Identity % std::forward<I>(x)));
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd;
  };

  template <copy_constructible_object T>
  struct applicative_traits<Identity<T>> {
    static constexpr auto pure = make_Identity;
    static constexpr auto seq_apply = monads::seq_apply;
    static constexpr auto liftA2 = applicatives::liftA2;
    static constexpr auto discard2nd = applicatives::discard2nd;
    static constexpr auto discard1st = monads::discard1st;
  };
  // clang-format on
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/functor/identity.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/functional/function.hpp
/// @file function.hpp

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
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/functional/function.hpp

// clang-format off

namespace mpc {
  // StateT
  // https://hackage.haskell.org/package/transformers-0.6.0.2/docs/Control-Monad-Trans-State-Lazy.html
  // [x] StateT
  // [x] is_StateT
  // [x] make_StateT
  // [x] run_StateT
  // [x] StateT_state_t
  // [x] StateT_monad_t

  /// newtype StateT s m a = StateT { runStateT :: s -> m (a,s) }
  template <class S, monad M>
  struct StateT : Identity<mpc::function<M(S)>> {
    using Identity<mpc::function<M(S)>>::Identity;
    using state_type = S;
    using monad_type = M;
  };

  // is_StateT
  namespace detail {
    template <class>
    struct is_StateT_impl : std::false_type {};

    template <class S, monad M>
    struct is_StateT_impl<StateT<S, M>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept is_StateT = detail::is_StateT_impl<std::remove_cvref_t<T>>::value;

  template <is_StateT ST>
  using StateT_state_t = typename std::remove_cvref_t<ST>::state_type;

  template <is_StateT ST>
  using StateT_monad_t = typename std::remove_cvref_t<ST>::monad_type;

  template <is_StateT ST>
  using eval_StateT_t = decltype(mpc::fmap(fst, std::declval<StateT_monad_t<ST>>()));

  template <class S, monad M>
  struct holding<StateT<S, M>> : holding<std::remove_cvref_t<eval_StateT_t<StateT<S, M>>>> {};

  // make_StateT, run_StateT
  namespace detail {
    template <class S>
    struct make_StateT_op {
      using state_type = std::decay_t<S>;

      template <class Fn>
      requires std::invocable<Fn&, state_type> and monad<std::invoke_result_t<Fn&, state_type>>
      constexpr auto operator()(Fn&& f) const {
        using M = std::invoke_result_t<Fn&, state_type>;
        return StateT<state_type, M>(std::forward<Fn>(f));
      }
    };

    struct run_StateT_op {
      template <is_StateT ST>
      constexpr auto operator()(ST&& x) const noexcept -> decltype(*std::forward<ST>(x)) {
        return *std::forward<ST>(x);
      }
    };
  } // namespace detail

  inline namespace cpo {
    template <class S>
    inline constexpr partial<detail::make_StateT_op<S>> make_StateT{};

    inline constexpr partial<detail::run_StateT_op> run_StateT{};
  } // namespace cpo

  // instances:
  // [x] functor
  // [x] monad
  // [x] applicative
  // [x] alternative
  // [x] monad_trans

  /// instance (Monad m) => Monad (StateT s m) where
  ///     m >>= k  = StateT $ s -> do
  ///         ~(a, s') <- runStateT m s
  ///         runStateT (k a) s'
  template <class S, monad M>
  struct monad_traits<StateT<S, M>> {
    /// (>>=)  :: forall a b. m a -> (a -> m b) -> m b
    struct bind_op {
      struct nested_closure {
        template <class Fn, class U>
        constexpr auto operator()(Fn&& f, U&& u) const
        noexcept(noexcept(run_StateT % std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))) % snd(std::forward<U>(u))))
        -> decltype(      run_StateT % std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))) % snd(std::forward<U>(u)))
        { return          run_StateT % std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))) % snd(std::forward<U>(u)); }
      };

      struct closure {
        template <is_StateT ST, class Fn, class T>
        constexpr auto operator()(ST&& x, Fn&& f, T&& t) const noexcept(
          noexcept(
            mpc::bind(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            partial(nested_closure{}, std::forward<Fn>(f)))))
          -> decltype(
            mpc::bind(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            partial(nested_closure{}, std::forward<Fn>(f)))) {
          return
            mpc::bind(
            run_StateT % std::forward<ST>(x) % std::forward<T>(t),
            partial(nested_closure{}, std::forward<Fn>(f)));
        }
      };

      template <is_StateT ST, class Fn>
      constexpr auto operator()(ST&& x, Fn&& f) const noexcept(
          noexcept(   make_StateT<S>(partial(closure{}, std::forward<ST>(x), std::forward<Fn>(f)))))
          -> decltype(make_StateT<S>(partial(closure{}, std::forward<ST>(x), std::forward<Fn>(f)))) {
        return        make_StateT<S>(partial(closure{}, std::forward<ST>(x), std::forward<Fn>(f)));
      }
    };

    static constexpr bind_op bind{};
  };

  /// instance (Functor m) => Functor (StateT s m) where
  ///     fmap f m = StateT $ s ->
  ///         fmap (~(a, s') -> (f a, s')) $ runStateT m s
  template <class S, monad M>
  struct functor_traits<StateT<S, M>> {
    // fmap  :: (a -> b) -> f a -> f b
    struct fmap_op {
      struct nested_closure {
        template <class Fn, class U>
        constexpr auto operator()(Fn&& f, U&& u) const
        noexcept(noexcept(std::make_pair(std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))), snd(std::forward<U>(u)))))
        -> decltype(      std::make_pair(std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))), snd(std::forward<U>(u))))
        { return          std::make_pair(std::invoke(std::forward<Fn>(f), fst(std::forward<U>(u))), snd(std::forward<U>(u))); }
      };

      struct closure {
        template <class Fn, is_StateT ST, class T>
        constexpr auto operator()(Fn&& f, ST&& x, T&& t) const noexcept(
          noexcept(
            mpc::fmap(
            partial(nested_closure{}, std::forward<Fn>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
          -> decltype(
            mpc::fmap(
            partial(nested_closure{}, std::forward<Fn>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
          return
            mpc::fmap(
            partial(nested_closure{}, std::forward<Fn>(f)),
            run_StateT % std::forward<ST>(x) % std::forward<T>(t));
        }
      };

      template <class Fn, is_StateT ST>
      constexpr auto operator()(Fn&& f, ST&& x) const noexcept(
          noexcept(   make_StateT<S>(partial(closure{}, std::forward<Fn>(f), std::forward<ST>(x)))))
          -> decltype(make_StateT<S>(partial(closure{}, std::forward<Fn>(f), std::forward<ST>(x)))) {
        return        make_StateT<S>(partial(closure{}, std::forward<Fn>(f), std::forward<ST>(x)));
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd;
  };

  /// instance (Functor m, Monad m) => Applicative (StateT s m) where
  ///     pure a = StateT $ s -> return (a, s)
  ///     StateT mf <*> StateT mx = StateT $ s -> do
  ///         ~(f, s') <- mf s
  ///         ~(x, s'') <- mx s'
  ///         return (f x, s'')
  ///     m *> k = m >>= (_ -> k)
  template <class S, monad M>
  struct applicative_traits<StateT<S, M>> {
    /// pure   :: a -> f a
    struct pure_op {
      struct closure {
        template <class A, class T>
        constexpr auto operator()(A&& a, T&& t) const noexcept(
          noexcept(   returns<M>(std::make_pair(std::forward<A>(a), std::forward<T>(t)))))
          -> decltype(returns<M>(std::make_pair(std::forward<A>(a), std::forward<T>(t)))) {
          return      returns<M>(std::make_pair(std::forward<A>(a), std::forward<T>(t)));
        }
      };

      template <class A>
      constexpr auto operator()(A&& a) const noexcept(
        noexcept(   make_StateT<S>(partial(closure{}, std::forward<A>(a)))))
        -> decltype(make_StateT<S>(partial(closure{}, std::forward<A>(a)))) {
        return      make_StateT<S>(partial(closure{}, std::forward<A>(a)));
      }
    };

    static constexpr pure_op pure{};
    static constexpr auto seq_apply = monads::seq_apply;
    static constexpr auto liftA2 = applicatives::liftA2;
    static constexpr auto discard2nd = applicatives::discard2nd;
    static constexpr auto discard1st = monads::discard1st;
  };

  /// instance (Functor m, MonadPlus m) => Alternative (StateT s m) where
  ///     empty = StateT $ (_ -> mzero)
  ///     StateT m <|> StateT n = StateT $ s -> m s `mplus` n s
  namespace detail {
    template <class ST>
    struct StateT_alternative_traits_empty {};

    template <is_StateT ST>
    requires has_alternative_traits_empty<StateT_monad_t<ST>>
    struct StateT_alternative_traits_empty<ST> {
      struct empty_op {
        struct closure {
          template <class T>
          constexpr auto operator()() const noexcept(
            noexcept(   *mpc::empty<StateT_monad_t<ST>>))
            -> decltype(*mpc::empty<StateT_monad_t<ST>>) {
            return      *mpc::empty<StateT_monad_t<ST>>;
          }
        };

        constexpr auto operator()() const noexcept(
          noexcept(   make_StateT<StateT_state_t<ST>>(closure{})))
          -> decltype(make_StateT<StateT_state_t<ST>>(closure{})) {
          return      make_StateT<StateT_state_t<ST>>(closure{});
        }
      };

      static constexpr empty_op empty{};
    };

    template <class ST>
    struct StateT_alternative_traits_combine {};

    template <is_StateT ST>
    requires has_alternative_traits_combine<StateT_monad_t<ST>>
    struct StateT_alternative_traits_combine<ST> {
      struct combine_op {
        struct closure {
          template <is_StateT ST1, is_StateT ST2, class T>
          constexpr auto operator()(ST1&& x, ST2&& y, T&& t) const
            noexcept(noexcept(mpc::combine(run_StateT % std::forward<ST1>(x) % t, run_StateT % std::forward<ST2>(y) % t)))
            -> decltype(      mpc::combine(run_StateT % std::forward<ST1>(x) % t, run_StateT % std::forward<ST2>(y) % t)) {
            auto t2 = t;
            return            mpc::combine(run_StateT % std::forward<ST1>(x) % std::forward<T>(t), run_StateT % std::forward<ST2>(y) % std::move(t2));
          }
        };

        template <is_StateT ST1, is_StateT ST2>
        constexpr auto operator()(ST1&& x, ST2&& y) const
          noexcept(noexcept(make_StateT<StateT_state_t<ST>>(partial(closure{}, std::forward<ST1>(x), std::forward<ST2>(y)))))
          -> decltype(      make_StateT<StateT_state_t<ST>>(partial(closure{}, std::forward<ST1>(x), std::forward<ST2>(y)))) {
          return            make_StateT<StateT_state_t<ST>>(partial(closure{}, std::forward<ST1>(x), std::forward<ST2>(y)));
        }
      };

      static constexpr combine_op combine{};
    };
  } // namespace detail

  template <class S, monad M>
  struct alternative_traits<StateT<S, M>>
    : detail::StateT_alternative_traits_empty<StateT<S, M>>,
      detail::StateT_alternative_traits_combine<StateT<S, M>> {};

  /// instance MonadTrans (StateT s) where
  ///     lift m = StateT $ s -> do
  ///         a <- m
  ///         return (a, s)
  template <class S, monad M>
  struct monad_trans_traits<StateT<S, M>> {
    /// lift :: (Monad m) => m a -> t m a
    struct lift_op {
      struct nested_closure {
        template <class T, class A>
        constexpr auto operator()(T&& t, A&& a) const
          noexcept(noexcept(std::make_pair(std::forward<A>(a), std::forward<T>(t))))
          -> decltype(      std::make_pair(std::forward<A>(a), std::forward<T>(t))) {
          return            std::make_pair(std::forward<A>(a), std::forward<T>(t));
        }
      };

      struct closure {
        template <monad N, class T>
        constexpr auto operator()(N&& n, T&& t) const
          noexcept(noexcept(mpc::fmap(partial(nested_closure{}, std::forward<T>(t)), std::forward<N>(n))))
          -> decltype(      mpc::fmap(partial(nested_closure{}, std::forward<T>(t)), std::forward<N>(n))) {
          return            mpc::fmap(partial(nested_closure{}, std::forward<T>(t)), std::forward<N>(n));
        }
      };

      template <monad N>
      constexpr auto operator()(N&& n) const
        noexcept(noexcept(make_StateT<S>(partial(closure{}, std::forward<N>(n)))))
        -> decltype(      make_StateT<S>(partial(closure{}, std::forward<N>(n)))) {
        return            make_StateT<S>(partial(closure{}, std::forward<N>(n)));
      }
    };

    static constexpr lift_op lift{};
  };

  /// instance MonadState (StateT s) where
  ///     state f = StateT (return . f)
  template <class S, monad M>
  struct monad_state_traits<StateT<S, M>> {
    struct state_op {
      template <copy_constructible_object Fn>
      constexpr auto operator()(Fn&& f) const noexcept(
        noexcept(   make_StateT<S>(compose(mpc::returns<M>, std::forward<Fn>(f)))))
        -> decltype(make_StateT<S>(compose(mpc::returns<M>, std::forward<Fn>(f)))) {
        return      make_StateT<S>(compose(mpc::returns<M>, std::forward<Fn>(f)));
      }
    };

    static constexpr state_op state{};
    static constexpr auto gets = states::gets<StateT<S, M>>;
    static constexpr auto put = states::put<StateT<S, M>>;
  };

  // Grobal methods:
  // [x] eval_StateT
  // [x] exec_StateT
  // [x] map_StateT
  // [x] with_StateT

  // eval_StateT, exec_StateT, map_StateT, with_StateT
  namespace detail {
    struct eval_StateT_op {
      template <is_StateT ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   mpc::fmap(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(mpc::fmap(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
        return      mpc::fmap(fst, run_StateT % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct exec_StateT_op {
      template <is_StateT ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   mpc::fmap(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(mpc::fmap(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t))) {
        return      mpc::fmap(snd, run_StateT % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct map_StateT_op {
      template <class Fn, is_StateT ST>
      constexpr auto operator()(Fn&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn>(f), run_StateT % std::forward<ST>(x)))))
        -> decltype(make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn>(f), run_StateT % std::forward<ST>(x)))) {
        return      make_StateT<StateT_state_t<ST>>(compose(std::forward<Fn>(f), run_StateT % std::forward<ST>(x)));
      }
    };

    struct with_StateT_op {
      template <class Fn, is_StateT ST>
      constexpr auto operator()(Fn&& f, ST&& x) const noexcept(
        noexcept(   make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn>(f)))))
        -> decltype(make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn>(f)))) {
        return      make_StateT<StateT_state_t<ST>>(compose(run_StateT % std::forward<ST>(x), std::forward<Fn>(f)));
      }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr partial<detail::eval_StateT_op> eval_StateT{};

    inline constexpr partial<detail::exec_StateT_op> exec_StateT{};

    inline constexpr partial<detail::map_StateT_op> map_StateT{};

    inline constexpr partial<detail::with_StateT_op> with_StateT{};
  } // namespace cpo
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/state/stateT.hpp

// clang-format off

namespace mpc {
  // State
  // [x] State
  // [x] is_State
  // [x] State_state_t
  // [x] State_monad_t
  // [x] make_State
  // [x] run_State

  /// type State s = StateT s Identity
  template <class S, is_Identity M>
  using State = StateT<S, M>;

  // is_State
  namespace detail {
    template <class>
    struct is_State_impl : std::false_type {};

    template <class S, is_Identity M>
    struct is_State_impl<StateT<S, M>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept is_State = detail::is_State_impl<std::remove_cvref_t<T>>::value;

  template <is_State ST>
  using State_state_t = StateT_state_t<ST>;

  template<is_State ST>
  using State_monad_t = StateT_monad_t<ST>;

  // make_State, run_State
  namespace detail {
    template <class S>
    struct make_State_op {
      using state_type = std::decay_t<S>;

      template <class Fn>
      requires std::invocable<Fn&, state_type> and is_Identity<std::invoke_result_t<Fn&, state_type>>
      constexpr auto operator()(Fn&& f) const {
        using M = std::invoke_result_t<Fn&, state_type>;
        return State<state_type, M>(std::forward<Fn>(f));
      }
    };

    struct run_State_op {
      template <is_State ST>
      constexpr auto operator()(ST&& x) const noexcept
        -> decltype(compose(run_Identity, run_StateT % std::forward<ST>(x))) {
        return compose(run_Identity, run_StateT % std::forward<ST>(x));
      }
    };
  } // namespace detail

  inline namespace cpo {
    template <class S>
    inline constexpr partial<detail::make_State_op<S>> make_State{};

    inline constexpr partial<detail::run_State_op> run_State{};
  } // namespace cpo

  // Grobal methods:
  // [x] eval_State
  // [x] exec_State
  // [x] map_State
  // [x] with_State

  // eval_State, exec_State, map_State, with_State
  namespace detail {
    // clang-format off
    struct eval_State_op {
      template <is_State ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   fst(run_State % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(fst(run_State % std::forward<ST>(x) % std::forward<T>(t))) {
        return      fst(run_State % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct exec_State_op {
      template <is_State ST, class T>
      constexpr auto operator()(ST&& x, T&& t) const noexcept(
        noexcept(   snd(run_State % std::forward<ST>(x) % std::forward<T>(t))))
        -> decltype(snd(run_State % std::forward<ST>(x) % std::forward<T>(t))) {
        return      snd(run_State % std::forward<ST>(x) % std::forward<T>(t));
      }
    };

    struct map_State_op {
      template <class Fn2, is_State ST>
      constexpr auto operator()(Fn2&& f, ST&& x) const noexcept(
        noexcept(   map_StateT % compose(make_Identity, compose(std::forward<Fn2>(f), run_Identity)) % std::forward<ST>(x)))
        -> decltype(map_StateT % compose(make_Identity, compose(std::forward<Fn2>(f), run_Identity)) % std::forward<ST>(x)) {
        return      map_StateT % compose(make_Identity, compose(std::forward<Fn2>(f), run_Identity)) % std::forward<ST>(x);
      }
    };
    // clang-format on
  } // namespace detail

  inline namespace cpo {
    inline constexpr partial<detail::eval_State_op> eval_State{};

    inline constexpr partial<detail::exec_State_op> exec_State{};

    inline constexpr partial<detail::map_State_op> map_State{};

    inline constexpr auto with_State = with_StateT;
  } // namespace cpo
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/state/state.hpp
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/state.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/trans.hpp
/// @file trans.hpp
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control/trans.hpp
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/control.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data.hpp
/// @file data.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/char.hpp
/// @file char.hpp

namespace mpc {
  namespace detail {
    // See also:
    // https://en.cppreference.com/w/cpp/string/byte/isalnum

    struct isalnum_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isalnum(c))) {
        return std::isalnum(c);
      }
    };

    struct isalpha_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isalpha(c))) {
        return std::isalpha(c);
      }
    };

    struct islower_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::islower(c))) {
        return std::islower(c);
      }
    };

    struct isupper_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isupper(c))) {
        return std::isupper(c);
      }
    };

    struct isdigit_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isdigit(c))) {
        return std::isdigit(c);
      }
    };

    struct isxdigit_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isxdigit(c))) {
        return std::isxdigit(c);
      }
    };

    struct iscntrl_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::iscntrl(c))) {
        return std::iscntrl(c);
      }
    };

    struct isgraph_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isgraph(c))) {
        return std::isgraph(c);
      }
    };

    struct isspace_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isspace(c))) {
        return std::isspace(c);
      }
    };

    struct isblank_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isblank(c))) {
        return std::isblank(c);
      }
    };

    struct isprint_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::isprint(c))) {
        return std::isprint(c);
      }
    };

    struct ispunct_op {
      bool operator()(unsigned char c) const noexcept(noexcept(std::ispunct(c))) {
        return std::ispunct(c);
      }
    };
  } // namespace detail

  inline namespace cpo {
    /// Partially applicable std::isalnum.
    inline constexpr partial<detail::isalnum_op> isalnum{};
    /// Partially applicable std::isalpha.
    inline constexpr partial<detail::isalpha_op> isalpha{};
    /// Partially applicable std::islower.
    inline constexpr partial<detail::islower_op> islower{};
    /// Partially applicable std::isupper.
    inline constexpr partial<detail::isupper_op> isupper{};
    /// Partially applicable std::isdigit.
    inline constexpr partial<detail::isdigit_op> isdigit{};
    /// Partially applicable std::isxdigit.
    inline constexpr partial<detail::isxdigit_op> isxdigit{};
    /// Partially applicable std::iscntrl.
    inline constexpr partial<detail::iscntrl_op> iscntrl{};
    /// Partially applicable std::isgraph.
    inline constexpr partial<detail::isgraph_op> isgraph{};
    /// Partially applicable std::isspace.
    inline constexpr partial<detail::isspace_op> isspace{};
    /// Partially applicable std::isblank.
    inline constexpr partial<detail::isblank_op> isblank{};
    /// Partially applicable std::isprint.
    inline constexpr partial<detail::isprint_op> isprint{};
    /// Partially applicable std::ispunct.
    inline constexpr partial<detail::ispunct_op> ispunct{};
  } // namespace cpo

  /// Surrounds a string with delimiter.
  template <class charT>
  inline std::basic_string<charT> quoted(charT s, charT delim = charT('\'')) {
    return {delim, s, delim};
  }

  /// @overl quoted
  template <class charT>
  inline auto quoted(const charT* s, charT delim = charT('"')) {
    return std::basic_string<charT>{delim} + s + delim;
  }

  /// @overl quoted
  template <class charT, class traits>
  inline auto quoted(std::basic_string_view<charT, traits> s, charT delim = charT('"')) {
    return delim + std::basic_string<charT, traits>{s} + delim;
  }

  /// @overl quoted
  template <class charT, class traits, class Allocator>
  inline auto quoted(const std::basic_string<charT, traits, Allocator>& s,
                     charT delim = charT('"')) {
    return delim + s + delim;
  }

  /// @overl quoted
  template <class charT, class traits, class Allocator>
  inline auto quoted(std::basic_string<charT, traits, Allocator>&& s, charT delim = charT('"')) {
    return delim + std::move(s) + delim;
  }
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/char.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/either.hpp
/// @file either.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/alternative_value_t.hpp
/// @file alternative_value_t.hpp

namespace mpc {
  /// alternative_value_t
  template <std::size_t Idx, class Variant>
  requires requires {
    typename std::variant_alternative_t<Idx, std::remove_cvref_t<Variant>>::value_type;
  }
  using alternative_value_t =
    typename std::variant_alternative_t<Idx, std::remove_cvref_t<Variant>>::value_type;
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/alternative_value_t.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/single.hpp
/// @file single.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/tuple_like.hpp
/// @file tuple_like.hpp

namespace mpc {
  /// @cond undocumented
  namespace detail::_tuple_like {
    template <auto>
    void get(auto&) = delete;
    template <auto>
    void get(const auto&) = delete;

    template <class T, class = std::make_index_sequence<std::tuple_size_v<T>>, class = void>
    struct has_tuple_element : std::false_type {};

    template <class T, std::size_t... Idx>
    struct has_tuple_element<T, std::index_sequence<Idx...>,
                             std::void_t<typename std::tuple_element<Idx, T>::type...>>
      : std::true_type {};

    template <class T, class = std::make_index_sequence<std::tuple_size_v<T>>, class = void>
    struct has_unqualified_get : std::false_type {};

    template <class T, std::size_t... Idx>
    struct has_unqualified_get<T, std::index_sequence<Idx...>,
                               std::void_t<decltype(get<Idx>(std::declval<T>()))...>>
      : std::true_type {};
  } // namespace detail::_tuple_like
  /// @endcond undocumented

  /// %is_tuple_like
  template <class T, class = void>
  struct is_tuple_like : std::false_type {};

  /// @spec is_tuple_like
  template <class T>
  struct is_tuple_like<T, std::void_t<decltype(std::tuple_size<T>::value)>>
    : _and<detail::_tuple_like::has_tuple_element<T>, detail::_tuple_like::has_unqualified_get<T>> {
  };

  /// @ivar is_tuple_like
  template <class T>
  inline constexpr bool is_tuple_like_v = is_tuple_like<T>::value;

  /// Requires std::tuple_size, std::tuple_element and unqualified get is valid.
  template <class T>
  concept tuple_like = is_tuple_like_v<std::remove_cvref_t<T>>;
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/tuple_like.hpp

// clang-format off

namespace mpc {
  /**
   * @brief A class that holds a single value.
   * @details To prepare multiple classes with different types but the same
   * properties, the tag type can be assigned to the second type parameter.
   */
  template <class T, class Tag = void>
  struct single {
  private:
    T instance_{};

  public:
    // methods:
    // [x] value_type
    // [x] ctors
    // [x] assignment ops
    // [x] operator<=>
    // [x] swap
    // [x] operator*
    // [x] operator->

    using value_type = T;

    // ctors

    // (1)
    explicit(not detail::is_implicitly_default_constructible_v<T>)
    constexpr single()
      requires std::default_initializable<T> =default;

    // (2)
    single(const single&) = default;

    // (3)
    single(single&&) = default;

    // (4)
    explicit(not std::is_convertible_v<const T&, T>)
    constexpr single(const T& rhs)
      requires std::copy_constructible<T>
      : instance_(rhs) {}

    // (5)
    template <class U = T>
    requires std::constructible_from<T, U&&>
    explicit(not std::is_convertible_v<U, T>)
    constexpr single(U&& rhs)
      : instance_(std::forward<U>(rhs)) {}

    // (6)
    template <tuple_like Tuple>
    requires (
      not std::constructible_from<T, Tuple&&> and
      std::constructible_from<T, detail::copy_reference_t<Tuple, std::tuple_element_t<0, Tuple>>&&>
    )
    explicit(not std::is_convertible_v<std::tuple_element_t<0, Tuple>, T>)
    constexpr single(Tuple&& rhs)
      : instance_(get<0>(std::forward<Tuple>(rhs))) {}

    // (7)
    template <class... Args>
    requires std::constructible_from<T, Args&&...>
    constexpr single(std::in_place_t, Args&&... args)
      : instance_(std::forward<Args>(args)...) {}

    // (8)
    template <class U, class... Args>
    requires std::constructible_from<T, std::initializer_list<U>, Args&&...>
    constexpr single(std::in_place_t, std::initializer_list<U> il, Args&&... args)
      : instance_(il, std::forward<Args>(args)...) {}

    // assignment ops.

    // (1)
    constexpr single& operator=(const single& rhs)
      requires std::is_copy_assignable_v<T> {
      instance_ = *rhs;
      return *this;
    }

    // (2)
    constexpr single& operator=(single&& rhs)
      noexcept(std::is_nothrow_move_assignable_v<T>)
      requires std::is_move_assignable_v<T> {
      instance_ = *std::move(rhs);
      return *this;
    }

    // (3)
    template <std::convertible_to<T> U = T>
    single& operator=(U&& rhs) {
      instance_ = std::forward<U>(rhs);
      return *this;
    }

    // (4)
    template <tuple_like Tuple>
    requires (
      not std::convertible_to<Tuple, T> and
      std::convertible_to<std::tuple_element_t<0, Tuple>, T>
    )
    constexpr single& operator=(Tuple&& rhs) {
      instance_ = get<0>(std::forward<Tuple>(rhs));
      return *this;
    }

    // operator <=>

    auto operator<=>(const single&) const
      requires std::equality_comparable<T> = default;

    // swap

    void swap(single& other)
      noexcept(std::is_nothrow_swappable_v<T>)
      requires std::swappable<T> {
      using std::swap;
      swap(instance_, other.instance_);
    }

    // operator*

    constexpr decltype(auto) operator*() & {
      return instance_;
    }

    constexpr decltype(auto) operator*() const& {
      return instance_;
    }

    constexpr decltype(auto) operator*() && {
      return std::move(instance_);
    }

    constexpr decltype(auto) operator*() const&& {
      return std::move(instance_);
    }

    // operator->

    constexpr auto operator->() {
      return std::addressof(instance_);
    }

    constexpr auto operator->() const {
      return std::addressof(instance_);
    }
  }; // struct single

  // grobal methods:
  // [x] deduction guides
  // [x] swap
  // [x] get

  /// @dguide @ref single
  template <class T, class Tag = void>
  single(T) -> single<T>;

  /// swap for @ref single
  template <std::swappable T, class Tag>
  void swap(single<T, Tag>& lhs, single<T, Tag>& rhs)
    noexcept(std::is_nothrow_swappable_v<T>) {
    lhs.swap(rhs);
  }

  /// get for @ref single
  template <std::size_t Idx, class T, class Tag>
  requires (Idx < 1)
  constexpr decltype(auto) get(single<T, Tag>& s) {
    return *s;
  }

  /// @overl get
  template <std::size_t Idx, class T, class Tag>
  requires (Idx < 1)
  constexpr decltype(auto) get(const single<T, Tag>& s) {
    return *s;
  }

  /// @overl get
  template <std::size_t Idx, class T, class Tag>
  requires (Idx < 1)
  constexpr decltype(auto) get(single<T, Tag>&& s) {
    return *std::move(s);
  }

  /// @overl get
  template <std::size_t Idx, class T, class Tag>
  requires (Idx < 1)
  constexpr decltype(auto) get(const single<T, Tag>&& s) {
    return *std::move(s);
  }
} // namespace mpc

// tuple-like methods:
// [x] std::tuple_size
// [x] std::tuple_element

/// @spec std::tuple_size for mpc::single
template <class T, class Tag>
struct std::tuple_size<mpc::single<T, Tag>> : mpc::index_constant<1> {};

/// @spec std::tuple_element for mpc::single
template <std::size_t Idx, class T, class Tag>
requires (Idx < 1)
struct std::tuple_element<Idx, mpc::single<T, Tag>> {
  using type = T;
};

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/single.hpp

// clang-format off

namespace mpc {
  // either
  // https://hackage.haskell.org/package/base-4.15.0.0/docs/src/Data-Either.html#Either

  template <class T>
  using left_t = single<T, std::false_type>;

  template <class T>
  using right_t = single<T, std::true_type>;

  template <class T>
  constexpr left_t<std::unwrap_ref_decay_t<T>> make_left(T&& t) {
    return std::forward<T>(t);
  }

  template <class T>
  constexpr right_t<std::unwrap_ref_decay_t<T>> make_right(T&& t) {
    return std::forward<T>(t);
  }

  /// data Either a b = Left a | Right b
  template <class T, class U>
  using either = std::variant<left_t<T>, right_t<U>>;

  namespace detail {
    template <class>
    struct is_either_impl : std::false_type {};

    template <class T, class U>
    struct is_either_impl<either<T, U>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept is_either = detail::is_either_impl<std::remove_cvref_t<T>>::value;

  template <class T, class U>
  struct holding<either<T, U>> : std::type_identity<U> {};

  /// instance Functor (Either a) where
  ///   fmap _ (Left x) = Left x
  ///   fmap f (Right y) = Right (f y)
  template <class T1, class T2>
  struct functor_traits<either<T1, T2>> {
    /// fmap :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class F, is_either E>
      constexpr auto operator()(F&& f, E&& e) const
        -> either<alternative_value_t<0, E>,
                  std::unwrap_ref_decay_t<std::invoke_result_t<F, alternative_value_t<1, E>>>> {
        if (e.index() == 0) {
          return std::get<0>(std::forward<E>(e));
        } else {
          return make_right(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<E>(e))));
        }
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr auto replace2nd = functors::replace2nd;
  };

  /// instance Monad (Either e) where
  ///   Left  l >>= _ = Left l
  ///   Right r >>= k = k r
  template <class T1, class T2>
  struct monad_traits<either<T1, T2>> {
    /// bind :: forall a b. m a -> (a -> m b) -> m b
    struct bind_op {
      template <is_either E, class F>
      constexpr auto operator()(E&& e, F&& f) const //
        -> decltype(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<E>(e)))) {
        if (e.index() == 0) {
          return std::get<0>(std::forward<E>(e));
        } else {
          return std::invoke(std::forward<F>(f), *std::get<1>(std::forward<E>(e)));
        }
      }
    };

    static constexpr bind_op bind{};
  };

  /// instance Applicative (Either e) where
  ///   pure          = Right
  ///   Left  e <*> _ = Left e
  ///   Right f <*> r = fmap f r
  template <class T1, class T2>
  struct applicative_traits<either<T1, T2>> {
    /// pure :: a -> f a
    struct pure_op {
      template <class U>
      constexpr auto operator()(U&& u) const        //
        -> either<T1, std::unwrap_ref_decay_t<U>> { //
        return make_right(std::forward<U>(u));
      }
    };

    /// seq_apply :: f (a -> b) -> f a -> f b
    struct seq_apply_op {
      template <is_either E1, is_either E2>
      constexpr auto operator()(E1&& e1, E2&& e2) const
        -> either<
          std::common_type_t<
            alternative_value_t<0, E1>,
            alternative_value_t<0, decltype(mpc::fmap(*std::get<1>(std::forward<E1>(e1)), std::forward<E2>(e2)))>
          >,
          alternative_value_t<1, decltype(mpc::fmap(*std::get<1>(std::forward<E1>(e1)), std::forward<E2>(e2)))>
        > {
        if (e1.index() == 0) {
          return std::get<0>(std::forward<E1>(e1));
        } else {
          return mpc::fmap(*std::get<1>(std::forward<E1>(e1)), std::forward<E2>(e2));
        }
      }
    };

    static constexpr pure_op pure{};
    static constexpr seq_apply_op seq_apply{};
    static constexpr auto liftA2 = applicatives::liftA2;
    static constexpr auto discard2nd = applicatives::discard2nd;
    static constexpr auto discard1st = monads::discard1st;
  };
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/either.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/functor.hpp
/// @file functor.hpp
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/functor.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/list.hpp
/// @file list.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/maybe.hpp
/// @file maybe.hpp

// clang-format off

namespace mpc {
  // maybe
  // https://hackage.haskell.org/package/base-4.15.0.0/docs/src/GHC-Maybe.html#Maybe

  struct nothing_t {
    inline constexpr auto operator<=>(const nothing_t&) const = default;
  }; // struct nothing_t

  inline constexpr nothing_t nothing;

  template <class T>
  using just_t = single<T, nothing_t>;

  template <class T>
  constexpr just_t<std::unwrap_ref_decay_t<T>> make_just(T&& t) {
    return std::forward<T>(t);
  }

  /// data Maybe a = Nothing | Just a
  template <class T>
  using maybe = std::variant<nothing_t, just_t<T>>;

  namespace detail {
    template <class>
    struct is_maybe_impl : std::false_type {};

    template <class T>
    struct is_maybe_impl<maybe<T>> : std::true_type {};
  } // namespace detail

  template <class T>
  concept is_maybe = detail::is_maybe_impl<std::remove_cvref_t<T>>::value;

  /// instance Functor Maybe where
  ///     fmap _ Nothing  = Nothing
  ///     fmap f (Just a) = Just (f a)
  ///     _ <$ Nothing = Nothing
  ///     a <$ Just _  = Just a
  template <class T1>
  struct functor_traits<maybe<T1>> {
    /// fmap :: (a -> b) -> f a -> f b
    struct fmap_op {
      template <class F, is_maybe M>
      constexpr auto operator()(F&& f, M&& x) const //
        -> maybe<std::unwrap_ref_decay_t<decltype(
          std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x))))>> {
        if (x.index() == 0) {
          return nothing;
        } else {
          return make_just(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x))));
        }
      }
    };

    /// replace2nd :: a -> f b -> f a
    struct replace2nd_op {
      template <class U, is_maybe M>
      constexpr auto operator()(U&& u, M&& m) const //
        -> maybe<std::unwrap_ref_decay_t<U&&>> {
        if (m.index() == 0) {
          return nothing;
        } else {
          return make_just(std::forward<U>(u));
        }
      }
    };

    static constexpr fmap_op fmap{};
    static constexpr replace2nd_op replace2nd{};
  };

  /// instance Monad Maybe where
  ///     (Just x) >>= k = k x
  ///     Nothing  >>= _ = Nothing
  template <class T1>
  struct monad_traits<maybe<T1>> {
    /// bind :: forall a b. m a -> (a -> m b) -> m b
    struct bind_op {
      template <is_maybe M, class F>
      constexpr auto operator()(M&& x, F&& f) const //
        -> decltype(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x)))) {
        if (x.index() == 0) {
          return nothing;
        } else {
          return std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M>(x)));
        }
      }
    };

    static constexpr bind_op bind{};
  };

  /// instance Applicative Maybe where
  ///     pure = Just
  ///     Nothing <*> _       = Nothing
  ///     Just f  <*> m       = fmap f m
  ///     liftA2 f (Just x) (Just y) = Just (f x y)
  ///     liftA2 _ _ _ = Nothing
  template <class T1>
  struct applicative_traits<maybe<T1>> {
    /// pure :: a -> f a
    struct pure_op {
      template <class U>
      constexpr auto operator()(U&& u) const   //
        -> maybe<std::unwrap_ref_decay_t<U>> { //
        return make_just(std::forward<U>(u));
      }
    };

    /// seq_apply :: f (a -> b) -> f a -> f b
    struct seq_apply_op {
      template <is_maybe M1, is_maybe M2>
      constexpr auto operator()(M1&& f, M2&& x) const //
        -> decltype(mpc::fmap(*std::get<1>(std::forward<M1>(f)), std::forward<M2>(x))) {
        if (f.index() == 0) {
          return nothing;
        } else {
          return mpc::fmap(*std::get<1>(std::forward<M1>(f)), std::forward<M2>(x));
        }
      }
    };

    /// liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    struct liftA2_op {
      template <class F, is_maybe M1, is_maybe M2>
      constexpr auto operator()(F&& f, M1&& m1, M2&& m2) const //
        -> maybe<std::unwrap_ref_decay_t<decltype(
          std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M1>(m1)),
                                          *std::get<1>(std::forward<M2>(m2))))>> {
        if (m1.index() == 1 and m2.index() == 1) {
          return make_just(std::invoke(std::forward<F>(f), *std::get<1>(std::forward<M1>(m1)),
                                                           *std::get<1>(std::forward<M2>(m2))));
        } else {
          return nothing;
        }
      }
    };

    static constexpr pure_op pure{};
    static constexpr seq_apply_op seq_apply{};
    static constexpr liftA2_op liftA2{};
    static constexpr auto discard2nd = applicatives::discard2nd;
    static constexpr auto discard1st = applicatives::discard1st_opt;
  };

  /// instance Alternative Maybe where
  ///     empty = Nothing
  ///     Nothing <|> r = r
  ///     l       <|> _ = l
  template <class T1>
  struct alternative_traits<maybe<T1>> {
    struct empty_op {
      constexpr auto operator()() const -> maybe<T1> {
        return nothing;
      }
    };

    struct combine_op {
      template <is_maybe M1, is_maybe M2>
      requires std::same_as<std::remove_cvref_t<M1>, std::remove_cvref_t<M2>>
      constexpr auto operator()(M1&& m1, M2&& m2) const //
        -> std::remove_cvref_t<M1> {
        return (m1.index() == 1 ? m1 : m2);
      }
    };

    static constexpr empty_op empty{};
    static constexpr combine_op combine{};
  };
} // namespace mpc

// clang-format on
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/maybe.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/ranges.hpp
/// @file ranges.hpp

#if defined(__cpp_lib_ranges)


namespace mpc::ranges::inline cpo {
  // clang-format off
  using std::ranges::begin,
    std::ranges::end,
    std::ranges::range,
    std::ranges::input_range,
    std::ranges::iterator_t,std::ranges::sentinel_t,
    std::ranges::range_difference_t,
    std::ranges::range_value_t,
    std::ranges::range_reference_t,
    std::ranges::range_rvalue_reference_t;
  // clang-format on
}

#else

namespace mpc::ranges::detail {
  using std::begin, std::end;

  struct begin_fn {
    template <class C>
    constexpr auto operator()(C&& c) const noexcept(noexcept(begin(c))) -> decltype(begin(c)) {
      return begin(c);
    }
  };

  struct end_fn {
    template <class C>
    constexpr auto operator()(C&& c) const noexcept(noexcept(end(c))) -> decltype(end(c)) {
      return end(c);
    }
  };
} // namespace mpc::ranges::detail

namespace mpc::ranges::inline cpo {
  inline constexpr detail::begin_fn begin{};
  inline constexpr detail::end_fn end{};
} // namespace mpc::ranges::inline cpo

namespace mpc::ranges {
  template <class T>
  concept range = requires(T& t) {
    ranges::begin(t); // equality-preserving for forward iterators
    ranges::end(t);
  };

  template <class T>
  using iterator_t = decltype(ranges::begin(std::declval<T&>()));

  template <ranges::range R>
  using sentinel_t = decltype(ranges::end(std::declval<R&>()));

  template <ranges::range R>
  using range_difference_t = std::iter_difference_t<ranges::iterator_t<R>>;

  template <ranges::range R>
  using range_value_t = std::iter_value_t<ranges::iterator_t<R>>;

  template <ranges::range R>
  using range_reference_t = std::iter_reference_t<ranges::iterator_t<R>>;

  template <ranges::range R>
  using range_rvalue_reference_t = std::iter_rvalue_reference_t<ranges::iterator_t<R>>;

  template <class T>
  concept input_range = ranges::range<T> and std::input_iterator<ranges::iterator_t<T>>;
} // namespace mpc::ranges

#endif
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/ranges.hpp

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
    };

    struct uncons_op {
      template <is_list L>
      constexpr auto operator()(L&& l) const
        -> maybe<std::pair<mpc::ranges::range_value_t<L>, std::remove_cvref_t<L>>> {
        if (l.empty()) {
          return nothing;
        } else {
          auto tail = std::forward<L>(l);
          auto head = std::move(tail.front());
          tail.pop_front();
          return make_just(std::make_pair(std::move(head), std::move(tail)));
        }
      }
    };

    struct append_op {
      template <is_list L1, is_list L2>
      constexpr auto operator()(L1&& l1, L2&& l2) const {
        auto head = std::forward<L1>(l1);
        auto tail = std::forward<L2>(l2);
        head.splice(head.end(), std::move(tail));
        return head;
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

    inline constexpr partial<detail::uncons_op> uncons;

    inline constexpr partial<detail::append_op> append;

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
        return foldr(mpc::liftA2 % cons, mpc::returns<T> % std::list<U>{}, l);
      }
    };
  } // namespace detail

  inline namespace cpo {
    inline constexpr partial<detail::sequence_op> sequence;
  } // namespace cpo
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/list.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/tagged_union.hpp
/// @file tagged_union.hpp

namespace mpc {
  template <class T, std::size_t Idx>
  using nth_element_t = single<T, index_constant<Idx>>;

  namespace detail {
    template <class, class...>
    struct tagged_union_impl;

    template <std::size_t... Idx, class... Args>
    struct tagged_union_impl<std::index_sequence<Idx...>, Args...> {
      using type = std::variant<nth_element_t<Args, Idx>...>;
    };
  } // namespace detail

  template <class... Args>
  using tagged_union =
    typename detail::tagged_union_impl<std::index_sequence_for<Args...>, Args...>::type;

  template <std::size_t Idx, class T>
  constexpr nth_element_t<T, Idx> make_nth_element(T&& t) {
    return std::forward<T>(t);
  }
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data/tagged_union.hpp
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/data.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/functional.hpp
/// @file functional.hpp
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/functional.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/parser.hpp
/// @file parser.hpp

namespace mpc {
#define MPC_FORWARD(x) std::forward<decltype(x)>(x)

  template <class T>
  T decay(T); // no definition

  template <class T, class U>
  concept similar_to = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

  using String = std::list<char>;
  using ParseError = std::string;
  using ParseResult = either<ParseError, std::pair<char, String>>;
  using Parser = StateT<String, ParseResult>;
} // namespace mpc

template <class T>
struct mpc::alternative_traits<mpc::either<mpc::ParseError, std::pair<T, mpc::String>>> {
  static constexpr auto combine = //
    // TODO: m1, m2 を is_Parser<T> で制約
    [](auto&& m1, auto&& m2) -> either<mpc::ParseError, std::pair<T, mpc::String>> {
    if (m1.index() == 0) {
      if (m2.index() == 0) {
        return make_left(*fst(MPC_FORWARD(m1)) + " and " + *fst(MPC_FORWARD(m2)));
      } else {
        return MPC_FORWARD(m2);
      }
    } else {
      return MPC_FORWARD(m1);
    }
  };
};

namespace mpc {
  // Parsers
  // https://hackage.haskell.org/package/parsec-3.1.15.1/docs/Text-Parsec.html#g:1

  /// パーサーと文字列を受け取り、パースする。パースに成功した場合、パース結果を表示する。失敗した場合、エラーメッセージを表示する。
  inline constexpr auto parse_test = //
    // TODO: parser を is_Parser<T> で制約
    partial([](auto&& parser, std::string_view sv) {
      auto result = eval_StateT % MPC_FORWARD(parser) % String(sv.begin(), sv.end());
      if (result.index() == 0) {
        // fail
        std::cout << mpc::quoted(sv) << ' ' << *fst(result) << std::endl;
      } else {
        // succeed
        for (const auto& c : *snd(result))
          std::cout << c;
        std::cout << std::endl;
      }
    });

  // Combinators
  // https://hackage.haskell.org/package/parsec-3.1.15.1/docs/Text-Parsec.html#g:2

  /// エラーメッセージを受け取り、必ず失敗するパーサーを返す。
  inline constexpr auto left = //
    compose
    % lift<Parser> % partial([](similar_to<ParseError> auto&& str) -> eval_StateT_t<Parser> {
        return make_left(MPC_FORWARD(str));
      });

  /// パーサーを受け取り、パーサーを返す。このパーサーはパースに失敗しても直ちにエラーとならない。
  inline constexpr auto try1 = //
    // TODO: parser を is_Parser<T> で制約
    partial([](auto&& parser) {
      return make_StateT<String>(partial(
        [](auto&& parser2, similar_to<String> auto&& str) {
          return run_StateT % MPC_FORWARD(parser2) % MPC_FORWARD(str);
        },
        MPC_FORWARD(parser)));
    });

  /// パーサーを受け取り、パーサーを返す。このパーサーは受け取ったパーサーを1回以上可能な限り適用した結果をリストで返す。
  /// some :: f a -> f [a]
  /// some v = (:) <$> v <*> many v
  inline constexpr auto many1 = //
    // TODO: p, sep を is_Parser<T> で制約
    partial([](auto&& p) {
      return make_StateT<String>(partial(
        [](auto&& p2, similar_to<String> auto&& str)
          -> decltype(run_StateT % (sequence % std::list{p2}) % str) {
          const auto parse = run_StateT % MPC_FORWARD(p2);
          auto result = parse % MPC_FORWARD(str);
          if (result.index() == 0)
            return make_left(*fst(std::move(result)));
          auto [value, state] = *snd(std::move(result));
          std::list<holding_t<decltype(p2)>> ret{std::move(value)};

          for (result = parse % state; result.index() != 0; result = parse % state) {
            std::tie(value, state) = *snd(std::move(result));
            ret.push_back(std::move(value));
          }
          return make_right(std::make_pair(std::move(ret), std::move(state)));
        },
        MPC_FORWARD(p)));
    });

  /// パーサーを受け取り、パーサーを返す。このパーサーは受け取ったパーサーを可能な限り適用した結果をリストで返す。
  /// many :: f a -> f [a]
  /// many v = some v <|> pure []
  inline constexpr auto many = //
    // TODO: p を is_Parser<T> で制約
    partial([](auto&& p) {
      using namespace operators::alternatives;
      return many1(MPC_FORWARD(p)) or pure<decltype(p)>(std::list<holding_t<decltype(p)>>{});
    });

  /// @brief between open p close = open *> p <* close
  inline constexpr auto between = //
    // TODO: open, p, close を is_Parser<T> で制約
    partial([](auto&& open, auto&& p, auto&& close) {
      return discard2nd(discard1st(MPC_FORWARD(open), MPC_FORWARD(p)), MPC_FORWARD(close));
    });

  /// @brief sepBy1 p sep = liftA2 (:) p (many (sep *> p))
  inline constexpr auto sep_by1 = //
    // TODO: p, sep を is_Parser<T> で制約
    partial([](auto&& p, auto&& sep) {
      auto p2 = p;
      return liftA2(cons, MPC_FORWARD(p), many % discard1st(MPC_FORWARD(sep), std::move(p2)));
    });

  /// @brief sepBy p sep = sepBy1 p sep <|> pure []
  inline constexpr auto sep_by = //
    // TODO: p, sep を is_Parser<T> で制約
    partial([](auto&& p, auto&& sep) {
      using namespace operators::alternatives;
      return sep_by1(MPC_FORWARD(p), MPC_FORWARD(sep))
             or pure<decltype(p)>(std::list<holding_t<decltype(p)>>{});
    });

  /// chainl1 p op = do {
  ///   x <- p;
  ///   rest x
  /// } where rest x = do {
  ///   f <- op;
  ///   y <- p;
  ///   rest (f x y)
  /// } <|> return x
  inline constexpr auto chainl1 = //
    // TODO: p, op を is_Parser<T> で制約
    partial([](auto&& p, auto&& op) {
      return make_StateT<String>(partial(
        [](auto&& p2, auto&& op2, similar_to<String> auto&& str)
          -> decltype(run_StateT % p2 % str) {
          const auto parse = run_StateT % MPC_FORWARD(p2);
          const auto parse_op = run_StateT % MPC_FORWARD(op2);

          auto result = parse % MPC_FORWARD(str);
          if (result.index() == 0)
            return make_left(*fst(std::move(result)));
          auto [value, state] = *snd(std::move(result));

          for (;;) {
            auto result_op = parse_op % state;
            if (result_op.index() == 0)
              break;
            auto [fn, state_op] = *snd(std::move(result_op));
            auto result2 = parse % std::move(state_op);
            if (result2.index() == 0)
              break;
            auto [value2, state2] = *snd(std::move(result2));
            value = fn(std::move(value), std::move(value2));
            state = std::move(state2);
          }
          return make_right(std::make_pair(std::move(value), std::move(state)));
        },
        MPC_FORWARD(p), MPC_FORWARD(op)));
    });
} // namespace mpc

namespace mpc {
  // Character Parsing
  // https://hackage.haskell.org/package/parsec-3.1.15.1/docs/Text-Parsec-Char.html

  /// 述語を受け取り、パーサーを返す。このパーサーは、文字列の先頭が述語を満たす場合にそれを返す。
  inline constexpr auto satisfy = //
    partial([](std::predicate<char> auto&& pred) {
      return make_StateT<String>(partial(
        [](auto&& pred2, similar_to<String> auto&& str) -> ParseResult {
          using namespace std::string_literals;

          if (auto m = uncons(MPC_FORWARD(str)); m.index() == 0) {
            return make_left("unexpected end of input"s);
          } else if (auto [x, xs] = *snd(std::move(m)); not std::invoke(MPC_FORWARD(pred2), x)) {
            return make_left("unexpected "s + mpc::quoted(std::move(x)));
          } else {
            return make_right(std::make_pair(std::move(x), std::move(xs)));
          }
        },
        MPC_FORWARD(pred)));
    });

  /// 文字を受け取り、パーサーを返す。このパーサーは、文字列の先頭が渡した文字に一致する場合にそれを返す。
  inline constexpr auto char1 = //
    partial([](char c) {
      using namespace operators::alternatives;
      using namespace std::string_literals;
      auto c2 = c;
      return satisfy % (equal_to % std::move(c))
             or left % ("expecting char "s + mpc::quoted(std::move(c2)));
    });

  /// 文字列を受け取り、パーサーを返す。このパーサーは、文字列の先頭が渡した文字列から始まる場合にその文字列を返す。
  inline constexpr auto string = //
    partial([](std::string_view sv) {
      return sequence
             % fmap(partial([](char c) { return char1 % std::move(c); }),
                    std::list(sv.begin(), sv.end()));
    });

  namespace detail {
    using namespace operators::alternatives;
    using namespace std::string_literals;

    inline const auto alnum = satisfy % mpc::isalnum or left % "expecting alnum"s;
    inline const auto alpha = satisfy % mpc::isalpha or left % "expecting alpha"s;
    inline const auto lower = satisfy % mpc::islower or left % "expecting lower"s;
    inline const auto upper = satisfy % mpc::isupper or left % "expecting upper"s;
    inline const auto digit = satisfy % mpc::isdigit or left % "expecting digit"s;
    inline const auto xdigit = satisfy % mpc::isxdigit or left % "expecting xdigit"s;
    inline const auto cntrl = satisfy % mpc::iscntrl or left % "expecting cntrl"s;
    inline const auto graph = satisfy % mpc::isgraph or left % "expecting graph"s;
    inline const auto space = satisfy % mpc::isspace or left % "expecting space"s;
    inline const auto blank = satisfy % mpc::isblank or left % "expecting blank"s;
    inline const auto print = satisfy % mpc::isprint or left % "expecting print"s;
    inline const auto punct = satisfy % mpc::ispunct or left % "expecting punct"s;
    inline const auto any_char = satisfy % (constant % true);
  } // namespace detail

  using detail::alnum, detail::alpha, detail::lower, detail::upper, detail::digit, detail::xdigit,
    detail::cntrl, detail::graph, detail::space, detail::blank, detail::print, detail::punct,
    detail::any_char;
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/parser.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility.hpp
/// @file utility.hpp
//BEGIN_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/overloaded.hpp
/// @file overloaded.hpp

namespace mpc {
  // overloaded
  // https://en.cppreference.com/w/cpp/utility/variant/visit

  /// %overloaded
  template <typename... Ts>
  struct overloaded : Ts... {
    using Ts::operator()...;
  };

  /// @dguide @ref overloaded
  template <typename... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;
} // namespace mpc
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility/overloaded.hpp
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/utility.hpp
//END_FILE_INCLUDE: /home/runner/work/monadic-parser-combinator/monadic-parser-combinator/include/mpc/mpc.hpp

#endif // RICH_SINGLE_HEADER_INCLUDED
