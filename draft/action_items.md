# Action items
## tier 1

## tier 2
[ ] `StateT` を `std::function` を用いて再実装
[ ] `sequence` 修正?
[ ] `some`, `many` in `alternative`
[ ] `list` 周りの整備

## tier 3
[ ] `class... Args` に対し、 `std::size_t... Idx` は変 (`Idcs` であるべき?)
[ ] `perfect_forwarded_t` によって `***_t` が不要になった (`***_op` のみでok)
[ ] `perfect_forwarded` → `partial_applicable` ???
[ ] `seq_apply` → `ap` ???
[x] `liftA, liftA3` 不要では??? → `liftA` のみ消した
[ ] `liftA4, liftA5` ← やっぱ必要だと思う



```cpp
/// @file fname.hpp
#pragma once
#include <mpc/prelude.hpp>

namespace mpc {

} // namespace mpc
