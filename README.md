# `mpc`: Haskell-like feature supports in C++
[![Linux build status](https://github.com/acd1034/monadic-parser-combinator/actions/workflows/linux-build.yml/badge.svg)](https://github.com/acd1034/monadic-parser-combinator/actions/workflows/linux-build.yml)
<!-- [![macOS build status](https://github.com/acd1034/monadic-parser-combinator/actions/workflows/macos-build.yml/badge.svg)](https://github.com/acd1034/monadic-parser-combinator/actions/workflows/macos-build.yml) -->
This library aims to implement a monadic parser combinator in C++ (still under development).

Click [here](https://acd1034.github.io/monadic-parser-combinator/index.html) to see the HTML documentation generated by Doxygen.

<!-- ## Noteworthy Features -->

## First example
A sample code written in Haskell is shown below.

```hs
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
using ST = mpc::StateT<int, mpc::Identity<int>>;
constexpr auto fn1 = [](int n, auto&&) { return n; };
constexpr auto fn2 = [](int n1, int n2, int n3) { return n1 + n2 + n3; };

const auto tick = mpc::liftA2(
  mpc::partially_applicable(fn1),
  *mpc::gets<ST>,
  mpc::modify<ST> % (mpc::plus % 1)
);

const auto threeTicks = mpc::liftA<3>(
  mpc::partially_applicable(fn2),
  tick,
  tick,
  tick
);

const auto [a, s] = mpc::run_State % threeTicks % 5;
std::cout << a << "," << s << std::endl; // 18,8
```

## Supported Compilers
This library will work on the following compilers:
- GCC 11.2.0
- Clang 13.0.0
<!-- - Apple clang 12.0.5 -->

## Library Dependencies
This library depends on no external libraries.
