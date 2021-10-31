# [StateT](https://hackage.haskell.org/package/transformers-0.6.0.2/docs/src/Control.Monad.Trans.State.Lazy.html#StateT)
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

-- correct
  StateT<State, Monad, V> {
    using Fn = State -> Monad (V, State);
    Fn runStateT;
    StateT(Fn other) : runStateT(other) {}
  };

-- experiment
  StateT<State, Monad, V> {
    using Fn = State -> Monad (V, State);
    Fn fn;
    StateT(Fn fn2) : fn(fn2) {}
  };

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

-- StateT
-- https://qiita.com/sand/items/802b8c4a8ae19f04102b
-- | Construct a state monad computation from a function.
-- (The inverse of 'runState'.)
state :: (Monad m)
      => (s -> (a, s))  -- ^pure state transformer
      -> StateT s m a   -- ^equivalent state-passing computation
state f = StateT (return . f)

-- | Fetch the current value of the state within the monad.
get :: (Monad m) => StateT s m s
get = state $ \ s -> (s, s)

-- | @'put' s@ sets the state within the monad to @s@.
put :: (Monad m) => s -> StateT s m ()
put s = state $ \ _ -> ((), s)
```

# MonadTrans
```hs
class MonadTrans t where
    -- | Lift a computation from the argument monad to the constructed monad.
    lift :: (Monad m) => m a -> t m a

-- StateT
-- https://hackage.haskell.org/package/transformers-0.6.0.2/docs/src/Control.Monad.Trans.State.Lazy.html#StateT
instance MonadTrans (StateT s) where
    lift m = StateT $ \ s -> do
        a <- m
        return (a, s)
```
