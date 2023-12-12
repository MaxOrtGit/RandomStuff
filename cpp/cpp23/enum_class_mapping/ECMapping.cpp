#include <iostream>
#include <functional> // for std::function and std::mem_fn
#include <vector>
#include <unordered_map>
#include <map>

#include <memory>
#include <random>
#include <ctime> // for std::time for random
#include "Classes.h"

// ------------------ Functions for creating static calls ------------------
template <typename T, typename Base>
concept HasStaticCallOperator = requires {std::function(T::template operator()<Base>);};

template <typename T, typename Base>
concept HasCallOperator = requires {std::mem_fn(T::template operator()<Base>);};

// MakeContainerOfCalls generic function (allows for passing of any Args)
// Static call operator returns std::functions
// Non-static call operator returns std::mem_fns
template<typename Base, typename T, template <typename> class Container, typename ...Args>
auto constexpr _MakeContainerOfCalls_Generic()
{
  if constexpr (HasStaticCallOperator<T, Base>) 
  {
    // Static call operator
    using Func = decltype(std::function(T::template operator()<Base>));
    return Container<Func>{T::template operator()<Args>...};
  }
  else if constexpr (HasCallOperator<T, Base>) 
  {
    // Non-static call operator
    using Func = decltype(std::mem_fn(T::template operator()<Base>));
    return Container<Func>{std::mem_fn(T::template operator()<Args>)...};
  }
  else 
  {
    static_assert(false, "T does not have a call operator");
  }
}

// Default with CLASSES_LIST as Args
template<typename T, template <typename> class Container = std::vector>
auto constexpr MakeContainerOfCalls()
{
  // automatically put CLASSES_LIST
  return _MakeContainerOfCalls_Generic<BaseClass, T, Container, CLASSES_LIST>();
}

// MakeMapOfCalls generic function (allows for passing of any Args)
// Static call operator returns std::functions
// Non-static call operator returns std::mem_fns
template<typename Base, typename T, template <typename, typename> class Map, typename ...Args>
auto constexpr _MakeMapOfCalls_Generic(int offset = 0)
{
  if constexpr (HasStaticCallOperator<T, Base>) 
  {
    // Static call operator
    using Func = decltype(std::function(T::template operator()<Base>));
    return Map<Classes, Func>{ {static_cast<Classes>(offset++), (T::template operator()<Args>)}...};
  }
  else if constexpr (HasCallOperator<T, Base>) 
  {
    // Non-static call operator
    using Func = decltype(std::mem_fn(T::template operator()<Base>));
    return Map<Classes, Func>{ {static_cast<Classes>(offset++), std::mem_fn(T::template operator()<Args>)}...};
  }
  else 
  {
    static_assert(false, "T does not have a call operator");
  }
}

// Default with CLASSES_LIST as Args
template<typename T, template <typename, typename> class Map = std::unordered_map>
auto constexpr MakeMapOfCalls(int offset = 0)
{
  // automatically put CLASSES_LIST
  return _MakeMapOfCalls_Generic<BaseClass, T, Map, CLASSES_LIST>(offset);
}

// name, return type, (args){code}
#define MakeGenericInstance(Name, ReturnType, ...) \
struct Name \
{ \
  template <typename T> \
  static constexpr ReturnType operator() \
    __VA_ARGS__; \
}; 


// ------------------ Function to create an instance ------------------
MakeGenericInstance(ConstructorInstance, std::unique_ptr<T>, 
/*constexpr std::unique_ptr<T> ConstructorInstance*/(){
  return std::make_unique<T>();
})

// additional cases need to be added in a function body
static constexpr std::unique_ptr<BaseClass> NoneClassConstructor()
{
  std::cout << "No Class" << std::endl;
  return nullptr;
}

auto constructors = MakeContainerOfCalls<ConstructorInstance>();


// ------------------ Functions for call color on an instance ------------------
template <typename T, typename... Args>
concept ClassInArgs = (std::same_as<T, Args> || ...);

template <typename T>
concept IsCoolClass = ClassInArgs<T, ClassC, ClassD>;

MakeGenericInstance(ColorInstance, void, 
(int r, int g, int b) {
  // override
  if constexpr (IsCoolClass<T>) {
    std::cout << "Cooler ";
  }
  T::PrintColor(g, b, r);
})

static constexpr void BaseClassColorer(int r, int g, int b) 
{
  std::cout << "BaseClass ";
  BaseClass::PrintColor(r, g, b);
}
static constexpr void NoneClassColorer(int r, int g, int b) 
{
  std::cout << "No Class to print color" << std::endl;
}

// because Classes::None is 0, we need to offset it by 1 to the first class
// overriding default map type (unordered_map -> map)
auto colorers = MakeMapOfCalls<ColorInstance, std::map>(static_cast<int>(Classes::ClassA));
// have to add new cases / overrides in a function body


// ------------------ Functions for set color ------------------
// we manually  create functor so we can store values
class ColorSetter {
public:
  int r, g, b;

  constexpr ColorSetter(int r, int g, int b) : r(r), g(g), b(b) {}

  // default
  template <typename T>
  constexpr void operator()() {
    T::PrintColor(r, g, b);
  }
  
  constexpr void BaseSet() {
    std::cout << "BaseClass ";
    BaseClass::PrintColor(r, g, b);
  }

  constexpr void NoneSet() {
    std::cout << "No Class to color" << std::endl;
  }
};

// For ColorSetter we cant use the [] operator (because mem_fn has no default constructor)
// instead we use the at() and insert(std::mem_fn(function)) functions
auto setters = MakeMapOfCalls<ColorSetter>(static_cast<int>(Classes::ClassA));


// ------------------ Setting up random ------------------
using Dist = std::uniform_int_distribution<int>;
std::mt19937 rng(static_cast<unsigned>(std::time(0)));


// ------------------ Adding new cases and overrides ------------------
void AddOverrides() {
  // Add None constructor to the beginning of the vector
  //constructors.insert(constructors.begin(), NoneClassConstructor);

  // Add new cases and overrides for colorers
  colorers.insert({{Classes::None, NoneClassColorer}, {Classes::BaseClass, BaseClassColorer}});

  // Adding new case into setters
  setters.insert({Classes::None, std::mem_fn(ColorSetter::NoneSet)});
  setters.insert({Classes::BaseClass, std::mem_fn(ColorSetter::BaseSet)});
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
  ColorSetter color(255, 25, 5);
  setters.at(Classes::None)(color);
  setters.at(Classes::ClassA)(color);
  setters.at(Classes::ClassB)(color);
  setters.at(Classes::BaseClass)(color);
  
  return 0;
}
