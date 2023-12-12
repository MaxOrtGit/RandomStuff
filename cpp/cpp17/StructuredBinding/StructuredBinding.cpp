#include <iostream>

// run with std >= c++20 for concepts

// ------------ Basic ------------
class Point {
private:
  int x;
  int y;

public:
  Point(int x, int y) : x(x), y(y) {}

  template <size_t I> auto get() const 
  {
    if constexpr (I == 0) 
      return x;
    else if constexpr (I == 1) 
      return y;
  }
};

namespace std {
  template <> struct tuple_size<Point> : std::integral_constant<size_t, 2> {};
  template <size_t I> struct tuple_element<I, Point> { using type = int; };
}

int Normal() 
{
  Point p{1, 2};
  auto [x, y] = p;
  std::cout << x << ", " << y << std::endl;
  std::cout << typeid(x).name() << ", " << typeid(y).name() << std::endl;
  return 0;
}

// ------------ Concepts ------------

template <typename T> 
concept PointLike = T::IsPoint && std::is_default_constructible_v<T> &&
requires(T t) {
  { t.get<0>() };
  { t.get<1>() };
};

class Point2 {
private:
  int x;
  float y;
public:
  static constexpr bool IsPoint = true;

  Point2() : x(0), y(0) {}
  Point2(int x, int y) : x(x), y(y) {}
  template <size_t I> 
  auto get() const 
  {
    if constexpr (I == 0)
      return x;
    else if constexpr (I == 1)
      return y;
  }
};

namespace std {
  template <PointLike T>
  struct tuple_size<T> : std::integral_constant<size_t, 2> {};
  template <size_t I, PointLike T>
  struct tuple_element<I, T> { using type = decltype(T{}.get<I>()); };
}

int Concept() 
{
  Point2 p{1, 2};
  auto [x, y] = p;
  std::cout << x << ", " << y << std::endl;
  std::cout << typeid(x).name() << ", " << typeid(y).name() << std::endl;
  return 0;
}

int main() {
  Normal();
  std::cout << "----------------" << std::endl;
  Concept();
  return 0;
}


