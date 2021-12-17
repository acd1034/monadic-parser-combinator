```hs
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html#t:Functor
class Functor f where
-- Methods required for the class definition.
-- Requires fmap and replace2nd is valid in @link mpc::functor_traits functor_traits @endlink.
fmap :: (a -> b) -> f a -> f b
replace2nd :: a -> f b -> f a
-- (<$) in Haskell

-- Methods that can be deduced from other methods of @link mpc::functor functor @endlink.
replace2nd :: a -> f b -> f a
replace2nd = fmap . const

-- Methods
replace1st :: Functor f => f a -> b -> f b
-- ($>) in Haskell
replace1st = flip replace2nd
```
