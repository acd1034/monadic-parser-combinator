```hs
--------------------------------------------------------------------------------
-- List instances
-- https://hackage.haskell.org/package/base-4.17.0.0/docs/GHC-Base.html
-- https://hackage.haskell.org/package/base-4.17.0.0/docs/src/GHC.Base.html#line-1166
--------------------------------------------------------------------------------
instance Functor [] where
    fmap = map
    where
      map :: (a -> b) -> [a] -> [b]
      map _ []     = []
      map f (x:xs) = f x : map f xs

instance Applicative [] where
    pure x    = [x]
    fs <*> xs = [f x | f <- fs, x <- xs]
    liftA2 f xs ys = [f x y | x <- xs, y <- ys]
    xs *> ys  = [y | _ <- xs, y <- ys]

instance Monad []  where
    xs >>= f             = [y | x <- xs, y <- f x]
    (>>) = (*>)

instance Alternative [] where
    empty = []
    (<|>) = (++)

--------------------------------------------------------------------------------
-- class Semigroup
-- https://hackage.haskell.org/package/base-4.17.0.0/docs/Prelude.html#t:Semigroup
-- https://hackage.haskell.org/package/base-4.17.0.0/docs/src/GHC.Base.html#Semigroup
--------------------------------------------------------------------------------
class Semigroup a where
    (<>) :: a -> a -> a

-- https://hackage.haskell.org/package/base-4.17.0.0/docs/src/GHC.Base.html#line-300
instance Semigroup [a] where
    (<>) :: a -> a -> a
    (<>) = (++)

--------------------------------------------------------------------------------
-- class Foldable
-- https://hackage.haskell.org/package/base-4.17.0.0/docs/Data-Foldable.html
--------------------------------------------------------------------------------
class Foldable t where
    -- MINIMAL foldMap | foldr

    foldMap :: Monoid m => (a -> m) -> t a -> m
    foldMap f = foldr (mappend . f) mempty

    foldr :: (a -> b -> b) -> b -> t a -> b
    foldr f z t = appEndo (foldMap (Endo #. f) t) z

-- where...
-- Endo: endomorphism (自己準同型) a -> a
newtype Endo a = Endo { appEndo :: a -> a }
instance Monoid (Endo a) where
    Endo f <> Endo g = Endo (f . g)
    mempty = Endo id

instance Foldable [] where
    foldr :: (a -> b -> b) -> b -> [a] -> b
    foldr _ z []     =  z
    foldr f z (x:xs) =  f x (foldr f z xs)

--------------------------------------------------------------------------------
-- class Traversable
-- https://hackage.haskell.org/package/base-4.17.0.0/docs/Data-Traversable.html
--------------------------------------------------------------------------------
class (Functor t, Foldable t) => Traversable t where
    -- MINIMAL traverse | sequenceA
    -- NOTE: sequenceA を定義するよりも traverse を定義した方が効率がよい (要出典)

    traverse :: Applicative f => (a -> f b) -> t a -> f (t b)
    traverse f = sequenceA . fmap f

    sequenceA :: Applicative f => t (f a) -> f (t a)
    sequenceA = traverse id

-- instance Traversable []
-- https://hackage.haskell.org/package/base-4.17.0.0/docs/src/Data.Traversable.html#line-297
instance Traversable [] where
    traverse :: Applicative f => (a -> f b) -> [a] -> f [b]
    traverse g xs = List.foldr cons_g (pure []) xs
    where
      cons_g :: a -> f [b] -> f [b]
      cons_g x ys = liftA2 (:) (g x) ys

-- Another way to instantiate
instance Traversable [] where
    sequenceA :: Applicative f => [f a] -> f [a]
    sequenceA xs = foldr cons (pure []) xs
    where
      cons :: f a -> f [a] -> f [a]
      cons x ys = liftA2 (:) x ys
```
