#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <random>
#include <ctime>
#include "Classes.cpp"

enum class Classes { CLASSES_LIST };


template<typename Function>
using FuncVec = std::vector<std::function<Function>>;

#define MAKE_VECTOR_FUNCTION(func) \
  template<typename... Args> \
  auto Make##func##Vector() { \
    return FuncVec<decltype(func<BaseClass>)>{func<Args>...}; \
  }

#define CreateVectorOfFunctions(variableName, func) \
  MAKE_VECTOR_FUNCTION(func) \
  auto variableName = Make##func##Vector<CLASSES_LIST>();

/* example of CreateVectorOfFunctions(constructors, ConstructorInstance)

  // ------------------ Function to return vector ------------------
  template<typename... Args>
  auto MakeConstructorInstanceVector() {
    return FuncVec<decltype(ConstructorInstance<BaseClass>)>{ConstructorInstance<Args>...};
  }

  // ------------------ Creating the vector ------------------
  auto constructors = MakeConstructorInstanceVector<CLASSES_LIST>();
*/

// ------------------ Function to create an instance ------------------
template<typename T>
std::unique_ptr<BaseClass> ConstructorInstance() {
  return std::make_unique<T>();
}
CreateVectorOfFunctions(constructors, ConstructorInstance)

// ------------------ Function call color on an instance ------------------
template<typename T>
void ColorInstance(int r, int g, int b) {
  T::PrintColor(r, g, b);
}
CreateVectorOfFunctions(colorers, ColorInstance)

// ------------------ Using the functions ------------------

// ------------------ Enum to creating an instance ------------------
// Function to create an instance based on the enum value
std::unique_ptr<BaseClass> CreateObjectFromEnum(Classes enumValue) {
  // Creates a vector of functions that create an instance of the corresponding class
  int index = static_cast<int>(enumValue);

  // Calls the function and returns pointer to the instance
  return constructors[index]();
}

int main() {
  // Enum to object creation
  Classes enumValue = Classes::ClassC;
  auto instance = CreateObjectFromEnum(enumValue);
  std::cout << std::endl;


  // Constructs 10 random classes
  auto constructors = MakeConstructorInstanceVector<CLASSES_LIST>();
  
  std::mt19937 rng(static_cast<unsigned>(std::time(0)));
  std::uniform_int_distribution<int> dist(0, constructors.size() - 1);
  for (int i = 0; i < 10; ++i) {
    int randomIndex = dist(rng);
    std::unique_ptr<BaseClass> instance = constructors[randomIndex]();
  }

  // Colors all classes (calling function with arguments)
  for (auto& colorer : colorers) {
    colorer(1, 2, 3);
  }

  return 0;
}
