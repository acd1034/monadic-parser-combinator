/// @file parser.hpp
#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <mpc/control.hpp>
#include <mpc/data.hpp>

namespace mpc {
#define MPC_FORWARD(x) std::forward<decltype(x)>(x)

  template <class T>
  T decay(T); // no definition

  template <class T, class U>
  concept similar_to = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

  using String = std::list<char>;
  using ParseError = std::string;
  using ParseResult = either<ParseError, std::pair<char, String>>;
  using Parser = StateT<String, ParseResult>;
} // namespace mpc

template <class T>
struct mpc::alternative_traits<mpc::either<mpc::ParseError, std::pair<T, mpc::String>>> {
  static constexpr auto combine = //
    // TODO: m1, m2 を is_Parser<T> で制約
    [](auto&& m1, auto&& m2) -> either<mpc::ParseError, std::pair<T, mpc::String>> {
    if (m1.index() == 0) {
      if (m2.index() == 0) {
        return make_left(*fst(MPC_FORWARD(m1)) + " and " + *fst(MPC_FORWARD(m2)));
      } else {
        return MPC_FORWARD(m2);
      }
    } else {
      return MPC_FORWARD(m1);
    }
  };
};

namespace mpc {
  // Parsers
  // https://hackage.haskell.org/package/parsec-3.1.15.1/docs/Text-Parsec.html#g:1

  /// パーサーと文字列を受け取り、パースする。パースに成功した場合、パース結果を表示する。失敗した場合、エラーメッセージを表示する。
  inline constexpr auto parse_test = //
    // TODO: parser を is_Parser<T> で制約
    partial([](auto&& parser, std::string_view sv) {
      auto result = eval_StateT % MPC_FORWARD(parser) % String(sv.begin(), sv.end());
      if (result.index() == 0) {
        // fail
        std::cout << mpc::quoted(sv) << ' ' << *fst(result) << std::endl;
      } else {
        // succeed
        for (const auto& c : *snd(result))
          std::cout << c;
        std::cout << std::endl;
      }
    });

  // Combinators
  // https://hackage.haskell.org/package/parsec-3.1.15.1/docs/Text-Parsec.html#g:2

  /// エラーメッセージを受け取り、必ず失敗するパーサーを返す。
  inline constexpr auto left = //
    compose
    % lift<Parser> % partial([](similar_to<ParseError> auto&& str) -> eval_StateT_t<Parser> {
        return make_left(MPC_FORWARD(str));
      });

  /// パーサーを受け取り、パーサーを返す。このパーサーはパースに失敗しても直ちにエラーとならない。
  inline constexpr auto try1 = //
    // TODO: parser を is_Parser<T> で制約
    partial([](auto&& parser) {
      return make_StateT<String>(partial(
        [](auto&& parser2, similar_to<String> auto&& str) {
          return run_StateT % MPC_FORWARD(parser2) % MPC_FORWARD(str);
        },
        MPC_FORWARD(parser)));
    });

  /// パーサーを受け取り、パーサーを返す。このパーサーは受け取ったパーサーを1回以上可能な限り適用した結果をリストで返す。
  /// some :: f a -> f [a]
  /// some v = (:) <$> v <*> many v
  inline constexpr auto many1 = //
    // TODO: p, sep を is_Parser<T> で制約
    partial([](auto&& p) {
      return make_StateT<String>(partial(
        [](auto&& p2, similar_to<String> auto&& str)
          -> decltype(run_StateT % (sequence % std::list{p2}) % str) {
          const auto parse = run_StateT % MPC_FORWARD(p2);
          auto result = parse % MPC_FORWARD(str);
          if (result.index() == 0)
            return make_left(*fst(std::move(result)));
          auto [value, state] = *snd(std::move(result));
          std::list<holding_t<decltype(p2)>> ret{std::move(value)};

          for (result = parse % state; result.index() != 0; result = parse % state) {
            std::tie(value, state) = *snd(std::move(result));
            ret.push_back(std::move(value));
          }
          return make_right(std::make_pair(std::move(ret), std::move(state)));
        },
        MPC_FORWARD(p)));
    });

  /// パーサーを受け取り、パーサーを返す。このパーサーは受け取ったパーサーを可能な限り適用した結果をリストで返す。
  /// many :: f a -> f [a]
  /// many v = some v <|> pure []
  inline constexpr auto many = //
    // TODO: p を is_Parser<T> で制約
    partial([](auto&& p) {
      using namespace operators::alternatives;
      return many1(MPC_FORWARD(p)) or pure<decltype(p)>(std::list<holding_t<decltype(p)>>{});
    });

  /// @brief between open p close = open *> p <* close
  inline constexpr auto between = //
    // TODO: open, p, close を is_Parser<T> で制約
    partial([](auto&& open, auto&& p, auto&& close) {
      return discard2nd(discard1st(MPC_FORWARD(open), MPC_FORWARD(p)), MPC_FORWARD(close));
    });

  /// @brief sepBy1 p sep = liftA2 (:) p (many (sep *> p))
  inline constexpr auto sep_by1 = //
    // TODO: p, sep を is_Parser<T> で制約
    partial([](auto&& p, auto&& sep) {
      auto p2 = p;
      return liftA2(cons, MPC_FORWARD(p), many % discard1st(MPC_FORWARD(sep), std::move(p2)));
    });

