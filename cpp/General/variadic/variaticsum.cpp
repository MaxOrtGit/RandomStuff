#include <iostream>
#include <tuple>
#include <vector>
#include <ranges>
#include <string>


template <typename T>
concept Addable = requires(T a, T b) {
  { a + b };
};

template <typename T>
concept Range = requires(T a) {
  { std::ranges::size(a) };
  { std::ranges::begin(a) };
  { std::ranges::end(a) };
};

template <Range R>
std::ranges::range_value_t<R> inline constexpr to_num(const R& range) {
  using type = typename std::ranges::range_value_t<R>;
  if (std::ranges::size(range) == 0) {
    return type();
  }
  auto it = std::ranges::begin(range);
  auto end = std::ranges::end(range);
  type total = *it;
  while (++it != end) {
    total += *it;
  }
  return total;
}

template <Addable A>
auto inline constexpr to_num(A a) {
  return a;
}

template <typename ... Args>
void constexpr sum(const Args&... args) {
  auto total = (to_num(args) + ...);
  
  std::cout << "Sum: " << total << std::endl;
}


int main() {
  sum(1, 2, 3, 4, 5);
  sum(10, std::vector{1, 3, 4}, 30);
  sum(7, 13, 25);
  
  return 0;
}