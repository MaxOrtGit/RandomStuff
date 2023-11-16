#include "reflectLib.hpp"
#include "json.hpp"
#include <iostream>
#include <random>
#include <functional>
#include <type_traits>

using namespace std::meta;

using json = nlohmann::ordered_json;

// ----------------Serializers----------------
template <typename DataType>
void DefaultToJson(json& j, const DataType& dataType)
{
  j[display_name_of(^dataType)] = dataType;
}
template <typename DataType>
void DefaultFromJson(const json& j, DataType& dataType)
{
  dataType = j.value(display_name_of(^dataType), dataType);
} 


// ----------------Serializing marking code----------------
template <typename DataType>
using ToJsonFunc = void (*)(json& j, const DataType&);

template <typename DataType>
using FromJsonFunc = void (*)(const json& j, DataType&);

template <typename DataType>
struct SerializeOptions
{
  ToJsonFunc<DataType> toJsonFunc = DefaultToJson;
  FromJsonFunc<DataType> fromJsonFunc = DefaultFromJson;
};

template<typename DataType, SerializeOptions<DataType> Options = SerializeOptions<DataType>{}>
using Serialize = DataType;

template<typename DataType>
using DontSerialize = DataType;

template <typename T>
consteval void RunFuncOnAllMembers(T& object, std::function<bool(info)> condition, auto& func)
{
  template for (constexpr auto member : nonstatic_data_members_of(^T))
  {
    if constexpr (!condition(member))
    {
      continue;
    }
    func(object.[:member:]);
  }
}


consteval bool IsSerialize(const info& member)
{
  // Has to be a template of Serialize
  if (template_of(member) != ^Serialize)
  {
    return false;
  }
  // Has to have 2 template arguments and the second one has to be SerializeOptions
  constexpr auto templateArgs = template_arguments_of(type_of(member));
  if constexpr (!(templateArgs.size() == 2 && templateArgs[1] == ^SerializeOptions>))
  {
    return false;
  }
}

consteval bool IsDontSerialize(const info& member)
{
  return template_of(member) == ^DontSerialize;
}



// ----------------Custom Serializers----------------
// json degrees store radians
constexpr SerializeOptions<float> RadianSerializeToDegrees
{
  .toJsonFunc = [](json& j, const float& radians)
  {
    j["temp"] = radians * 180 / 3.14159265358979323846;
  },
  .fromJsonFunc = [](const json& j, float& radians)
  {
    radians = j["temp"].get<float>() * 3.14159265358979323846 / 180;
  }
};

// ----------------Serialize flag Code----------------

struct SerializeFlags
{
  bool serializeNormalMembers = true;
  bool serializeSerializeMembers = true; // Should basically always be true
  bool serializeDontSerializeMembers = false;
};


template <SerializeFlags serializeFlags>
bool SerializeConditionFunc(info member)
{
  if constexpr (serializeFlags.serializeSerializeMembers)
    if (IsSerialize(member))
      return true;
  else
    if (IsSerialize(member))
      return false;

  if constexpr (serializeFlags.serializeDontSerialize)
    if (IsDontSerialize(member))
      return true;
  else
    if (IsDontSerialize(member))
      return false;
  
  return serializeFlags.serializeNormalMembers;
}


// ----------------Serialize Run Code----------------
template <SerializeFlags serializeFlags, typename T>
void ToJsonAllMembers(json& j, const T& object)
{
  RunFuncOnAllMembers(object, SerializeConditionFunc<serializeFlags>, [&](auto& member)
  {
    if constexpr (IsSerialize(member))
    {
      template_arguments_of(typeof(member))[1].toJsonFunc(j, object.[:member:]);
    }
    else
    {
      DefaultToJson(j, object.[:member:]);
    }
  });
}

template <SerializeFlags serializeFlags, typename T>
void FromJsonAllMembers(const json& j, T& object)
{
  RunFuncOnAllMembers(object, SerializeConditionFunc<serializeFlags>, [&](auto& member)
  {
    if constexpr (IsSerialize(member))
    {
      template_arguments_of(typeof(member))[1].fromJsonFunc(j, object.[:member:]);
    }
    else
    {
      DefaultFromJson(j, object.[:member:]);
    }
  });
}

#define JSON_SERIALIZE_AUTO_FLAGS(class, serializeFlags)\
friend void to_json(json& j, const class& object)\
{\
  ToJsonAllMembers<serializeFlags>(j, object);\
}\
friend void from_json(const json& j, class& object)\
{\
  FromJsonAllMembers<serializeFlags>(j, object);\
}
#define JSON_SERIALIZE_AUTO(class) JSON_SERIALIZE_AUTO_FLAGS(class, SerializeFlags{})


struct FloatVec2
{
  float x = 0;
  float y = 0;
  JSON_SERIALIZE_AUTO(FloatVec2)
};

struct Transform
{
  FloatVec2 position;
  FloatVec2 scale;
  Serialize<float, RadianSerializeToDegrees> rotation;

  DontSerialize<FloatVec2> privateVar;
  JSON_SERIALIZE_AUTO(Transform)
};

int main()
{
  
}
