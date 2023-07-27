#include <iostream>
class BaseClass {
public:
  static void PrintColor(int r, int g, int b) {
    std::cout << "Color: " << r << ", " << g << ", " << b << std::endl;
  }
};

class ClassA : public BaseClass {
public:
  ClassA() {
    std::cout << "ClassA constructor" << std::endl;
  }
};

class ClassB : public BaseClass {
public:
  ClassB() {
    std::cout << "ClassB constructor" << std::endl;
  }
};

class ClassC : public BaseClass {
public:
  ClassC() {
    std::cout << "ClassC constructor" << std::endl;
  }
};

class ClassD : public BaseClass {
public:
  ClassD() {
    std::cout << "ClassD constructor" << std::endl;
  }
};

#define CLASSES_LIST ClassA, ClassB, ClassC, ClassD