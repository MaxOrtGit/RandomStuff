#include <iostream>

template <typename Parent, typename Child>
using MatchConst = std::conditional_t<std::is_const_v<std::remove_reference_t<Parent>>, const Child, Child>;

// predefine
struct Data;
class Class;

struct Data
{
  int i = 0, j = 0, k = 0;

  Data(int i, int j, int k) : i(i), j(j), k(k) {}
};

class Class
{
public:
  Data data;
  std::unique_ptr<Data> dataPTR;
  
  Class(int i, int j, int k) : data(i, j, k), dataPTR(std::make_unique<Data>(i, j, k)) {}


  // with deducing this
  // like_t wasn't added to the standard and there is no way to check for const in Self
  template <typename Self>
  auto&& operator*(this Self&& self)
  {
    return std::forward_like<Self>(self.data);
  }

  template <typename Self>
  MatchConst<Self, Data>* operator->(this Self&& self)
  {
    return self.dataPTR.get();
  }

  // with the old syntax
  /*
  Data& operator*(this) &
  {
    return data;
  }
  const Data& operator*(this) const&
  {
    return data;
  }
  Data&& operator*(this) &&
  {
    return std::move(data);
  }
  
  Data* operator->(this)
  {
    return &data;
  }
  const Data* operator->(this) const
  {
    return &data;
  }
  */
};

int main()
{
  Class c(1, 2, 3);
  const Class cc(-1, -2, -3);

  *c = { 4, 5, 6 };
  // *cc = { 7, 8, 9 }; // error 

  c->i = 10;
  std::cout << cc->i << ", " << cc->j << ", " << cc->k << "\n";
  // cc->i = 10; // error

  return 0;
}


