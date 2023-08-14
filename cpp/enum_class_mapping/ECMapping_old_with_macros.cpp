#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <random>
#include <ctime>
#include <unordered_map>
#include "Classes.h"

// Template for vector of functions (pass in ReturnType(Args...)))
template<typename Function>
using FuncVec = std::vector<std::function<Function>>;

// Macro to create function that returns vector of the function templated with 
//  all classes passed into the template
#define MAKE_VECTOR_FUNCTION(func) \
  template<typename... Args> \
  auto Make##func##Vector() { \
    return FuncVec<decltype(func<BaseClass>)>{func<Args>...}; \
  }

// Macro to create a variable of a vector of functions (pass in variable name, function)
#define CreateVectorOfFunctions(variableName, func) \
  MAKE_VECTOR_FUNCTION(func) \
  FuncVec<decltype(func<BaseClass>)> variableName = Make##func##Vector<CLASSES_LIST>();

// Function to map enum to vector of functions
template<typename Function>
auto MapEnumToVector(FuncVec<Function>&& vector)
{
  std::unordered_map<Classes, std::function<Function>> map;
  for (int i = 0; i < vector.size(); ++i) {
    map.emplace(static_cast<Classes>(i), vector[i]);
  }
  return map;
}

// Macro to create a variable of a map of functions (pass in variable name, function)
#define CreateMapOfFunctions(variableName, func) \
  MAKE_VECTOR_FUNCTION(func) \
  auto variableName = MapEnumToVector(Make##func##Vector<CLASSES_LIST>());


// ------------------ Function to create an instance ------------------
template<typename T>
std::unique_ptr<BaseClass> ConstructorInstance() {
  return std::make_unique<T>();
}
CreateMapOfFunctions(constructors, ConstructorInstance)

// ------------------ Function call color on an instance ------------------
template<typename T>
void ColorInstance(int r, int g, int b) {
  T::PrintColor(r, g, b);
}
CreateMapOfFunctions(colorers, ColorInstance)

// ------------------ Setting up random ------------------

using Dist = std::uniform_int_distribution<int>;
std::mt19937 rng(static_cast<unsigned>(std::time(0)));


// ------------------ Using the functions ------------------

int main() {
  // Calling templated function
  auto instance = constructors[Classes::ClassC]();
  std::cout << std::endl;
  
  Dist dist(0, constructors.size() - 1);
  for (int i = 0; i < 3; ++i) {
    auto instance = constructors[static_cast<Classes>(dist(rng))]();
  }

  // Colors all classes (calling function with arguments)
  Dist CDist(0, 255);
  for (const auto& [_, func] : colorers) {
    func(CDist(rng), CDist(rng), CDist(rng));
  }


  return 0;
}
