#include <experimental/meta>
#include <string>
#include <type_traits>
#include <iostream>
#include <optional>

// Design explanations:
// This is bad and I know it is. This is more of an example this it is "possible".
//
// Many times I pass in the std::meta::info of a type insead of the type, 
//  that is because it dealiases it if passed as a template arg.
//  Also because of this you cant look at the attributes within the Print function.
//   A work around could be having the Print function be templated with the type std::meta::info instead.
//
// Would use std::format but there are issues in the header
//
// I also don't use inheritance because it messes with the constructor so you have to do FloatVec2{{}, x, y}
//
// Issues I ran into:
// Constexpr members dealias the type so I can't put an attributer on it (in general keeping the alias of a type is walking on nails)
// Comparisons are not very stable and I get tons of "Compilation" aborted. The follow code does not work:
//   static_assert(^int == ^int);
// name_of doesn't write of the name of the type many times and the best way to actually figure out what a 
//  meta::info is to send it as a template to a function that throws and read what the error mesage names it
//  



// --------------- Utils ---------------

namespace meta = std::meta;

// --------- Enum Utils ---------

template <typename T>
concept EnumType = std::is_enum_v<T>;

template<EnumType E>
constexpr std::string enum_to_string(E value) {
  std::string result = "<unnamed>";
  [:expand(std::meta::enumerators_of(^E)):] >>
  [&]<auto e>{
    if (value == [:e:]) {
      result = meta::name_of(e);
    }
  };
  return result;
}


// --------- Formatting ---------

// Tab counter for formatting
static int tabCount = 0;

void PrintTabs()
{
  for (int i = 0; i < tabCount; i++)
    std::cout << "  ";
}


// --------- Pre Definitons ---------
template <typename T>
void Print(std::string_view name, const T& value);

// --------- Reflection Helpers ---------

// Dealiases and removes const and refs
template <meta::info T>
constexpr auto CleanInfo() 
{
  return meta::dealias(std::meta::substitute(^std::remove_cvref_t, {T}));
}

// Gets the static member with the type of the given VariableType class
// There is no way to test if constexpr
// dealiases both T and the type_of members (so it doesnt conflict with Attributer)
template <typename VariableType, typename T>
consteval std::optional<VariableType> GetConstexprVariable()
{
  // Need to check if T it is a class because meta::members_of wouldn't compile
  if constexpr (!std::is_class<T>())
  {
    return std::nullopt;
  }
  else
  {
    std::optional<VariableType> out;
    // For all static variables 
    [:expand(meta::members_of(^T)):] >>
    [&]<auto member>
    {
      // If first instance already found quit
      if (out)
        return;

      // Filters don't currently work with the expand statement
      if constexpr(!(meta::has_static_storage_duration(member) && meta::is_variable(member)))
        return;
        

      // Static inline also pass this but I dont know how to fix it
      // I feel there should be an is_constexpr to avoid this down the line
      else if constexpr(CleanInfo<meta::type_of(member)>()
                       //== clean<^VariableType>()) // causes Compilation aborted for some reason
                        == meta::dealias(std::meta::substitute(^std::remove_cvref_t, {^VariableType})))
      {
        // Return the value within the constexpr variable
        out = meta::value_of<VariableType>(member);
      }
    };
    return out;
  }
}



// --------------- Attributes ---------------

// Attributes are added by puting them in the template parameter
template <typename T, auto... Attributes>
using Attributer = T;

// Gets the Attribute of the given type in the template args of typeInfo
// TypeInfo Needs to be passed as an info because template dealiases
// Dealiases attributes
template <typename Attribute, meta::info typeInfo>
consteval std::optional<Attribute> GetAttribute()
{
  // Checks if it is a template
  // It can be any template so we can alias it
  if constexpr (meta::has_template_arguments(typeInfo))// && meta::template_of(typeInfo) == ^Attributer)
  {
    // Look through the template args
    for (auto arg : meta::template_arguments_of(typeInfo)) // std::views::drop(1) should be used but there is an issue with the ranges header
    {
      // If the type is the same as Attribute
      if (!meta::is_type(arg) && meta::dealias(meta::type_of(arg)) == ^Attribute)
      {
        // Return the value within the argument
        return meta::value_of<Attribute>(arg);
      }
    }
  }
  return std::nullopt;
}

