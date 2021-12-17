```hs
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html#t:Monad
class Applicative m => Monad m where
-- Methods required for the class definition.
-- Requires applicative and bind is valid in @link mpc::monad_traits monad_traits @endlink.
bind :: forall a b. m a -> (a -> m b) -> m b
-- (>>=) in Haskell

-- Methods that can be deduced from other methods of @link mpc::monad monad @endlink.
fmap :: (a -> b) -> f a -> f b
fmap f xs = bind xs (return . f) -- TODO: この式で再実装

seq_apply :: f (a -> b) -> f a -> f b
seq_apply mf xs = bind mf (\f -> bind xs (return . f))) -- TODO: この式で再実装

discard1st :: f a -> f b -> f b
discard1st m1 m2 = bind m1 (const m2) -- TODO: この式で再実装

-- Methods
returns :: a -> m a
returns = pure

karrow :: Monad m => (a -> m b) -> (b -> m c) -> (a -> m c)
-- (>=>) in Haskell
karrow f g = (\x -> f x >>= g)
```
