#include "reflectLib.hpp"
#include "json.hpp"
#include "imgui.h"
#include <ranges>

using namespace std::meta;

using json = nlohmann::ordered_json;



// ----------------Pre Declarations----------------
template <typename DataType>
void RunObjectImGuiEditorInterface(DataType& object)


// ----------------Property marking code----------------
template <typename DataType>
using ToJsonFunc = void (*)(json& j, const DataType&);

template <typename DataType>
using FromJsonFunc = void (*)(const json& j, DataType&);

template <typename DataType>
using EditorFunc = void (*)(DataType&);


template <typename DataType>
struct PropertyOptions
{
  // explicit serialize option 
  bool serialize = false;
  // explicit dontSerialize option
  bool dontSerialize = false;

  bool editor = true;

  ToJsonFunc<DataType> toJsonFunc = [](DataType dataType)
  {
    j[display_name_of(^dataType)] = dataType;
  };
  FromJsonFunc<DataType> fromJsonFunc = [](DataType dataType)
  {
    dataType = j.value(display_name_of(^dataType), dataType);
  };
  EditorFunc<DataType> editorFunc = RunObjectImGuiEditorInterface;
};

template<typename DataType, PropertyOptions<DataType> Options = PropertyOptions<DataType>{}>
using Property = DataType;

// can provide a function that either calls on the member or the member info
template <typename T, typename Arg>
consteval void RunFuncOnAllNSDM(T& object, std::function<void(Arg)>& func, std::function<bool(info)> conditionFunc = nullptr)
{
  template for (constexpr auto member : nonstatic_data_members_of(^T))
  {
    if constexpr (conditionFunc && !conditionFunc(member))
    {
      continue;
    }
    
    // if the type of Arg is info
    if constexpr (std::is_same_v<Arg, info>)
    {
      func(member);
    }
    else
    {
      func(object.[:member:]);
    }
  }
}

consteval auto GetPropertyOptions(const info& member)
{
  constexpr auto defaultOptions = PropertyOptions<typename[:type_of(member):]>{};

  // Has to be a template of Property
  if (template_of(type_of(member)) != ^Property)
  {
    return defaultOptions;
  }
  // Has to have 2 template arguments and the second one has to be PropertyOptions
  constexpr auto templateArgs = template_arguments_of(type_of(member));
  if constexpr (!(templateArgs.size() == 2 && templateArgs[1] == ^PropertyOptions>))
  {
    return defaultOptions;
  }
  return [:templateArgs[1]:];
}



// ------------------------------------------------
// -----------------Serialize Code-----------------
// ------------------------------------------------



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
  PropertyOptions options = GetPropertyOptions(member);
  if constexpr (options.serialize)
    return serializeFlags.serializeSerializeMembers;
    
  if constexpr (options.dontSerialize)
    return serializeFlags.serializeDontSerializeMembers;

  return serializeFlags.serializeNormalMembers;
}


// ----------------Serialize Run Code----------------
template <SerializeFlags serializeFlags, typename T>
void ToJsonAllMembers(json& j, const T& object)
{
  RunFuncOnAllNSDM(object, [&](auto& member)
  {
    if constexpr (GetPropertyOptions(member))
    {
      template_arguments_of(typeof(member))[1].toJsonFunc(j, object.[:member:]);
    }
    else
    {
      DefaultToJson(j, object.[:member:]);
    }
  }, SerializeConditionFunc<serializeFlags>);
}

template <SerializeFlags serializeFlags, typename T>
void FromJsonAllMembers(const json& j, T& object)
{
  RunFuncOnAllNSDM(object, [&](auto& member)
  {
    if constexpr (GetPropertyOptions(member))
    {
      template_arguments_of(typeof(member))[1].fromJsonFunc(j, object.[:member:]);
    }
    else
    {
      DefaultFromJson(j, object.[:member:]);
    }
  }, SerializeConditionFunc<serializeFlags>);
}

// ----------------Serialize Macros----------------
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


