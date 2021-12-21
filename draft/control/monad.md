```hs
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Control-Monad.html#t:Monad
class Applicative m => Monad m where
-- Methods required for the class definition.
-- Requires applicative and bind is valid in @link mpc::monad_traits monad_traits @endlink.
bind :: forall a b. m a -> (a -> m b) -> m b
-- (>>=) in Haskell

-- Methods deducible from other methods of @link mpc::monad monad @endlink.
fmap :: (a -> b) -> f a -> f b
fmap f xs = xs `bind` (return . f)

seq_apply :: f (a -> b) -> f a -> f b
seq_apply mf xs = mf `bind` (\f -> xs `bind` (return . f))

discard1st :: f a -> f b -> f b
discard1st m1 m2 = m1 `bind` (constant m2)

-- Methods
returns :: a -> m a
returns = pure

karrow :: Monad m => (a -> m b) -> (b -> m c) -> a -> m c
-- (>=>) in Haskell
karrow f g x = f x `bind` g
```
