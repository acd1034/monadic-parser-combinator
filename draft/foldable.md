```hs
traverse :: Applicative f => (a -> f b) -> t a -> f (t b)
    {-# INLINE traverse #-}  -- See Note [Inline default methods]
    traverse f = sequenceA . fmap f

sequenceA :: Applicative f => t (f a) -> f (t a)
    {-# INLINE sequenceA #-}  -- See Note [Inline default methods]
    sequenceA = traverse id

instance Traversable [] where
    {-# INLINE traverse #-} -- so that traverse can fuse
    traverse f = List.foldr cons_f (pure [])
      where cons_f x ys = liftA2 (:) (f x) ys

-- (:) → cons
-- [] → nil
-- List instances
-- https://hackage.haskell.org/package/base-4.15.0.0/docs/src/GHC-Base.html#NonEmpty
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

-- Traversable
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Data-Traversable.html
instance Traversable [] where
sequenceA :: Applicative f => [f a] -> f [a]
sequenceA xs = foldr cons (pure []) xs
where
  cons :: f a -> f [a] -> f [a]
  cons x ys = liftA2 (:) x ys

-- Foldable
-- https://hackage.haskell.org/package/base-4.16.0.0/docs/Data-Foldable.html#t:Foldable
instance Foldable [] where
foldr            :: (a -> b -> b) -> b -> [a] -> b
foldr _ z []     =  z
foldr f z (x:xs) =  f x (foldr f z xs)
```
