#include <iostream>

// use cases:
// Removing Duplicate Code for const/ref type (RemoveDuplicates.cpp)
// CRTP without needing to template


// predefine
struct Base;
struct Derived1;
struct Derived2;

struct OtherBase;

// concepts
template <typename T>
concept isBaseType = std::is_base_of_v<Base, T>;
template <typename T>
concept IsBase = std::is_same_v<T, Base>;
template <typename T>
concept IsDerived1 = std::is_same_v<T, Derived1>;
template <typename T>
concept IsDerived2 = std::is_same_v<T, Derived2>;
template <typename T>
concept IsDerived = std::is_base_of_v<Base, T> && !IsBase<T>;

struct Base 
{
  int i = 0, j = 0, k = 0;
  
  // Gets the name of the class through deducing this
  template <typename Self>
  void PrintClass(this const Self& self) 
  {
    std::cout << typeid(Self).name() << "\n";
  }
  
  // "overrides" by adding the l/m properties if derived
  template <typename Self>
  void PrintProperties(this const Self& self)
  {
    std::cout << self.i << ", " << self.j << ", " << self.k;
    if constexpr (IsDerived1<Self>) 
    {
      std::cout << ", l: " << self.l;
    }
    if constexpr (IsDerived2<Self>) 
    {
      std::cout << ", m: " << self.m;
    }

    std::cout << "\n";
  }
  
  // able to make a copy of the derived class in the base class
  template <typename Self>
  Self operator++(this Self& self)
  {
    auto temp = self;
    ++self.i;
    ++self.j;
    ++self.k;

    // might want to have it also increment l/m if derived
    
    return temp;
  }

  // Deduce as copy
  template <typename Self>
  Self CopyAndFlipSelf(this Self self)
  {
    std::swap(self.i, self.k);
    return self;
  }

  // Derived only func
  template <IsDerived Self>
  void DerivedOnlyFunc(this Self& self)
  {
    std::cout << "Derived only func\n";
  }

  // Conversion as this (defined later to be below OtherBase)
  int ConvertToN(this OtherBase otherBase);

  
  // ----------------- Structured Bindings -----------------

  // get overload for structured bindings
  // for the equivalent func:
  //   int GetIndex(int i) const;
  // it can only be called by r-value references
  // but this one can be called by both l-value and r-value references
  // it will convert the type to the right value so constness is preserved
  // so instead of writing two functions or const_cast, we can just write one
  template <size_t I, typename Self>
  auto get(this Self&& self)
  {
    if constexpr (I == 0) 
      return self.i;
    else if constexpr (I == 1)
      return self.j;
    else if constexpr (I == 2)
      return self.k;
    
    if constexpr (I == 3)
    {
      if constexpr (IsDerived1<Self>)
        return self.l;
      else if constexpr (IsDerived2<Self>) 
        return self.m;
    }
  }

  // consteval func to get the size of the tuple
  template <typename Self>
  consteval static size_t GetSize()
  {
    if constexpr (IsDerived1<Self> || IsDerived2<Self>)
      return 4;
    else
      return 3;
  }

};

namespace std 
{
  template <isBaseType BaseT>
  struct tuple_size<BaseT> : std::integral_constant<size_t, BaseT::template GetSize<BaseT>()> {};
  template <size_t I, isBaseType BaseT>
  struct tuple_element<I, BaseT> 
  {
    using type = decltype(BaseT{}.get<I>());
  };
}

// ----------------- Derived -----------------
struct Derived1 : Base 
{
  int l = 0;
};

struct Derived2 : Base 
{
  int m = 0;
};


struct OtherBase 
{
  int n = 0;

  OtherBase(Base base) 
  {
    n = base.i + base.j + base.k;
  }
};

int Base::ConvertToN(this OtherBase otherBase) 
{
  return otherBase.n;
}



int main() {
  std::cout << "Printing values with no virtual functions\n";

  Base base{1, 2, 3};
  base.PrintClass();
  base.PrintProperties();

  Derived1 derived{1, 2, 3, 4};
  derived.PrintClass();
  derived.PrintProperties();

  Derived2 derived2{1, 2, 3, 4};
  derived2.PrintClass();
  derived2.PrintProperties();

  std::cout << "Derived1 in Base*:\n";

  Base* derivedPtr = &derived;
  derivedPtr->PrintClass();

  std::cout << "\n\n";

  const Base& constBase = base;
  //base.get<0>() = 99;
  std::cout << "non-const: " << base.get<0>() << "\n";
  // std::get<0>(constBase) = 99; // compile error
  std::cout << "const: " << constBase.get<0>() << "\n";

  std::cout << std::tuple_size_v<Base> << " " << 
              typeid(std::tuple_element_t<0, Base>).name() << " " <<
              typeid(std::tuple_element_t<1, Base>).name() << " " <<
              typeid(std::tuple_element_t<2, Base>).name() << "\n";
  
  std::cout << std::tuple_size_v<Derived1> << " " << 
              typeid(std::tuple_element_t<0, Derived1>).name() << " " <<
              typeid(std::tuple_element_t<1, Derived1>).name() << " " <<
              typeid(std::tuple_element_t<2, Derived1>).name() << " " <<
              typeid(std::tuple_element_t<3, Derived1>).name() << "\n";

  // As of 12/12/2023 this causes an internal compiler error but it should work
  // https://developercommunity.visualstudio.com/t/Explicit-this-parameter-does-not-work-to/10405265
  // auto [i, j, k] = base;

  std::cout << "\nderived only func:\n";
  // base.DerivedOnlyFunc(); // compile error
  derived.DerivedOnlyFunc();
  derived2.DerivedOnlyFunc();

  std::cout << "\nconversion deducing this:\n";
  // conversion this
  std::cout << "base converted to n: " << base.ConvertToN() << "\n";


  std::cout << "\n\n";


  // This in lambda for recursive lambda
  std::cout << "factorial of 5: " << [](this auto self, int n) -> int {
    if (n == 0) {
      return 1;
    }
    return n * self(n - 1);
  }(5) << "\n";
}
