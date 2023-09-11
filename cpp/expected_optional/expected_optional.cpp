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

// returns the bool value or if optional is empty, returns an random error code
auto is_right(optbool force = std::nullopt) -> expbool
{
  if (force)
    return force.value();
  else
    return std::unexpected(get_error_code());
}

// and_then must return an expected doesn't have to be the same type
// or_else must return an expected does have to be the same type
// transform returns an expected with a different main type
// transform_error returns an expected with a different error type

int main()
{
  auto vals = { is_right(), is_right(true), is_right(false) };
  for (auto& v : vals)
  {
    std::cout <<
    v.and_then([](bool b) -> expstr { // keeps error code but changes type to string
        std::cout << "converting to string\n";
        return std::format("\"{}\"", b); 
      })
      .transform_error([](Error_Code ec) -> int { // changes error type to int
        std::cout << "converting error code to int type\n";
        return static_cast<int>(ec); 
      })
      .or_else([](int ec) -> expstr { 
        std::cout << "correcting error: " << ec << "\n"; 
        return std::format("error code: {}", ec);
      })
      .transform([](std::string s) -> std::string {
        return std::format("value: {}\n\n", s);
      }).value();
  }     
}