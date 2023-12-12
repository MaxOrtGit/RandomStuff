#include <fstream>
#include <iostream>

class A
{
  virtual std::istream& operator>>(std::istream& in) = 0;
};

class B : public A
{
  virtual std::istream& operator>>(std::istream& in)
  {
    std::cout << "B\n";
    // read B
    return in;
  }
};

class C : public A
{
  virtual std::istream& operator>>(std::istream& in)
  {
    std::cout << "C\n";
    // read C
    return in;
  }
};

int main()
{
  A* b = new B;
  std::ifstream file("file.entity");
  file >> *b;
}