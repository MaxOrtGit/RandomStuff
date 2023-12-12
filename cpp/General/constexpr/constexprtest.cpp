#include <array>
#include <iostream>

consteval auto makeArray()
{
  constexpr std::array v{1, 2, 3};
  std::array<int, v.size()> arr;
  for (int i = 0; i < 3; ++i)
    arr[i] = i;
  return arr;
}

template <typename... Ts> 
struct MyTuple
{
  MyTuple(Ts const& ..._)
  {

  }
};

struct Flags
{
  bool a = false;
  bool b = false;
  bool c = false;

  template <typename derived>
  auto tst(this derived const& der)
  {
    return der.d;
  }
};

struct DerivedFlags : Flags
{
  bool d = false;
};

template <typename T, Flags f>
struct Option
{
  static constexpr Flags flags = f;
};

struct Args
{
  Option<int, Flags{.a = true, .b = true}> a;
};

int main()
{
  std::cout << makeArray()[2] << std::endl;

  MyTuple t{1, 2, 3};
  Flags f{.b = true, .c = true};
  DerivedFlags df{};
  std::cout << df.tst() << std::endl;
  std::cout << sizeof(Flags) << std::endl;
  std::cout << sizeof(DerivedFlags) << std::endl;

  Args args{};
  auto const& a = args.a;
  std::cout << a.flags.a << std::endl;
}
