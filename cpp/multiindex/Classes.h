#include <iostream>
#include <string>

class BaseClass {

public:
  BaseClass(std::string name = "BaseClass")
  {
    std::cout << name << " constructor" << std::endl;
  }

  static void PrintColor(int r, int g, int b) {
    std::cout << "Color: " << r << ", " << g << ", " << b << std::endl;
  }
};

class ClassA : public BaseClass {
public:
  ClassA() : BaseClass("ClassA") {}
};

class ClassB : public BaseClass {
public:
  ClassB() : BaseClass("ClassB") {}
};

class ClassC : public BaseClass {
public:
  ClassC() : BaseClass("ClassC") {}
};

class ClassD : public BaseClass {
public:
  ClassD() : BaseClass("ClassD") {}
};

#define CLASSES_LIST ClassA, ClassB, ClassC, ClassD

enum class Classes {BaseClass = -1, None = 0, CLASSES_LIST };