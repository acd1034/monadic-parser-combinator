```hs
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html#t:Monad
class Applicative m => Monad m where
-- Class requirements
-- Requires applicative and monad_traits<M>::bind is valid.
bind :: forall a b. m a -> (a -> m b) -> m b
-- (>>=) in Haskell

-- Deducible methods
fmap :: (a -> b) -> f a -> f b
fmap f xs = bind xs (return . f)

seq_apply :: f (a -> b) -> f a -> f b
seq_apply mf xs = bind mf (\f -> bind xs (return . f)))

discard1st :: f a -> f b -> f b
discard1st m1 m2 = bind m1 (const m2)

-- Methods
returns :: a -> m a
returns = pure

karrow :: Monad m => (a -> m b) -> (b -> m c) -> (a -> m c)
-- (>=>) in Haskell
karrow f g = (\x -> f x >>= g)
```
