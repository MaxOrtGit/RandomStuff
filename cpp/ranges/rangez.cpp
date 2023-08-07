#include <ranges>
#include <vector>
#include <iostream>
#include <numeric>

template <typename T>
concept Range = requires (T t) {
  std::ranges::begin(t);
  std::ranges::end(t);
};

void print(Range auto&& r)
{
  for (auto&& e : r)
    std::cout << e << ", ";
  std::cout << std::endl;
}

template <typename T>
std::vector<T> vector_from_range(Range auto&& r)
{
  return {std::ranges::begin(r), std::ranges::end(r)};
}


std::vector<int> vector_from_iota(int start, int end)
{
  auto iota = std::views::iota(start, end);
  return vector_from_range<int>(iota);
}

int main() 
{
  auto v1 = vector_from_iota(0, 20);
  print(v1);
  v1 = vector_from_range<int>(v1 | std::views::filter([](int i) { return i % 2 == 0; }));
  print(v1);
}