  /// @brief sepBy p sep = sepBy1 p sep <|> pure []
  inline constexpr auto sep_by = //
    // TODO: p, sep を is_Parser<T> で制約
    partial([](auto&& p, auto&& sep) {
      using namespace operators::alternatives;
      return sep_by1(MPC_FORWARD(p), MPC_FORWARD(sep))
             or pure<decltype(p)>(std::list<holding_t<decltype(p)>>{});
    });

  /// chainl1 p op = do {
  ///   x <- p;
  ///   rest x
  /// } where rest x = do {
  ///   f <- op;
  ///   y <- p;
  ///   rest (f x y)
  /// } <|> return x
  inline constexpr auto chainl1 = //
    // TODO: p, op を is_Parser<T> で制約
    partial([](auto&& p, auto&& op) {
      return make_StateT<String>(partial(
        [](auto&& p2, auto&& op2, similar_to<String> auto&& str)
          -> decltype(run_StateT % p2 % str) {
          const auto parse = run_StateT % MPC_FORWARD(p2);
          const auto parse_op = run_StateT % MPC_FORWARD(op2);

          auto result = parse % MPC_FORWARD(str);
          if (result.index() == 0)
            return make_left(*fst(std::move(result)));
          auto [value, state] = *snd(std::move(result));

          for (;;) {
            auto result_op = parse_op % state;
            if (result_op.index() == 0)
              break;
            auto [fn, state_op] = *snd(std::move(result_op));
            auto result2 = parse % std::move(state_op);
            if (result2.index() == 0)
              break;
            auto [value2, state2] = *snd(std::move(result2));
            value = fn(std::move(value), std::move(value2));
            state = std::move(state2);
          }
          return make_right(std::make_pair(std::move(value), std::move(state)));
        },
        MPC_FORWARD(p), MPC_FORWARD(op)));
    });
} // namespace mpc

namespace mpc {
  // Character Parsing
  // https://hackage.haskell.org/package/parsec-3.1.15.1/docs/Text-Parsec-Char.html

  /// 述語を受け取り、パーサーを返す。このパーサーは、文字列の先頭が述語を満たす場合にそれを返す。
  inline constexpr auto satisfy = //
    partial([](std::predicate<char> auto&& pred) {
      return make_StateT<String>(partial(
        [](auto&& pred2, similar_to<String> auto&& str) -> ParseResult {
          using namespace std::string_literals;

          if (auto m = uncons(MPC_FORWARD(str)); m.index() == 0) {
            return make_left("unexpected end of input"s);
          } else if (auto [x, xs] = *snd(std::move(m)); not std::invoke(MPC_FORWARD(pred2), x)) {
            return make_left("unexpected "s + mpc::quoted(std::move(x)));
          } else {
            return make_right(std::make_pair(std::move(x), std::move(xs)));
          }
        },
        MPC_FORWARD(pred)));
    });

  /// 文字を受け取り、パーサーを返す。このパーサーは、文字列の先頭が渡した文字に一致する場合にそれを返す。
  inline constexpr auto char1 = //
    partial([](char c) {
      using namespace operators::alternatives;
      using namespace std::string_literals;
      auto c2 = c;
      return satisfy % (equal_to % std::move(c))
             or left % ("expecting char "s + mpc::quoted(std::move(c2)));
    });

  /// 文字列を受け取り、パーサーを返す。このパーサーは、文字列の先頭が渡した文字列から始まる場合にその文字列を返す。
  inline constexpr auto string = //
    partial([](std::string_view sv) {
      return sequence
             % fmap(partial([](char c) { return char1 % std::move(c); }),
                    std::list(sv.begin(), sv.end()));
    });

  namespace detail {
    using namespace operators::alternatives;
    using namespace std::string_literals;

    inline const auto alnum = satisfy % mpc::isalnum or left % "expecting alnum"s;
    inline const auto alpha = satisfy % mpc::isalpha or left % "expecting alpha"s;
    inline const auto lower = satisfy % mpc::islower or left % "expecting lower"s;
    inline const auto upper = satisfy % mpc::isupper or left % "expecting upper"s;
    inline const auto digit = satisfy % mpc::isdigit or left % "expecting digit"s;
    inline const auto xdigit = satisfy % mpc::isxdigit or left % "expecting xdigit"s;
    inline const auto cntrl = satisfy % mpc::iscntrl or left % "expecting cntrl"s;
    inline const auto graph = satisfy % mpc::isgraph or left % "expecting graph"s;
    inline const auto space = satisfy % mpc::isspace or left % "expecting space"s;
    inline const auto blank = satisfy % mpc::isblank or left % "expecting blank"s;
    inline const auto print = satisfy % mpc::isprint or left % "expecting print"s;
    inline const auto punct = satisfy % mpc::ispunct or left % "expecting punct"s;
    inline const auto any_char = satisfy % (constant % true);
  } // namespace detail

  using detail::alnum, detail::alpha, detail::lower, detail::upper, detail::digit, detail::xdigit,
    detail::cntrl, detail::graph, detail::space, detail::blank, detail::print, detail::punct,
    detail::any_char;
} // namespace mpc
