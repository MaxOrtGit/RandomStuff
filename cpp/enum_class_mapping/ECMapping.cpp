#include <vector>
#include <functional>
#include <random>
#include <ctime>
#include <memory>
#include "Classes.cpp" // This file contains the CLASSES_LIST. it is just the classes with commas between them

enum class Classes { CLASSES_LIST }; // This enum is used pick which class to instantiate

template<typename T>
std::unique_ptr<BaseClass> createInstance() {
  return std::make_unique<T>();
}

using ClassCreator = std::function<std::unique_ptr<BaseClass>()>;
using ClassCreators = std::vector<ClassCreator>;

// Helper function to add constructors to the vector
template<typename T>
void addConstructorImpl(ClassCreators& constructors) 
{
  constructors.push_back(&createInstance<T>);
}

template<typename... Args>
void addConstructors(ClassCreators& constructors) {
  (addConstructorImpl<Args>(constructors), ...);
}

// Function to create an instance based on the enum value
std::unique_ptr<BaseClass> createInstanceFromEnum(Classes enumValue) {
  ClassCreators constructors;
  addConstructors<CLASSES_LIST>(constructors);

  int index = static_cast<int>(enumValue);
  if (index >= 0 && index < constructors.size()) {
    return constructors[index]();
  } else {
    return nullptr; // Return nullptr for invalid enum values or out-of-range indices
  }
}

int main() {
  // Now you can pass the enum value to the function to create an instance of the corresponding class.
  Classes enumValue = Classes::ClassC;
  auto instance = createInstanceFromEnum(enumValue);

  std::cout << std::endl;

  ClassCreators constructors;
  addConstructors<CLASSES_LIST>(constructors);

  std::srand(static_cast<unsigned>(std::time(0)));
  for (int i = 0; i < 10; ++i) {
    int randomIndex = std::rand() % constructors.size();
    std::unique_ptr<BaseClass> instance = constructors[randomIndex]();
  }

  return 0;
}
