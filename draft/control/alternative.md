```hs
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html#t:Alternative
class Applicative f => Alternative f where
-- Class requirements
-- Requires applicative and alternative_traits<F>::empty and combine is valid.
empty :: f a
combine :: f a -> f a -> f a
-- (<|>) in Haskell
```
