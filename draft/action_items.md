# Action items
## tier 1
[ ] GitHub repogitory 作る

## tier 2
[ ] `stateT` の (a, s) は `tuple-like` と決めるか?
[ ] `copyable_box`
[ ] `identity` 修正
  [ ] identity<T> の T の制約
  [ ] instance は copyable_box<T>
[ ] `stateT` 修正
  [ ] struct stateT<S, Fn> : identity<Fn>
  [ ] make_stateT<S>(fn) と書けるようにする
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
