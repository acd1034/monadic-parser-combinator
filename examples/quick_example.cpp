#include <charconv>
#include <mpc/parser.hpp>

inline const auto spaces = mpc::many % mpc::space;

inline constexpr auto token = mpc::partial([](auto&& parser) {
  return mpc::discard2nd(MPC_FORWARD(parser), spaces);
});

inline constexpr auto char_token = mpc::compose(token, mpc::char1);

inline constexpr auto readint = //
  mpc::partial([](mpc::similar_to<mpc::String> auto&& s) {
    std::string str(s.begin(), s.end());
    std::int64_t num{};
    if (auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), num);
        ec == std::errc{}) {
      return num;
    } else
      throw "Conversion from chars to integer failed";
  });

inline constexpr auto readstr = mpc::partial([](mpc::similar_to<mpc::String> auto&& s) {
  return std::string(s.begin(), s.end());
});

inline constexpr auto numop = //
  mpc::partial([](const char c, auto&& x, auto&& y) {
    switch (c) {
    case '+':
      return MPC_FORWARD(x) + MPC_FORWARD(y);
    case '-':
      return MPC_FORWARD(x) - MPC_FORWARD(y);
    case '*':
      return MPC_FORWARD(x) * MPC_FORWARD(y);
    case '/':
      return MPC_FORWARD(x) / MPC_FORWARD(y);
    default:
      throw "Unexpected operator";
    }
  });

int main() {
  // expr   = term ("+" term | "-" term)*
  // term   = factor ("*" factor | "/" factor)*
  // factor = number
  // number = [0-9]+
  using namespace mpc::operators::alternatives;
  const auto number = token % (mpc::many1 % mpc::digit);
  const auto factor = mpc::fmap(readint, number);
  const auto termop = mpc::fmap(numop, char_token % '*' or char_token % '/');
  const auto term = mpc::chainl1(factor, termop);
  const auto exprop = mpc::fmap(numop, char_token % '+' or char_token % '-');
  const auto expr = mpc::chainl1(term, exprop);

  // test
  std::string_view sv = "1*2/3 + 3/4*5 - 5*6/7";
  std::int64_t ans    =  1*2/3 + 3/4*5 - 5*6/7 ;
  auto result = mpc::eval_StateT % expr % mpc::String(sv.begin(), sv.end());
  assert(result.index() == 1);
  assert(*mpc::snd(result) == ans);
}