// -----------------------------------------------
// ------------------Editor Code------------------
// -----------------------------------------------
template <typename T>
consteval ImGuiDataType MapTypeToImGuiDataType() 
{
  if constexpr (std::is_same_v<T, std::uint8_t>)
    return ImGuiDataType_U8;
  else if constexpr (std::is_same_v<T, std::int8_t>)
    return ImGuiDataType_S8;
  else if constexpr (std::is_same_v<T, std::uint16_t>)
    return ImGuiDataType_U16;
  else if constexpr (std::is_same_v<T, std::int16_t>)
    return ImGuiDataType_S16;
  else if constexpr (std::is_same_v<T, std::uint32_t>)
    return ImGuiDataType_U32;
  else if constexpr (std::is_same_v<T, std::int32_t>)
    return ImGuiDataType_S32;
  else if constexpr (std::is_same_v<T, std::uint64_t>)
    return ImGuiDataType_U64;
  else if constexpr (std::is_same_v<T, std::int64_t>)
    return ImGuiDataType_S64;
  else if constexpr (std::is_same_v<T, float>)
    return ImGuiDataType_Float;
  else if constexpr (std::is_same_v<T, double>)
    return ImGuiDataType_Double;

  return ImGuiDataType_COUNT;  // Default case, you can handle this appropriately
}

template <typename T>
concept Iterable = requires(T t) 
{
  std::begin(t); // must have a begin function
  std::end(t);   // must have an end function
};

struct EditorProperties
{
  bool isOpen = false;
};

// might need to split into a bool and this one for if constexpr
template <typename DataType>
constexpr std::optional<EditorProperties&> GetEditorProperties(DataType& object)
{
  template for (constexpr auto member : members(^T, is_static, is_variable))
  {
    if constexpr (type_of(member) == ^EditorProperties)
    {
      return object.[:member:];
    }
  }
  return std::nullopt;
}

template <typename DataType>
void DefaultRangeInterface(DataType& object, std::string_view name)
{
  if constexpr(std::optional<EditorProperties&> properties = GetEditorProperties(object))
  {
    if (properties->isOpen)
    {
      ImGui::SetNextItemOpen(true);
    }
  }

  bool open = ImGui::TreeNodeEx(name.data(), ImGuiTreeNodeFlags_AllowItemOverlap);
  // TODO: Add drop down bar cases
  
  // If not open return
  if (!open) return;

  int i = 0;
  for (auto& val : object)
  {
    // TODO: Add drop down start cases
    RunImGuiValueInterface(val);
  }
  
  ImGui::TreePop();
}


template <typename DataType>
void RunImGuiValueInterface(DataType& object, info member)
{
  // If it is an object with an editor properties
  if constexpr (GetEditorProperties(object))
  {
    RunFuncOnAllNSDM cv()
  }

  // If it is a range call the range interface
  if constexpr (Iterable<DataType>)
  {
    DefaultRangeInterface(val, );
    return;
  }

  // For all not specialized (std::string, bool, int, float, etc.)
  if (is_variable(^object))
  {
    ImGui::Text(display_name_of(^object).c_str());
  }

  constexpr std::string imGuiName = std::string("##") + display_name_of(^T);
  constexpr ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
  constexpr ImGuiDataType dataType = MapTypeToImGuiDataType<DataType>();
  if constexpr (dataType != ImGuiDataType_COUNT)
  {
    ImGui::InputScalar(imGuiName.c_str(), ScalarInput(dataType), &val, nullptr, nullptr, nullptr, flags)
  }
  else if constexpr (std::is_same_v<DataType, std::string>)
  {
    ImGui::InputText(imGuiName.c_str(), &val, flags)
  }
  else if constexpr (std::is_same_v<DataType, bool>)
  {
    ImGui::SameLine();
    ImGui::Checkbox(imGuiName.c_str(), &val)
  }
}




template <typename DataType>
void RunObjectImGuiEditorInterface(DataType& object)
{
  // If it is an object with an editor properties
  if constexpr (GetEditorProperties(object))
  {
    RunFuncOnAllNSDM(object, [&](info member)
    {
      if constexpr (is_variable(member))
      {
        RunImGuiValueInterface(object, member);
      }
    });
  }
}
