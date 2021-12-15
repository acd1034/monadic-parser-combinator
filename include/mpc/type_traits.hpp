/// @file type_traits.hpp
#pragma once
#include <mpc/stdfundamental.hpp>

namespace mpc::detail {
  // make_reversed_index_sequence
  // https://stackoverflow.com/questions/51408771/c-reversed-integer-sequence-implementation

  template <std::size_t... Idx>
  constexpr auto reversed_index_sequence_impl(std::index_sequence<Idx...> const&)
    -> decltype(std::index_sequence<sizeof...(Idx) - 1U - Idx...>{});

  /// make_reversed_index_sequence
  template <std::size_t N>
  using make_reversed_index_sequence = decltype(reversed_index_sequence_impl(std::make_index_sequence<N>{}));

  // is_implicitly_default_constructible
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/type_traits#L3025

  template <class T>
  void test_implicitly_default_constructible_impl(T);

  template <class T, class = void>
  struct is_implicitly_default_constructible_impl : std::false_type {};

  template <class T>
  struct is_implicitly_default_constructible_impl<
    T, decltype(test_implicitly_default_constructible_impl<const T&>({}))> : std::true_type {};

  /// %is_implicitly_default_constructible
  template <class T>
  struct is_implicitly_default_constructible
    : _and<std::is_default_constructible<T>, is_implicitly_default_constructible_impl<T>> {};

  template <class T>
  inline constexpr bool is_implicitly_default_constructible_v =
    is_implicitly_default_constructible<T>::value;

  // is_explicitly_convertible
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/__ranges/counted.h#L39

  /// %is_explicitly_convertible
  template <class, class, class = void>
  struct is_explicitly_convertible : std::false_type {};

  template <class From, class To>
  struct is_explicitly_convertible<From, To, std::void_t<decltype(To(std::declval<From>()))>>
    : std::true_type {};

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

  // Alias templates
  template <class From, class To>
  using copy_const_t = typename copy_const<From, To>::type;
  template <class From, class To>
  using clone_const_t = typename clone_const<From, To>::type;
  template <class From, class To>
  using copy_volatile_t = typename copy_volatile<From, To>::type;
  template <class From, class To>
  using clone_volatile_t = typename clone_volatile<From, To>::type;
  template <class From, class To>
  using copy_cv_t = typename copy_cv<From, To>::type;
  template <class From, class To>
  using clone_cv_t = typename clone_cv<From, To>::type;
  template <class From, class To>
  using copy_reference_t = typename copy_reference<From, To>::type;
  template <class From, class To>
  using clone_reference_t = typename clone_reference<From, To>::type;
  template <class From, class To>
  using copy_cvref_t = typename copy_cvref<From, To>::type;
  template <class From, class To>
  using clone_cvref_t = typename clone_cvref<From, To>::type;
} // namespace mpc::detail
