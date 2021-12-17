```hs
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html#t:Functor
class Functor f where
-- Class requirements
-- Requires functor_traits<F>::fmap and replace2nd is valid.
fmap :: (a -> b) -> f a -> f b
replace2nd :: a -> f b -> f a
-- (<$) in Haskell

-- Deducible methods
replace2nd :: a -> f b -> f a
replace2nd = fmap . const

-- Methods
replace1st :: Functor f => f a -> b -> f b
-- ($>) in Haskell
replace1st = flip replace2nd
```
