#include <iomanip>
#include <iostream>
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

int main() {}
