## Example for State Monads

A sample code written in Haskell is here:

```hs
-- Reference: https://hackage.haskell.org/package/mtl-2.3/docs/Control-Monad-State-Lazy.html#g:4
tick :: State Int Int
tick = do
  n <- get
  modify (+ 1)
  return n

threeTicks :: State Int Int
threeTicks = do
  n1 <- tick
  n2 <- tick
  n3 <- tick
  return $ n1 + n2 + n3

main :: IO ()
main = print $ runState threeTicks 5 -- (18,8)
```

Using this library, you can rewrite this code in C++ as follows:

```cpp
#include <iostream>
#include <mpc/control.hpp>

int main() {
  using ST = mpc::State<int, mpc::Identity<int>>;
  constexpr auto fn1 = mpc::partial([](int n, auto&&) { return n; });
  constexpr auto fn2 = mpc::partial([](int n1, int n2, int n3) {
    return n1 + n2 + n3;
  });

  const auto tick = mpc::liftA2(
    fn1,
    *mpc::gets<ST>,
    mpc::modify<ST> % (mpc::plus % 1)
  );

  const auto threeTicks = mpc::liftA<3>(
    fn2,
    tick,
    tick,
    tick
  );

  const auto [a, s] = mpc::run_State % threeTicks % 5;
  std::cout << a << "," << s << std::endl; // 18,8
}
```
