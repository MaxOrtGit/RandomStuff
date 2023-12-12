#include <iostream>


template <typename ... Args>
void print(Args ... args) {
  auto printWithComma = [i = sizeof...(args)](const auto& arg) mutable {
    std::cout << arg;
    if (--i)
      std::cout << ", ";
    else
      std::cout << std::endl;
  };

  (printWithComma(args), ...);
}

template<typename... Args>
void printArgs(Args... args) {
  ((std::cout << args << ' '), ...);
}


int main()
{
  printArgs(1, 2.4, "hello", 'c', 'c', 'c', 'c', 'c');
}