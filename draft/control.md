```hs
Functor:
  Minimal Definition:
    fmap
  Fully Optimized Definition:
    fmap, replace2nd
  Methods:
    [x] fmap
    [ ] replace2nd = fmap . const
    --
    fmap :: (a -> b) -> f a -> f b
    replace2nd :: a -> f b -> f a
  If Monad:
    [x] fmap f x = bind x (return . f)
  If Applicative:
    [x] fmap f x = seq_apply (pure f) x
  Grobal Methods:
    replace1st :: f a -> b -> f b
    [x] replace1st = flip replace2nd
    [ ] (<$>) = fmap -- infixl 4 → / or -
    [ ] (<$ ) = replace2nd -- infixl 4
    [ ] ( $>) = replace1st -- infixl 4

Applicative:
  Minimal Definition:
    pure, (seq_apply | liftA2)
  Fully Optimized Definition:
    pure, seq_apply, liftA2, discard2nd, discard1st
  Methods:
    [x] pure
    [x] seq_apply    = liftA2 id
    [x] liftA2 f x y = seq_apply (fmap f x) y
    [x] discard2nd   = liftA2 const
    [x] discard1st   = liftA2 (flip const)
    -- [x] ↑ If Functor instance has an optimized (<$), use `a1 *> a2 = (id <$ a1) <*> a2` instead.
    pure :: a -> f a
    seq_apply :: f (a -> b) -> f a -> f b
    liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    discard2nd :: f a -> f b -> f a
    discard1st :: f a -> f b -> f b
  If Monad:
    [x] pure = return
    [x] seq_apply m1 m2 = bind m1 (f -> bind m2 (x -> return (f x)))
    [x] discard1st m1 m2 = bind m1 (_ -> m2)
  Grobal Methods:
    liftA :: Applicative f => (a -> b) -> f a -> f b
    [x] liftA = fmap
    liftA3 :: Applicative f => (a -> b -> c -> d) -> f a -> f b -> f c -> f d
    [x] liftA3 f a b c = liftA2 f a b <*> c
    [ ] (<*>) = seq_apply -- infixl 4 → * or +
    [ ] (<* ) = discard2nd -- infixl 4
    [ ] ( *>) = discard1st -- infixl 4
    --
    when :: Applicative f => Bool -> f () -> f ()
    [ ] when p s = if p then s else pure ()
    unless :: Applicative f => Bool -> f () -> f ()
    [ ] unless p s =  if p then pure () else s

Alternative:
  Minimal Definition:
    empty, combine
  Fully Optimized Definition:
    empty, combine
  Methods:
    [x] empty
    [x] combine
    --
    empty :: f a
    combine :: f a -> f a -> f a
  Grobal Methods:
    [x] (<|>) = combine -- infixl 3 → ||

Monad:
  Minimal Definition:
    bind
  Fully Optimized Definition:
    bind
  Methods:
    [x] bind
    --
    bind :: forall a b. m a -> (a -> m b) -> m b
  Grobal Methods:
    bind_left :: forall a b. (a -> m b) -> m a -> m b
    [ ] bind_left = flip bind
    [ ] (>>=) = bind -- infixl 1 → &&
    [ ] (=<<) = bind_left -- infixl 1
    karrow :: Monad m => (a -> m b) -> (b -> m c) -> (a -> m c) -- infixr 1
    [x] karrow f g = \x -> f x >>= g
    karrow_left :: Monad m => (b -> m c) -> (a -> m b) -> (a -> m c)
    [ ] karrow_left = flip karrow
    --
    join :: (Monad m) => m (m a) -> m a
    [ ] join x = x >>= id
```

```cpp
/*
@brief
class Functor f where
@details
Minimal complete definition:
  fmap
Semantic requirements:
  Identity
    fmap id == id
  Composition
    fmap (f . g) == fmap f . fmap g
Methods:
  fmap  :: (a -> b) -> f a -> f b
  (<$)  :: a -> f b -> f a (infixl 4)
    default: (<$) = fmap . const

Grobal methods:
  (<$>) :: Functor f => (a -> b) -> f a -> f b (infixl 4)
    (<$>) = fmap
  ($>) :: Functor f => f a -> b -> f b (infixl 4)
    ($>) = flip (<$)
  // ↓ low priority???
  (<&>) :: Functor f => f a -> (a -> b) -> f b (infixl 1)
    (<&>) = flip fmap
*/

/*
@brief
class Functor f => Applicative f where
@details
Minimal complete definition:
  pure, ((<*>) | liftA2)
Semantic requirements:
  Identity
    pure id <*> v = v
  Composition
    pure (.) <*> u <*> v <*> w = u <*> (v <*> w)
  Homomorphism
    pure f <*> pure x = pure (f x)
  Interchange
    u <*> pure y = pure ($ y) <*> u
Methods:
  pure   :: a -> f a
  (<*>)  :: f (a -> b) -> f a -> f b (infixl 4)
    default: (<*>) = liftA2 id
  liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    default: liftA2 f x y = (fmap f x) <*> y
  (*>)   :: f a -> f b -> f b (infixl 4)
    default: u *> v = (id <$ u) <*> v
  (<*)   :: f a -> f b -> f a (infixl 4)
    default: u <* v = liftA2 const u v
Methods for Functor:
  fmap f x = (pure f) <*> x
*/

/*
@brief
class Applicative f => Alternative f where
@details
Minimal complete definition:
  empty, (<|>)
Methods:
  empty :: f a
  (<|>) :: f a -> f a -> f a (infixl 3)
  some  :: f a -> f [a]
    default: some v = (fmap (:) v) <*> many v
  many  :: f a -> f [a]
    default: many v = some v <|> pure []
*/

/*
@brief
class Applicative m => Monad m where
@details
Minimal complete definition:
  return, (>>=)
Semantic requirements:
  Left identity
    return a >>= k = k a
  Right identity
    m >>= return = m
  Associativity
    m >>= (\x -> k x >>= h) = (m >>= k) >>= h
Methods:
  return :: a -> m a
  (>>=)  :: forall a b. m a -> (a -> m b) -> m b (infixl 1)
  (>>)   :: forall a b. m a -> m b -> m b (infixl 1)
    default: u >> v = (id <$ u) <*> v
Methods for Applicative:
  pure      = return
  m1 <*> m2 = m1 >>= (x1 -> m2 >>= (x2 -> return (x1 x2)))
  (*>)      = (>>)
Methods for Functor:
  fmap f xs = xs >>= return . f

Grobal methods:
  (=<<) :: Monad m => (a -> m b) -> m a -> m b (infixr 1)
    f =<< x = x >>= f
  // ↓ low priority???
  (>=>) :: Monad m => (a -> m b) -> (b -> m c) -> a -> m c (infixr 1) -- Kleisli arrow
    f >=> g = \x -> f x >>= g
  (<=<) :: Monad m => (b -> m c) -> (a -> m b) -> (a -> m c)
  (<=<) = flip (>=>)
*/
```