// --------- Custom Print Attribute ---------

// std::function does not work in a template
template <typename T>
using PrintFunc = void (*)(std::string_view, const T&);

// T is the class the attribute is applied on because the func needs T as a parameter
template <typename T>
struct PrintAttribute
{
  PrintFunc<T> func = nullptr;
  bool explicitPrint = false;
  bool explicitDontPrint = false;
};

// Takes in the member info and returns the PrintAttribute struct in the template args.
// If tag not found it returns the default PrintAttribute for the given class
// Sets the func to the default print if not found
template <meta::info memberInfo>
consteval auto GetPrintAttribute()// -> PrintAttribute<typename[:meta::type_of(memberInfo):]>
{
  // Set T to the type of the member
  using T = typename[:meta::type_of(memberInfo):];

  PrintAttribute<T> attribute = GetAttribute<PrintAttribute<T>, meta::type_of(memberInfo)>().value_or(PrintAttribute<T>{});
  
  // Set the func to the default Print<T> if it is null
  if (!attribute.func) attribute.func = Print<T>;

  return attribute;
}

// --------- "Aliasing" attribute ---------
// I havent investigated this side too much but because we need to template the attribute we cant just use ues a varibale
template <typename T> 
consteval auto CustomPrintAttribute(PrintFunc<T> func)
{
  return PrintAttribute<T> {
      .func=func, .explicitPrint=true
    };
}

// --------- Aliasing Attributer ---------
// Explanation why the syntax bad:
// Attributers can get LOOOOOONG and aliasing them is not great
//  like here is the minimum example for just not printing an int:
  Attributer<int, PrintAttribute<int>{.explicitDontPrint=true}> hidden;
//  It would be nice if I could do:
  template <typename T> 
  using SkipBroken = Attributer<T, PrintAttribute<T>{.explicitDontPrint=true}>;
//  But because we get Attributes from the template arguments of an alias and std::meta::dealias
//   is recusive there would be no ways to get the args
//  So we have to have the Tags be within the template arg list as defaults
  template <typename T, auto PA = PrintAttribute<T>{.explicitDontPrint=true}> 
  using Skip = T; // doesn't need to alias Attributer because it does nothing
// But now we cant pass in any additional Tags 

// We can also use constexpr functions 
template <typename T, PrintFunc<T> Func, auto PA = CustomPrintAttribute(Func)> 
using CustomPrint = T;

// --------------- Print Properties ---------------

// Can either be put into a class as any static constexpr variable that will be found at run time
// Or you can overload the struct for a given class/concept but if you don't 
//  overload func you will need to copy the variables
template <typename T>
struct PrintProperties
{
  PrintFunc<T> func = nullptr;
  bool printPublic = true;
  bool printPrivate = false; // includes protected

  bool printNSDM = true;
  bool printStatic = false;
};

// Uses GetConstexprVariable to find the PrintProperies
// Dealiases
template <typename T>
consteval PrintProperties<T> GetPrintProperties()
{
  // I wanted a way to get a constexpr static variable from the class (GetConstexprVariable)
  // But it just wouldnt work so I am just returning the code below
  // If none are found it returns the default. So you can overload the default for a class/concept
  return GetConstexprVariable<PrintProperties<T>, T>().value_or(PrintProperties<T>{});
}

// --------------- Print Code ---------------

// Decides if a member should be printed
template <meta::info member, typename T, typename PA>
consteval bool DoPrint(PrintProperties<T> properties, const PA& attribute)
  requires (meta::template_of(^PA) == ^PrintAttribute)
{
  // If Explicit print
  if (attribute.explicitPrint)
    return true;

  // If Explicit don't print
  else if (attribute.explicitDontPrint)
  {
    return false;
  }

  // If nsdm excluded 
  else if (meta::is_nsdm(member) && !properties.printNSDM)
    return false;

  // If static variables excluded 
  else if (meta::is_variable(member) && meta::has_static_storage_duration(member) && !properties.printStatic)
    return false;
  
  // Return if visibility should be printed 
  return (meta::is_public(member) && properties.printPublic ) || 
        (meta::is_private(member) || meta::is_protected(member) && properties.printPrivate);
}

