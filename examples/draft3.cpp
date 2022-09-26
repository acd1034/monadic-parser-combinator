#include <charconv>
#include <mpc/parser.hpp>

inline constexpr auto readint = //
  mpc::partial([](mpc::similar_to<mpc::String> auto&& s) {
    std::string str(s.begin(), s.end());
    std::int64_t num{};
    if (auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), num);
        ec == std::errc{}) {
      return num;
    } else
      throw "Failed to convert chars to integer";
  });
inline constexpr auto readstr = //
  mpc::partial(
    [](mpc::similar_to<mpc::String> auto&& s) { return std::string(s.begin(), s.end()); });
inline constexpr auto make_pair = //
  mpc::partial([](auto&& x, auto&& y) { return std::make_pair(MPC_FORWARD(x), MPC_FORWARD(y)); });

inline const auto spaces = mpc::many % mpc::space;
inline constexpr auto token = //
  mpc::partial([](auto&& parser) { return mpc::discard2nd(MPC_FORWARD(parser), spaces); });
inline constexpr auto char_token = mpc::compose(token, mpc::char1);

int main() {
  using namespace mpc::operators::alternatives;
  const auto num = token % (mpc::many1 % mpc::digit);
  const auto ident1 = mpc::alpha or mpc::char1 % '_';
  const auto ident2 = mpc::alnum or mpc::char1 % '_';
  const auto ident = token % mpc::liftA2(mpc::cons, ident1, mpc::many % ident2);

  // primary = num
  const auto primary = mpc::fmap(readint, num);
  // assign = ident "=" primary
  const auto assign =
    mpc::liftA2(make_pair, mpc::fmap(readstr, ident), mpc::discard1st(char_token % '=', primary));
  // args = stmt?
  // stmt = assign ("," assign)*
  const auto args = mpc::sep_by(assign, char_token % ',');

  std::string_view sv = "num1 = 123, num2 = 234";
  auto result = mpc::eval_StateT % args % mpc::String(sv.begin(), sv.end());
  for (const auto& [id, n] : *mpc::snd(result))
    std::cout << id << ": " << n << std::endl;
}
