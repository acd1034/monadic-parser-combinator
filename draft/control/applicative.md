```hs
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Applicative.html#t:Applicative
class Functor f => Applicative f where
-- Methods required for the class definition.
-- Requires functor and pure, seq_apply, liftA2, discard2nd and discard1st is valid in @link mpc::applicative_traits applicative_traits @endlink.
pure :: a -> f a
seq_apply :: f (a -> b) -> f a -> f b
-- (<*>) in Haskell
liftA2 :: (a -> b -> c) -> f a -> f b -> f c
discard2nd :: f a -> f b -> f a
-- (<*) in Haskell
discard1st :: f a -> f b -> f b
-- (*>) in Haskell

-- Deducible methods
fmap :: (a -> b) -> f a -> f b
fmap f x = seq_apply (pure f) x

seq_apply :: f (a -> b) -> f a -> f b
seq_apply = liftA2 id

liftA2 :: (a -> b -> c) -> f a -> f b -> f c
liftA2 f x y = seq_apply (fmap f x) y

discard2nd :: f a -> f b -> f a
discard2nd = liftA2 const

discard1st :: f a -> f b -> f b
discard1st = liftA2 (flip const)

discard1st_opt :: f a -> f b -> f b
discard1st_opt x y = (id <$ x) <*> y

-- Basic methods
liftA :: Applicative f => (a -> b -> ... -> z) -> f a -> f b -> ... -> f z
liftA f a b ... z = f `fmap` a `seq_apply` b `seq_apply` ... `seq_apply` z
```