// Default print
template <typename T>
void Print(std::string_view name, const T& value) 
{
  // Get the print properties of the class
  static constexpr PrintProperties<T> properties = GetPrintProperties<T>();

  // If an explicit print function is given call that
  if constexpr(properties.func)
  {
    properties.func(name, value);
  }
  // If it can be couted just cout
  else if constexpr (requires (T v) {std::cout << v;})
  {
    std::cout << name << ": " << value << "\n";
  }
  else // Cout each member in the class
  {
    // Print name given (variable name)
    std::cout << name << ":\n";

    tabCount++; // for PrintTabs and formatting

    // For all members in the class
    [:expand(meta::members_of(^T)):] >>
    [&]<auto member>
    {
      // If it is not a variable skip (is_variable doesn't include nsdm)
      if constexpr (!meta::is_nsdm(member) && !meta::is_variable(member))
      {
        return;
      }
      else
      {
        // Get the print attribute of the member
        constexpr auto attribute = GetPrintAttribute<member>();
        
        // If the given member should be printed
        if constexpr (DoPrint<member>(properties, attribute))
        {
          PrintTabs();
          attribute.func(meta::name_of(member), value.[:member:]);
        }
      }
    };
    tabCount--;
  }
}

// Example overide PrintFunc for bool
template <>
struct PrintProperties<bool>
{
  PrintFunc<bool> func = [](std::string_view name, const bool& value)
  {
    std::cout << name << ": " << (value ? "true" : "false") << "\n";
  };
};

// SeperatePrint function to put within an attribute
template <typename T>
void OtherPrint(std::string_view name, const T& value)
{
  std::cout << name << " -> " << value << "\n";
}

// --------------- Examples ---------------

// --------- Simple ---------
struct FloatVec2
{
  float x;
  float y;
};

void SimpleMain()
{
  std::cout << "Struct Print:\n";
  // Not sure if we can avoid passing a string
  FloatVec2 vec2{1, 2};
  Print(meta::name_of(^vec2), vec2);
}


// --------- Complicated ---------
struct Complex
{
public: // By default all public members are printed
  // Default print variable
  float f;

  // Attributer with print attribute
  Attributer<float, PrintAttribute<float>{.func = OtherPrint}> f2;

  // Attributer alias with a lambda
  CustomPrint<float, 
  [](std::string_view name, const float& value) {
    std::cout << name << " >>> " << value << "\n"; 
  }> f3;

  // Class print
  FloatVec2 vec;
    
  // Explicitly not print (alias of attribute)
  Skip<int> hidden;

private: // By default all private members aren't printed
  
  // Because we need to access the private member below we must make Print a friend
  template <typename T>
  friend void Print(std::string_view, const T&);

  // Will be printed because we set the properties below
  static inline int globalEditCounter = 0;

  // Attributers dont work on constexpr so setting is below(gets dealiased for some reason)
  // Setting the printProperties within the class
  //  we could do this outside of the class like with bool/enum but because func is nullptr we
  //  would need to recreate all the variables
  //static constexpr Skip<PrintProperties<Complex>> arbitraryName{.printPrivate = true, .printStatic = true};

public:
  Complex(float f, float f2, float f3, FloatVec2 vec) : f(f), f2(f2), f3(f3), vec(vec), hidden(1) {};
};

// Because of issues with constexpr and attributers I need to override
// I have to set all the variables
template <>
struct PrintProperties<Complex>
{
  PrintFunc<Complex> func = nullptr;
  bool printPublic = true;
  bool printPrivate = true;  // changed

  bool printNSDM = true;
  bool printStatic = true;
};


void ComplicatedMain()
{
  std::cout << "\nClass Print:\n";
  Complex comp{1, 2, 3, {4, 5}};
  Print(meta::name_of(^comp), comp);
}

// --------- Non-struct/outside code ---------
enum class Color {red, green, blue};

// Overide PrintFunc for Enums
template <EnumType T>
struct PrintProperties<T>
{
  PrintFunc<T> func = [](std::string_view name, const T& value)
  {
    std::cout << name << ": " << enum_to_string(value) << "\n";
  };
};

void EnumMain()
{
  std::cout << "\nEnum Print:\n";
  Color color = Color::red;
  Print(meta::name_of(^color), color);
}


int main()
{
  SimpleMain();

  ComplicatedMain();
  
  EnumMain();
}
