#include "maybefundamental.hpp"

/*
template <class T>
struct mpc::monad_traits<maybe<T>> {
  static constexpr auto bind = ::detail::bind_op{};
};

template <class T>
struct mpc::functor_traits<maybe<T>> {
  static constexpr auto fmap = monads::fmap<maybe<T>>;
  static constexpr auto replace2nd = functors::replace2nd<maybe<T>>;
};

template <class T>
struct mpc::applicative_traits<maybe<T>> {
  static constexpr auto pure = ::detail::pure_op{};
  static constexpr auto seq_apply = monads::seq_apply<maybe<T>>;
  static constexpr auto liftA2 = applicatives::liftA2<maybe<T>>;
  //                             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ error!
  // `functor_traits<maybe<T>>::fmap` not found
  static constexpr auto discard2nd = applicatives::discard2nd<maybe<T>>;
  static constexpr auto discard1st = monads::discard1st<maybe<T>>;
};
*/

template <class T>
struct mpc::monad_traits<maybe<T>> {
  static constexpr auto bind = ::detail::bind_op{};
};

template <class T>
struct mpc::functor_traits<maybe<T>> {
  // static constexpr auto fmap = monads::fmap<maybe<T>>;
  static constexpr auto fmap = ::detail::fmap_op{};
  static constexpr auto replace2nd = functors::replace2nd<maybe<T>>;
};

template <class T>
struct mpc::applicative_traits<maybe<T>> {
  static constexpr auto pure = ::detail::pure_op{};
  static constexpr auto seq_apply = monads::seq_apply<maybe<T>>;
  // static constexpr auto seq_apply = ::detail::seq_apply_op{};
  static constexpr auto liftA2 = applicatives::liftA2<maybe<T>>;
  // static constexpr auto liftA2 = ::detail::liftA2_op{};
  static constexpr auto discard2nd = applicatives::discard2nd<maybe<T>>;
  static constexpr auto discard1st = monads::discard1st<maybe<T>>;
};

static_assert(mpc::functor<maybe<int>>);
static_assert(mpc::applicative<maybe<int>>);
static_assert(mpc::monad<maybe<int>>);

int main() {}
