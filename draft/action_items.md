# Action items
## tier 1
[x] GitHub repogitory 作る

## tier 2
[x] `StateT` の (a, s) は `tuple-like` と決めるか? → Yes
[x] `Identity` 修正
  [x] Identity<T> の T の制約
  [x] instance は copyable_box<T>
[x] `StateT` 修正
  [x] struct StateT<S, Fn> : Identity<Fn>
  [x] make_StateT<S>(fn) と書けるようにする
[ ] `type State s a = StateT s Identity a` 実装するか? → Yes
[ ] `fst`, `snd`
[ ] `gets` or `get2`

## tier 3
[ ] `class... Args` に対し、 `std::size_t... Idx` は変 (`Idcs` であるべき?)
[ ] `perfect_forwarded_t` によって `***_t` が不要になった (`***_op` のみでok)
[ ] `perfect_forwarded` → `partial_applicable` ???
[ ] `seq_apply` → `ap` ???
[x] `liftA, liftA3` 不要では??? → `liftA` のみ消した



```cpp
/// @file fname.hpp
#pragma once
#include <functional> // std::invoke
#include <mpc/functional/perfect_forward.hpp>

namespace mpc {

} // namespace mpc
