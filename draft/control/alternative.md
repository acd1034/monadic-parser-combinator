```hs
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html#t:Alternative
class Applicative f => Alternative f where
-- Methods required for the class definition.
-- Requires applicative and empty and combine is valid in @link mpc::alternative_traits alternative_traits @endlink.
empty :: f a
combine :: f a -> f a -> f a
-- (<|>) in Haskell
```
