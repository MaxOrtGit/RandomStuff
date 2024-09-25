// You can run the code here https://godbolt.org/z/7zq1je6jT
// No warnings or errors as of Sep 25, 2024 with EDG's reflection compiler

#include <experimental/meta>
#include <nlohmann/json.hpp>
#include <optional>
#include <iostream>

// ---------- Namespaces ----------
namespace meta = std::meta;
using json = nlohmann::ordered_json;


// ---------- Macros ----------
// Because inheritance changes the default constructor and metaclasses arn't
//   in the langauge I am creatting a macro similar to the built in ones
#define NLOHMANN_DEFINE_TYPE_REFLECT(cls) \
friend void to_json(json&, const cls&); \
friend void from_json(const json&, cls&); \
}; /* End class scope */ \
consteval { CreateJSONFunctions<cls>(); 
// Intential dangling so it uses end of class



// ---------- Helpers ----------
// Gets the first instance of class type from attributes
template <typename Attribute, meta::info info>
constexpr std::optional<Attribute> GetAttribute()
{
  std::optional<Attribute> val = std::nullopt;

  // Loops through all attributes
  [:expand(meta::annotations_of(info)):] >> [&]<auto ann>
  {
    // Skip to end if already found
    if (val) return;

    // If attribute of given type set to val
    if constexpr (meta::type_of(ann) == ^^Attribute)
    {
      val = meta::extract<Attribute>(ann);
    }
  };
  return val;
}

// Used in contexts there the given Attribute may not be constructable
template <typename Attribute, meta::info info>
constexpr auto GetAttributeConstexpr()
{
  // If no default constructor return an optional<nullptr_t>
  if constexpr (!IsConstexprConstructable<Attribute>)
    return static_cast<std::optional<std::nullptr_t>>(std::nullopt);
  else // return the given type
    return GetAttribute<Attribute, info>();
}


// Used for checking if class is constexpr constructable
// Mainly for constexpr Default attribute
template<auto t> 
using isConstexprHelper = void;
template<typename T>
concept IsConstexprConstructable = requires {
    // T::count must be usable in a context that requires a constant expression
    typename isConstexprHelper<T{}>;
};

// Concepts for checking if it can be json assignable
template<class T>
concept IsJSONAssignable = requires(json& j, const T& obj) {
  j = obj;
};
template<class T>
concept IsJSONGetable = requires(json& j) {
  j.get<T>();
};


// ---------- Attributes ----------

// Renaming the json member
struct Name
{
  std::string_view name;
};

// Skips the toJson or FromJson parsing of member
struct Skip 
{
  bool skipToJSON = true;
  bool skipFromJSON = true;
};

template <typename T = void>
struct Default;

// Defaults to given value
// T must be constexpr in this case
template <typename T>
struct Default 
{
  T value = T{};
};

// Default to default value
template <>
struct Default<void> {};


template <typename T>
using DefaultGetterFunc = T (*)();

// Default for non-costexpr values
template <typename T>
struct DefaultGetter 
{
  DefaultGetterFunc<T> getter = []() -> T
  {
    return T{};
  };
};


template <typename T>
using ToJsonFunc = void (*)(json&, const T&);
template <typename T>
using FromJsonFunc = void (*)(const json&, T&);

// Overrides json function
template <typename MemberType>
struct Override
{
  ToJsonFunc<MemberType> to_json = nullptr;
  FromJsonFunc<MemberType> from_json = nullptr;
};


// ---------- Predefinitions/Classes ----------

// Info for generating the functions
// Default for use with the macro
struct JsonFuncInfo
{
  // If parameters name will be reflected
  std::string_view jsonName = "j";
  std::string_view objName = "obj";
  // If you want to "wrap" the value
  std::string_view toJsonName = "to_json";
  std::string_view fromJsonName = "from_json";
};


// ---------- Get code for members ----------


// ---------- To JSON ----------

// Parses attribute for to json info
template <meta::info mem, typename MemberType>
struct ToJSONMemberInfo
{
  // For different name in json
  std::string_view name;

  // For excluding from print
  bool skip = false;

  // Function for to json (default is j = obj)
  ToJsonFunc<MemberType> func = [](json& memberJ, const MemberType& obj)
  {
    // Check if actually json assignable
    if constexpr (IsJSONAssignable<MemberType>)
      memberJ = obj;
    else
      // Should be replaced with a throw. Reason why is because of compile warning.
      std::cout << "Cannot convert member to json. Skip member of type " 
                << meta::identifier_of(^^MemberType) << "\n";
  };
  
  // Constructor to set values
  constexpr ToJSONMemberInfo()
  {
    // Get relevent attributes
    constexpr auto nameAttr = GetAttribute<Name, mem>();
    constexpr auto skipAttr = GetAttribute<Skip, mem>();
    constexpr auto overrideAttr = GetAttribute<Override<MemberType>, mem>();

    // Set name                        
    name = nameAttr.value_or(Name{meta::identifier_of(mem)}).name;

    // Set skiped
    if (skipAttr && skipAttr.value().skipToJSON)
    {
      skip = true;
      if (overrideAttr && overrideAttr.value().to_json)
      {
        // Log overriding error
      }
      return;
    }

    // Get override function
    if (overrideAttr && overrideAttr.value().to_json)
    {
      func = overrideAttr.value().to_json;
    }
  } 
};

// Function to get the tokens for a member's to json
template <meta::info mem>
consteval meta::info GetMemberToJSON(JsonFuncInfo jsonFuncInfo)
{
  // Get member type and to json info
  using MemberType = typename[:meta::type_of(mem):];
  constexpr ToJSONMemberInfo<mem, MemberType> toJsonInfo{};

  // Skip the member if Skip and skipToJSON
  if constexpr (toJsonInfo.skip) 
  {
    // ``
    return ^^{};
  } 
  else
  {
    // `toJSONFunc(j["mem"], obj.mem);`
    return ^^{
      \(toJsonInfo.func)(\id(jsonFuncInfo.jsonName)[\(toJsonInfo.name)], \id(jsonFuncInfo.objName).[:\(mem):]);
    };
  }
}


// ---------- From JSON ----------

// Parses attribute for to json info
template <meta::info mem, typename MemberType>
struct FromJSONMemberInfo
{
  // For different name in json
  std::string_view name;

  // For excluding from print
  bool skip = false;

  // Function for from json (default is obj = j.get<Obj>())
  FromJsonFunc<MemberType> func = [](const json& memberJ, MemberType& obj)
  {
    // Check if actually json convertable
    if constexpr (IsJSONGetable<MemberType>)
      obj = memberJ.get<MemberType>();
    else
      // Should be replaced with a throw. Reason why is because of compile warning.
      std::cout << "Cannot convert member from json. Skip member of type " 
                << meta::identifier_of(^^MemberType) << "\n";
  };

  // If is defaulted
  bool defaulted = false;

  // Constructor to set values
  constexpr FromJSONMemberInfo()
  {
    // Get relevent attributes
    constexpr auto nameAttr = GetAttribute<Name, mem>();
    constexpr auto skipAttr = GetAttribute<Skip, mem>(); 
    constexpr auto overrideAttr = GetAttribute<Override<MemberType>, mem>();
    constexpr auto defaultVoidAttr = GetAttribute<Default<void>, mem>();
    constexpr auto defaultAttr = GetAttributeConstexpr<Default<MemberType>, mem>();
    constexpr auto defaultGetterAttr = GetAttribute<DefaultGetter<MemberType>, mem>();

    // Set the name
    name = nameAttr.value_or(Name{meta::identifier_of(mem)}).name;

    // Check if skipped
    if (skipAttr && skipAttr.value().skipFromJSON)
    {
      skip = true;
      if ((overrideAttr && overrideAttr.value().from_json) || defaultVoidAttr || defaultAttr)
      {
        // Log overriding error
      }
      return;
    }
  
    // Check if overridden 
    if (overrideAttr && overrideAttr.value().from_json)
    {
      func = overrideAttr.value().from_json;
    }

    // Check if defaulted
    if (defaultAttr || defaultVoidAttr || defaultGetterAttr)
    {
      defaulted = true;
      
      // Is more than one set
      if (static_cast<bool>(defaultAttr) + 
          static_cast<bool>(defaultVoidAttr) +
          static_cast<bool>(defaultGetterAttr) > 1
         )
      {
        // Log overriding error
      }
    }
  } 
};

// Function to get the tokens for a member's from json
template <meta::info mem, typename ParentType>
consteval meta::info GetMemberFromJSON(JsonFuncInfo jsonFuncInfo)
{
  // Get member type and info
  using MemberType = typename[:meta::type_of(mem):];
  constexpr FromJSONMemberInfo<mem, MemberType> fromJsonInfo{};


  // Skip the member if Skip and skipFromJSON
  if constexpr (fromJsonInfo.skip) 
  {
    // ``
    return ^^{};
  }
  // If no default attribute
  else if constexpr (!fromJsonInfo.defaulted)
  {
  // `fromJsonFunc(json["mem"], obj.mem); `
    return ^^{
      \(fromJsonInfo.func)(\id(jsonFuncInfo.jsonName)[\(fromJsonInfo.name)], \id(jsonFuncInfo.objName).[:\(mem):]);
    };
  }
  else // Has default atrribute
  {
    // Because json is reset this is equivelent to get from parent
    meta::info defaultValTokens = ^^{};

    // If the default is a given value
    if constexpr (constexpr auto defaultAttr = 
                  GetAttributeConstexpr<Default<MemberType>, mem>())
    {
      defaultValTokens = ^^{
        \id(jsonFuncInfo.objName).[:\(mem):] = \(defaultAttr.value().value);
      };
    }
    // If the default is a getter
    else if constexpr (constexpr auto defaultGetterAttr = 
                       GetAttribute<DefaultGetter<MemberType>, mem>())
    {
      defaultValTokens = ^^{
        \id(jsonFuncInfo.objName).[:\(mem):] = \(defaultGetterAttr.value().getter)();
      };
    }

    // Mimics .value
    
    /* ``` 
    auto it = json.find("mem");
    if (it != json.end)
    {
      fromJsonFunc(*it, obj.mem);
    }
    else
    {
      obj.mem = defaultVal; // or nothing
    }
    ``` */
    return ^^{
      {
        auto it = \id(jsonFuncInfo.jsonName).find(\(fromJsonInfo.name));
        
        if (it != \id(jsonFuncInfo.jsonName).end())
        {
          \(fromJsonInfo.func)(*it, \id(jsonFuncInfo.objName).[:\(mem):]);
        }
        else
        {
          \tokens(defaultValTokens)
        }
      }
    };
  }
}


// ---------- Generate functions ----------
// For getting the tokens for just one function
enum class JsonFuncType
{
  ToJson,
  FromJson
};

// Get the tokens for the json function
template <typename Object>
consteval meta::info GetJSONFunc(JsonFuncInfo jsonFuncInfo, JsonFuncType type)
{
  // Get the function name
  bool isToJson = type == JsonFuncType::ToJson;
  std::string_view funcName = isToJson ? jsonFuncInfo.toJsonName : jsonFuncInfo.fromJsonName;

  // Set the right parameter to const
  meta::info json_const = ^^{ const };
  meta::info obj_const = ^^{};
  if (isToJson)
    std::swap(json_const, obj_const);

  
  // Get type of object
  constexpr meta::info obj = ^^Object;

  // Create a function body
  meta::info body = ^^{};

  // Add each member's to/from json tokens
  [:expand(meta::nonstatic_data_members_of(obj)):] >> [&]<auto member>
  {
    auto func = isToJson ? GetMemberToJSON<member> : GetMemberFromJSON<member, Object>;
    body = ^^{
      \tokens(body)
      \tokens(func(jsonFuncInfo))
    };
  };
    
  // Get the name of the class
  std::string_view className = meta::identifier_of(obj);

  // Create the function and add the body to the middle
  return ^^{
    void \id(funcName)(
      \tokens(json_const) json& \id(jsonFuncInfo.jsonName), 
      \tokens(obj_const) \id(className)& \id(jsonFuncInfo.objName))
    {
      \tokens(body)
    }
  };
}

// Injects the to_json and from_json functions into the code (non friend)
template <typename Object>
consteval void CreateJSONFunctions(JsonFuncInfo jsonFuncInfo = JsonFuncInfo{}) 
{
  // Inject the to and from json functions
  queue_injection(GetJSONFunc<Object>(jsonFuncInfo, JsonFuncType::ToJson));
  queue_injection(GetJSONFunc<Object>(jsonFuncInfo, JsonFuncType::FromJson));
}



// -----------------------------------------------------------------------
// ------------------------------ User Code ------------------------------
// -----------------------------------------------------------------------

#include <iostream>

struct Obj
{
  // Required or modern json will give an error
  int normal = 0;


  // If not provided it will be variables default
  [[=Default()]]
  int optional = 1;
  
  // You can have it have a different name with the Name attribute
  [[=Name("Different Name")]]
  [[=Default()]]
  int differentName = 0;

  // If not provided it will be provided value
  [[=Default(false)]]
  bool differentDefault = true;


  // You can exlude from to/from json with Skip
  [[=Skip{.skipToJSON=true, .skipFromJSON=false}]]
  [[=Default()]]
  int authToken = 0;

  // You can override the to/from json functions for custom types/edge cases
  // In this case we make a special case for optional
  [[=Override<std::optional<int>>{
    .to_json = [](json& j, const std::optional<int>& opt)
    {
      if (opt)
        j = opt.value();
      else
        j = nullptr;
    },
    .from_json = [](const json& j, std::optional<int>& opt)
    {
      if (j.is_null())
        opt = std::nullopt;
      else
        opt = j.get<int>();
    }
  }]]
  [[=Default()]]
  std::optional<int> opt;

  // For non constexpr values you have to set the default with a getter 
  //   (can override getter func too)
  [[=DefaultGetter<std::vector<int>>()]]
  std::vector<int> vec{};
  
  
  // Simple print for comparing
  friend std::ostream& operator<<(std::ostream& os, const Obj& obj);
  

  // Adds to_json and from_json function
  // Needs to be at the end
  NLOHMANN_DEFINE_TYPE_REFLECT(Obj)
};


int main() 
{
  // Create default object and parse to json
  Obj obj{};
  std::cout << obj << "\nConvert to\n";
  json j = obj;
  std::cout << "json " << j.dump(2) << "\n\n\n";
  
  // Parse from json
  j = {{"normal", -1}, {"authToken", 999}, {"opt", 100}};
  std::cout << "json " << j.dump(2) << "\nConvert to\n";
  obj = j;
  std::cout << obj << "\n";
}


// Console output:
/*
Obj {
  normal: 0
  optional: 1
  differentName: 0
  differentDefault: true
  authToken: 0
  opt: null
  vec: []
}
Convert to
json {
  "normal": 0,
  "optional": 1,
  "Different Name": 0,
  "differentDefault": true,
  "opt": null,
  "vec": []
}


json {
  "normal": -1,
  "authToken": 999,
  "opt": 100
}
Convert to
Obj {
  normal: -1
  optional: 1
  differentName: 0
  differentDefault: false
  authToken: 999
  opt: 100
  vec: []
}
*/





// Quick print for Obj
std::ostream& operator<<(std::ostream& os, const Obj& obj)
{
  // Print bools as text
  os << std::boolalpha << "Obj {\n";
  // Print all members
  [:expand(meta::nonstatic_data_members_of(^^Obj)):] >> [&]<auto member>
  {
    // If type is vector
    if constexpr (meta::type_of(member) == ^^std::vector<int>)
    {
      os << "  " << meta::identifier_of(member) << ": [";
      for (const auto& val : obj.[:member:])
        os << val << ", ";
      os << "]\n";
    } 
    // If type is optional
    else if constexpr (meta::type_of(member) == ^^std::optional<int>)
    {
      if (obj.[:member:])
        os << "  " << meta::identifier_of(member) << ": " << obj.[:member:].value() << "\n";
      else
        os << "  " << meta::identifier_of(member) << ": null\n";
    } 
    else // Just print the member
    {
      os << "  " << meta::identifier_of(member) << ": " << obj.[:member:] << "\n";
    }
  };
  os << "}";
  return os;
}

// Main above this function
