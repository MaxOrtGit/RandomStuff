#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <random>
#include <ctime>
#include <unordered_map>
#include "Classes.h"

// MakeVector impl function (allows for passing of any Args)
template<template <typename P> class T, typename ...Args>
auto constexpr _MakeVector_impl()
{
  using Func = decltype(std::function(T<BaseClass>()));
  return std::vector<Func>{T<Args>()...};
}

// Default with CLASSES_LIST as Args
template<template <typename P> class T>
auto constexpr MakeVector()
{
  // automatically put CLASSES_LIST
  return _MakeVector_impl<T, CLASSES_LIST>();
}

// MakeEnum impl function (allows for passing of any Args)
template<template <typename P> class T, typename ...Args>
auto constexpr _MakeEnum_impl()
{
  using Func = decltype(std::function(T<BaseClass>()));
  size_t i = 0;
  return std::unordered_map<Classes, Func>{ {static_cast<Classes>(i++), T<Args>()}...};
}

// Default with CLASSES_LIST as Args
template<template <typename P> class T>
auto constexpr MakeEnum()
{
  // automatically put CLASSES_LIST
  return _MakeEnum_impl<T, CLASSES_LIST>();
}


// ------------------ Function to create an instance ------------------

template<typename T>
struct ConstructorInstance
{
  static constexpr std::unique_ptr<BaseClass> operator()() 
  {
    return std::make_unique<T>();
  }
};
auto constructors = MakeVector<ConstructorInstance>();


// ------------------ Function call color on an instance ------------------
template<typename T>
struct ColorInstance 
{
  static constexpr void operator()(int r, int g, int b) 
  {
    T::PrintColor(r, g, b);
  }
};
auto colorers = MakeEnum<ColorInstance>();

// ------------------ Setting up random ------------------

using Dist = std::uniform_int_distribution<int>;
std::mt19937 rng(static_cast<unsigned>(std::time(0)));


// ------------------ Using the functions ------------------

int main() {
  // Calling templated functor
  auto instance = constructors[3]();
  std::cout << std::endl;
  
  // Randomly calling at runtime
  Dist dist(0, constructors.size() - 1);
  for (int i = 0; i < 3; ++i) {
    auto instance = constructors[dist(rng)]();
  }

  // Colors all classes (calling function with arguments)
  Dist CDist(0, 255);
  for (const auto& [_, func] : colorers) {
    func(CDist(rng), CDist(rng), CDist(rng));
  }


  return 0;
}
