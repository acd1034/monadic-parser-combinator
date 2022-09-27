/// @file stdfundamental.hpp
#include <array>
#include <functional>
#include <string>
#include <utility> // std::exchange
#include <vector>

template <class T>
[[deprecated]] constexpr void type_of() {}

template <class T>
[[deprecated]] constexpr void type_of(T&&) {}
