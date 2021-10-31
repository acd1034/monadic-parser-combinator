# Action items
## tier 1
[ ] GitHub repogitory 作る

## tier 2
[x] `stateT` の (a, s) は `tuple-like` と決めるか? → Yes
[x] `identity` 修正
  [x] identity<T> の T の制約
  [x] instance は copyable_box<T>
[x] `stateT` 修正
  [x] struct stateT<S, Fn> : identity<Fn>
  [x] make_stateT<S>(fn) と書けるようにする
[ ] `type State s a = StateT s Identity a` 実装するか?

## tier 3
[ ] `class... Args` に対し、 `std::size_t... Idx` は変 (`Idcs` であるべき?)
[ ] `perfect_forwarded_t` によって `***_t` が不要になった (`***_op` のみでok)
[ ] `perfect_forwarded` → `partial_applicable` ???
[ ] `seq_apply` → `ap` ???
[x] Maybe の Nothing は空のタプル??? → No!
[x] `liftA, liftA3` 不要では??? → `liftA` のみ消した



```cpp
/// @file fname.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {

} // namespace mpc
