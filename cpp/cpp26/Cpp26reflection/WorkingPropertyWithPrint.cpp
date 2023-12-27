#include <experimental/meta>
#include <string>
#include <type_traits>
#include <iostream>


template <typename T>
using PrintFunc = void (*)(std::string_view, const T&);



template <typename T>
void DefualtPrint(std::string_view name, const T& val)
{
    std::cout << name << " = " << val << "\n";
}

template <typename T>
void OtherPrint(std::string_view name, const T& val)
{
    std::cout << "-> " << name << " = " << val << "\n";
}

template <typename T, PrintFunc<T> Func = DefualtPrint<T>>
using Property = T;

using Float = float;
struct FloatVec
{
    Property<float> x;
    Property<float, OtherPrint<Float>> y;
    Property<float, [](std::string_view name, const float& val) { std::cout << ">>> " << name << " = " << val << "\n"; }
    > z;
};

template <typename T>
void PrintClassNDSM(const T& value) {
  [:expand(std::meta::nonstatic_data_members_of(^T)):] >>
  [&]<auto e>{

    // Have to compare types asstrings because it crashes if I try to compare the info
    if constexpr (std::meta::name_of(std::meta::type_of(e)) == std::meta::name_of(^Property))
    {
        using MemberType = typename[:std::meta::type_of(e):];

        // Has to be a one liner because it isnt letting me split it up into variables or functions
        // gets the value of the second template argument of the property (PrintFunc) and calls it
        std::meta::value_of<PrintFunc<MemberType>>(std::meta::template_arguments_of(std::meta::type_of(e))[1])(std::meta::name_of(e), value.[:e:]);
    }
    else
    {
        DefualtPrint(std::meta::name_of(e), value.[:e:]);
    }
  };
}




int main()
{
    FloatVec vec{1, 2, 3};
    PrintClassNDSM(vec);
    
}