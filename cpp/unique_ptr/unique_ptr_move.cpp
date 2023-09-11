#include <memory>
#include <vector>
#include <iostream>

struct Test {
  ~Test() { std::cout << "deleted" << std::endl; }
};

int main()
{
  std::unique_ptr<Test> test = std::make_unique<Test>();
  std::vector<std::unique_ptr<Test>> tests;
  tests.push_back(std::move(test));
  std::cout << "test is now " << (test ? "not null" : "null") << std::endl;
  std::cout << "tests[0] is now " << (tests[0] ? "not null" : "null") << std::endl;
  return 0;
}
