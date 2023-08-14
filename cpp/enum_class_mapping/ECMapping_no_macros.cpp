#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <random>
#include <ctime>
#include <unordered_map>
#include <map>
#include "Classes.h"

// MakeContainerOfFunctions impl function (allows for passing of any Args)
template<typename T, template <typename> class Container, typename ...Args>
auto constexpr _MakeContainerOfFunctions_impl()
{
  using Func = decltype(std::function(T::template operator()<BaseClass>));
  return Container<Func>{T::template operator()<Args>...};
}

// Default with CLASSES_LIST as Args
template<typename T, template <typename> class Container = std::vector>
auto constexpr MakeContainerOfFunctions()
{
  // automatically put CLASSES_LIST
  return _MakeContainerOfFunctions_impl<T, std::vector, CLASSES_LIST>();
}

// MakeMapOfFunctions impl function (allows for passing of any Args)
template<typename T, template <typename, typename> class Map, typename ...Args>
auto constexpr _MakeMapOfFunctions_impl(int offset = 0)
{
  using Func = decltype(std::function(T::template operator()<BaseClass>));
  return Map<Classes, Func>{ {static_cast<Classes>(offset++), (T::template operator()<Args>)}...};
}

// Default with CLASSES_LIST as Args
template<typename T, template <typename, typename> class Map = std::unordered_map>
auto constexpr MakeMapOfFunctions(int offset = 0)
{
  // automatically put CLASSES_LIST
  return _MakeMapOfFunctions_impl<T, Map, CLASSES_LIST>(offset);
}


// ------------------ Function to create an instance ------------------
struct ConstructorInstance
{
  template<typename T>
  static constexpr std::unique_ptr<T> operator()() 
  {
    return std::make_unique<T>();
  }
};

// additional cases need to be added in a function body
static constexpr std::unique_ptr<BaseClass> NoneClassConstructor()
{
  std::cout << "No Class" << std::endl;
  return nullptr;
}
auto constructors = MakeContainerOfFunctions<ConstructorInstance>();


// ------------------ Function call color on an instance ------------------
struct ColorInstance 
{
  template<typename T>
  static constexpr void operator()(int r, int g, int b) 
  {
    T::PrintColor(r, g, b);
  }
};

static constexpr void BaseClassColorer(int r, int g, int b) 
{
  std::cout << "BaseClass ";
  BaseClass::PrintColor(r, g, b);
}
static constexpr void NoneClassColorer(int r, int g, int b) 
{
  std::cout << "No Class" << std::endl;
}
template <typename T>
static constexpr void CoolerClassColorer(int r, int g, int b) 
{
  std::cout << "Cooler ";
  T::PrintColor(r, g, b);
}

// because Classes::None is 0, we need to offset it by 1 to the first class
// overriding default map type (unordered_map -> map)
auto colorers = MakeMapOfFunctions<ColorInstance, std::map>(static_cast<int>(Classes::ClassA));
// have to add new cases / overrides in a function body

// ------------------ Function Set color ------------------
// we manually  create functor so we can store values
class ColorSetter {
public:
  static int r, g, b;

  static constexpr void SetColor(int r, int g, int b) {
    ColorSetter::r = r;
    ColorSetter::g = g;
    ColorSetter::b = b;
  }

  template <typename T>
  static constexpr void operator()() {
    T::PrintColor(r, g, b);
  }
};
int ColorSetter::r = 0;
int ColorSetter::g = 0;
int ColorSetter::b = 0;

auto setters = MakeMapOfFunctions<ColorSetter>();

// ------------------ Setting up random ------------------
using Dist = std::uniform_int_distribution<int>;
std::mt19937 rng(static_cast<unsigned>(std::time(0)));

// ------------------ Adding new cases and overrides ------------------
void AddOverrides() {
  // Add None constructor to the beginning of the vector
  constructors.insert(constructors.begin(), NoneClassConstructor);

  // Add new cases and overrides for colorers
  colorers.insert({{Classes::BaseClass, BaseClassColorer}, {Classes::None, NoneClassColorer}});
  colorers[Classes::ClassC] = CoolerClassColorer<ClassC>;
  colorers[Classes::ClassD] = CoolerClassColorer<ClassD>;
}


// ------------------ Using the containers and maps ------------------
int main() {
  AddOverrides();

  // Calling templated functor
  auto instance = constructors[3]();
  std::cout << std::endl;
  
  // Randomly calling at runtime
  Dist dist(0, constructors.size() - 1);
  for (int i = 0; i < 5; ++i) {
    auto instance = constructors[dist(rng)]();
  }
  std::cout << std::endl;

  // Colors all classes (calling function with arguments)
  Dist CDist(0, 255);
  for (const auto& [_, func] : colorers) {
    func(CDist(rng), CDist(rng), CDist(rng));
  }
  std::cout << std::endl;

  // Storing values in custom functor
  ColorSetter::SetColor(255, 25, 5);
  setters[Classes::ClassA]();

  return 0;
}
