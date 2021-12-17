# StateT
```hs
newtype StateT s m a = StateT { runStateT :: s -> m (a,s) }

instance (Functor m, MonadPlus m) => Alternative (StateT s m) where
    empty = StateT $ \ _ -> mzero
    StateT m <|> StateT n = StateT $ \ s -> m s `mplus` n s

instance MonadTrans (StateT s) where
    lift m = StateT $ \ s -> do
        a <- m
        return (a, s)

evalStateT m s = liftM fst (runStateT m s)

execStateT m s = liftM snd (runStateT m s)

mapStateT :: (m (a, s) -> n (b, s)) -> StateT s m a -> StateT s n b
mapStateT f m = StateT $ f . runStateT m

-- withState f m = modify f >> m
withStateT :: (s -> s) -> StateT s m a -> StateT s m a
withStateT f m = StateT $ runStateT m . f

state :: (Monad m)
      => (s -> (a, s))  -- ^pure state transformer
      -> StateT s m a   -- ^equivalent state-passing computation
state f = StateT (return . f)

get :: (Monad m) => StateT s m s
get = state $ \ s -> (s, s)

put :: (Monad m) => s -> StateT s m ()
put s = state $ \ _ -> ((), s)

-- modify f = get >>= (put . f)
modify :: (Monad m) => (s -> s) -> StateT s m ()
modify f = state $ \ s -> ((), f s)

-- modify' f = get >>= (($!) put . f)
modify' :: (Monad m) => (s -> s) -> StateT s m ()
modify' f = do
    s <- get
    put $! f s

-- gets f = liftM f get
gets :: (Monad m) => (s -> a) -> StateT s m a
gets f = state $ \ s -> (f s, s)
```

# State
```hs
type State s = StateT s Identity

runState m = runIdentity . runStateT m

evalState m s = fst (runState m s)

execState m s = snd (runState m s)

mapState :: ((a, s) -> (b, s)) -> State s a -> State s b
mapState f = mapStateT (Identity . f . runIdentity)

withState = withStateT
```

# MonadTrans
```hs
-- https://hackage.haskell.org/package/transformers-0.6.0.2/docs/Control-Monad-Trans-Class.html
class (forall m. Monad m => Monad (t m)) => MonadTrans t where
    -- | Lift a computation from the argument monad to the constructed monad.
    lift :: (Monad m) => m a -> t m a
```


# MonadState
```hs
-- | Minimal definition is either both of @get@ and @put@ or just @state@
class Monad m => MonadState s m | m -> s where
    -- | Return the state from the internals of the monad.
    get :: m s
    get = state (\s -> (s, s))

    -- | Replace the state inside the monad.
    put :: s -> m ()
    put s = state (\_ -> ((), s))

    -- | Embed a simple state action into the monad.
    state :: (s -> (a, s)) -> m a
    state f = do
      s <- get
      let ~(a, s') = f s
      put s'
      return a

modify :: MonadState s m => (s -> s) -> m ()
modify f = state (\s -> ((), f s))

modify' :: MonadState s m => (s -> s) -> m ()
modify' f = do
  s' <- get
  put $! f s'

gets :: MonadState s m => (s -> a) -> m a
gets f = do
    s <- get
    return (f s)
```

# Understanding StateT
```hs
-- definition
newtype StateT state monad v =
  StateT { runStateT :: (state -> monad (v, state)) }

instance (Monad monad) => Monad (StateT state monad) where
  return v = StateT $ \state -> return (v, state)
  (StateT x) >>= f = StateT $ \state -> do
    -- `x` is function which take `state` and return `monad (v, state')`
    (v, state') <- x state -- unwrap `monad` and get new value and state
    runStateT (f v) state' -- pass them to f

-- experiment
  return<StateT<State, Monad, V>> v =
    StateT{ \state -> return<Monad> (v, state); }

  -- fn2 :: a -> StateT b
  -- (>>=) :: m a -> (a -> m b) -> m b
  (StateT fn) >>= fn2 = StateT{
    \state ->
    -- fn state == monad (v, state')
    -- (>>=) unwrap monad
    fn state >>= \(v, state') ->
    -- fn2 v == StateT fn'
    -- runStateT (StateT fn') == fn'
    runStateT (fn2 v) state' -- what is `runStateT` ?
  }

  -- another definition of (>>=)
  m >>= k = StateT { \s -> do
    -- runStateT (StateT fn) s == fn s
    (a, s') <- runStateT m s
    runStateT (k a) s'
  }
```

# do記法はliftAN
```hs
-- 以下は同じ書き方
-- https://qiita.com/7shi/items/4408b76624067c17e933#state%E3%83%A2%E3%83%8A%E3%83%89
get3 = evalStateT $ do
    x <- getch
    y <- getch
    z <- getch
    return [x, y, z]

get3 = evalStateT $
    liftA3 (\x y z -> [x, y, z]) getch getch getch
```
