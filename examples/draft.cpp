#include <iomanip>
#include <iostream>
#include <optional>
#include <mpc/control.hpp>
#include <mpc/data.hpp>
#include <mpc/functional.hpp>

namespace ns {
  template <class T>
  struct traits;

  template <class T>
  concept has_aaa = requires {
    traits<T>::aaa;
  };

  template <has_aaa T>
  inline constexpr auto bbb = traits<T>::aaa;
} // namespace ns

template <>
struct ns::traits<int> {
  static constexpr auto aaa = [](auto x) { return x; };
  static constexpr auto bbb = ns::bbb<int>;
};

template<class S, class M>
using deduceST = mpc::stateT<S, std::function<M(S)>>;

int main() {
  static_assert(mpc::functor<mpc::identity<int>>);
  static_assert(mpc::applicative<mpc::identity<int>>);
  static_assert(mpc::monad<mpc::identity<int>>);

  using ST = deduceST<int, mpc::identity<int>>;
  static_assert(mpc::functor<ST>);
  static_assert(mpc::applicative<ST>);
  static_assert(mpc::monad<ST>);

  // clang-format off
  {
    auto [a, s] = mpc::run_identity(mpc::run_stateT % mpc::get1<ST> % 3);
    std::cout << a << ' ' << s << std::endl;
  }

  {
    using Fn = decltype([](int n) { return n * 2; });
    const auto ti = mpc::fmap<ST>(
      mpc::perfect_forwarded_t<Fn>{},
      mpc::get1<ST>
    );
    auto [a, s] = mpc::run_identity(mpc::run_stateT % ti % 5);
    std::cout << a << ' ' << s << std::endl;
  }

  using FnTick = decltype([](int n, auto&&) { return n; });
  const auto tick = mpc::liftA2<ST>(
    mpc::perfect_forwarded_t<FnTick>{},
    mpc::get1<ST>,
    mpc::put<ST> % 1
  );
  {
    auto [a, s] = mpc::run_identity(mpc::run_stateT % tick % 5);
    std::cout << a << ' ' << s << std::endl;
  }

  {
    using Fn = decltype([](int n1, int n2, auto&&) { return n1 + n2; });
    const auto tock = mpc::liftA3<ST>(
      mpc::perfect_forwarded_t<Fn>{},
      mpc::get1<ST>,
      mpc::get1<ST>,
      mpc::put<ST> % 1
    );
    auto [a, s] = mpc::run_identity(mpc::run_stateT % tock % 5);
    std::cout << a << ' ' << s << std::endl;
  }

  {
    using Fn = decltype([](int n1, int n2) { return n1 + n2; });
    const auto tock = mpc::liftA2<ST>(
      mpc::perfect_forwarded_t<Fn>{},
      tick,
      tick
    );
    auto [a, s] = mpc::run_identity(mpc::run_stateT % tock % 5);
    std::cout << a << ' ' << s << std::endl;
  }

  {
    using Fn = decltype([](int n1, int n2, int n3) { return n1 + n2 + n3; });
    const auto tock = mpc::liftA3<ST>(
      mpc::perfect_forwarded_t<Fn>{},
      tick,
      tick,
      tick
    );
    auto [a, s] = mpc::run_identity(mpc::run_stateT % tock % 5);
    std::cout << a << ' ' << s << std::endl;
  }
}
