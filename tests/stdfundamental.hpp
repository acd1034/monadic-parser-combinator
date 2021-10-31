/// @file stdfundamental.hpp
#include <array>
#include <functional>
#include <string>
#include <vector>
using namespace std::literals;

template <class T>
[[deprecated]] constexpr void type_of() {}

template <class T>
[[deprecated]] constexpr void type_of(T&&) {}
