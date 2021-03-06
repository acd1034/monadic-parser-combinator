#include "maybefundamental.hpp"

template <class T>
struct mpc::monad_traits<maybe<T>> {
  static constexpr auto bind = ::detail::bind_op{};
};

template <class T>
struct mpc::functor_traits<maybe<T>> {
  static constexpr auto fmap = monads::fmap;
  static constexpr auto replace2nd = functors::replace2nd;
};

template <class T>
struct mpc::applicative_traits<maybe<T>> {
  static constexpr auto pure = ::detail::pure_op{};
  static constexpr auto seq_apply = monads::seq_apply;
  static constexpr auto liftA2 = applicatives::liftA2;
  static constexpr auto discard2nd = applicatives::discard2nd;
  static constexpr auto discard1st = monads::discard1st;
};

static_assert(mpc::functor<maybe<int>>);
static_assert(mpc::applicative<maybe<int>>);
static_assert(mpc::monad<maybe<int>>);

int main() {}
