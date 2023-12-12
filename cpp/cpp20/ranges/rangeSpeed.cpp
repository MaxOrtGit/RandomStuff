#include <ranges>
#include <vector>
#include <iostream>
#include <chrono>
#include <numeric>

template<typename Func>
auto measureExecutionTime(Func&& func, int iterations = 10000)
{
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iterations; ++i)
    func();
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template<typename Func>
void testFunc(Func&& func, int attempts = 10)
{
  for (int i = 0; i < attempts; ++i)
    std::cout << measureExecutionTime(func) << ", ";
  std::cout << std::endl;
}

std::vector<int> vector_from_iota(int start, int end)
{
  std::vector<int> v(end - start);
  std::iota(std::begin(v), std::end(v), start);
  return v;
}

auto v1 = vector_from_iota(0, 10000);
auto v2 = vector_from_iota(10000, 20000);

auto addVectors1()
{
  // add the two ranges
  auto result = std::views::zip(v1, v2) | std::views::transform([](const auto& pair) {
    return std::get<0>(pair) + std::get<1>(pair);
  });
}

auto addVectors2()
{
  auto result = v1 | std::views::transform([&](int a) {
    return a + *(std::next(v2.begin(), a));
  });
}

auto addVectors3()
{
  auto result = v1 | std::views::transform([&](int a) {
    return a + v2[a];
  });
}

auto addVectors4()
{
  std::vector<int> result(v1.size());
  for (std::size_t i = 0; i < v1.size(); ++i)
    result[i] = v1[i] + v2[i];
}

auto addVectors5()
{
  std::vector<int> result(v1.size());
  std::transform(v1.begin(), v1.end(), v2.begin(), result.begin(), std::plus<int>());
}

#define TestFunc(func) std::cout << "Execution time for " << #func << "(): " << std::endl; testFunc(func);
#define TestFuncN(func, num) std::cout << "Execution time for " << #func << "(): " << std::endl; testFunc(func, num);

int main()
{
  TestFunc(addVectors1);
  
  TestFunc(addVectors2);

  TestFunc(addVectors3);
  
  TestFuncN(addVectors4, 1);
  
  TestFuncN(addVectors5, 1);

  return 0;
}
