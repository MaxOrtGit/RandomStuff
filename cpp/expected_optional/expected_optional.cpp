#include <expected>
#include <random>
#include <optional>
#include <iostream>
#include <format>

enum Error_Code { Level1, Level2, Level3, Level4, Level5, Level6, Level7, Level8, Level9, Level10};

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> Error_dist(0, 9);

constexpr Error_Code get_error_code()
{
  return static_cast<Error_Code>(Error_dist(gen));
}

using optbool = std::optional<bool>;
using expbool = std::expected<bool, Error_Code>;
using expstr = std::expected<std::string, Error_Code>;

auto is_right(optbool force = std::nullopt) -> expbool
{
  if (force)
    return force.value();
  else
    return std::unexpected(get_error_code());
}

// and_then must return an expected doesn't have to be the same type
// or_else must return an expected does have to be the same type
// transform returns an expected

int main()
{
  auto vals = { is_right(), is_right(true), is_right(false) };
  for (auto& v : vals)
  {
    std::cout << "value: " <<
    v.and_then([](bool b) -> expstr { 
        std::cout << "converting to string\n";
        return std::format("\"{}\"", b); 
      })
      .or_else([](Error_Code ec) { 
        std::cout << "correcting error: " << ec << "\n"; 
        return expstr("hi");
      })
      .transform([](std::string s) {
        std::cout << "value: " << s << "\n";
        return s;
      }).value_or("hi") << "\n";
  }     
}