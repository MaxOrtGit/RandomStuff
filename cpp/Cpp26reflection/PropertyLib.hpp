#include "reflectLib.hpp"
#include "json.hpp"
#include "imgui.h"
#include <ranges>

using json = nlohmann::ordered_json;


// ----------------Pre Declarations----------------
template <typename DataType>
void RunObjectImGuiEditorInterface(DataType& object);


// ----------------Property marking code----------------
template <typename DataType>
using ToJsonFunc = void (*)(json&, const DataType&);

template <typename DataType>
using FromJsonFunc = void (*)(const json&, DataType&);

template <typename DataType>
using EditorFunc = void (*)(DataType&, const std::meta::info&);


template <typename DataType>
struct PropertyOptions
{
  // explicit serialize option 
  bool serialize = false;
  // explicit dontSerialize option
  bool dontSerialize = false;

  // explicit editable option
  bool editable = true;
  // explicit nonEditable option
  bool nonEditable = true;

  ToJsonFunc<DataType> toJsonFunc = [](json& j, const DataType& dataType) constexpr
  {
    j[std::meta::display_name_of(^dataType)] = dataType;
  };
  FromJsonFunc<DataType> fromJsonFunc = [](const json& j, DataType& dataType) constexpr
  {
    dataType = j.value(std::meta::display_name_of(^dataType), dataType);
  };
  EditorFunc<DataType> editorFunc = nullptr;
};

template<typename DataType, PropertyOptions<DataType> Options = PropertyOptions<DataType>{}>
using Property = DataType;

// can provide a function that either calls on the member or the member info
template <typename T, typename Arg>
consteval void RunFuncOnAllNSDM(T& object, std::function<void(Arg)>& func, std::function<bool(info)> conditionFunc = nullptr)
{
  template for (constexpr auto member : std::meta::nonstatic_data_members_of(^T))
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
  constexpr auto defaultOptions = PropertyOptions<typename[:std::meta::type_of(member):]>{};

  // Has to be a template of Property
  if (std::meta::template_of(std::meta::type_of(member)) != ^Property)
  {
    return defaultOptions;
  }
  // Has to have 2 template arguments and the second one has to be PropertyOptions
  constexpr auto templateArgs = std::meta::template_arguments_of(std::meta::type_of(member));
  if constexpr (!(templateArgs.size() == 2 && templateArgs[1] == ^PropertyOptions>))
  {
    return defaultOptions;
  }
  return [:templateArgs[1]:];
}

struct GenerateProperties
{
  bool normalMembers = true;
  bool explicitTrueMembers = true;   // Should basically always be true
  bool explicitFalseMembers = false; // Should basically always be false
  
//TODO flags for public vs private
};


// ------------------------------------------------
// -----------------Serialize Code-----------------
// ------------------------------------------------

// ----------------Serialize property Code----------------
template <GenerateProperties serializeProperties>
bool SerializeConditionFunc(std::meta::info member)
{
  PropertyOptions options = GetPropertyOptions(member);
  if constexpr (options.serialize)
    return serializeProperties.explicitMembers;
    
  if constexpr (options.dontSerialize)
    return serializeProperties.explicitFalseMembers;

  return serializeProperties.normalMembers;
}


// ----------------Serialize Run Code----------------
template <GenerateProperties serializeProperties, typename T>
void ToJsonAllMembers(json& j, const T& object)
{
  RunFuncOnAllNSDM(object, [&](auto& member)
  {
    if constexpr (GetPropertyOptions(member))
    {
      std::meta::template_arguments_of(std::meta::typeof(member))[1].toJsonFunc(j, object.[:member:]);
    }
    else
    {
      DefaultToJson(j, object.[:member:]);
    }
  }, SerializeConditionFunc<serializeProperties>);
}

template <GenerateProperties serializeProperties, typename T>
void FromJsonAllMembers(const json& j, T& object)
{
  RunFuncOnAllNSDM(object, [&](auto& member)
  {
    if constexpr (GetPropertyOptions(member))
    {
      std::meta::template_arguments_of(std::meta::typeof(member))[1].fromJsonFunc(j, object.[:member:]);
    }
    else
    {
      DefaultFromJson(j, object.[:member:]);
    }
  }, SerializeConditionFunc<serializeProperties>);
}

// ----------------Serialize Macros----------------
#define JSON_SERIALIZE_AUTO_FLAGS(class, serializeProperties)\
friend void to_json(json& j, const class& object)\
{\
  ToJsonAllMembers<serializeProperties>(j, object);\
}\
friend void from_json(const json& j, class& object)\
{\
  FromJsonAllMembers<serializeProperties>(j, object);\
}
#define JSON_SERIALIZE_AUTO(class) JSON_SERIALIZE_AUTO_FLAGS(class, SerializeProperties{})


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

template <GenerateProperties editorProperties>
bool EditorConditionFunc(std::meta::info member)
{
  PropertyOptions options = GetPropertyOptions(member);
  if constexpr (options.editable)
    return editorProperties.explicitMembers;
    
  if constexpr (options.dontSerialize)
    return editorProperties.explicitFalseMembers;

  return editorProperties.normalMembers;
}

// ----------------Editor Functions----------------

// Default editor for all types
template <typename DataType>
struct EditorProperties
{
  // default EditorFunc is general for numeric and floating point types and gives a No editor implemented for other types
  static void EditorFunc(DataType& object, const std::meta::info& objectInfo)
  {
    // For all not specialized (std::string, bool, int, float, etc.)
    if (!objectInfo.is_nsdm())
    {
      ImGui::Text(std::meta::display_name_of(objectInfo).c_str());
    }

    // General for all numeric and floating point types
    constexpr ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
    constexpr ImGuiDataType dataType = MapTypeToImGuiDataType<DataType>();
    if constexpr (dataType != ImGuiDataType_COUNT)
    {
      constexpr std::string imGuiName = std::string("##") + std::meta::display_name_of(objectInfo);
      ImGui::InputScalar(imGuiName.c_str(), ScalarInput(dataType), &val, nullptr, nullptr, nullptr, flags)
    }
    else
    {
      // No editor implemented
      consteval std::string msg = "No editor implemented for type: " + display_name_of(^T);
      ImGui::Text(msg.c_str());
    }
  }
};


// Specialized editor for lists
template <typename T>
concept Iterable = requires(T t) 
{
  std::begin(t); // must have a begin function
  std::end(t);   // must have an end function
};
template <Iterable DataType>
struct EditorProperties
{
  static void EditorFunc(DataType& object, const std::meta::info& objectInfo)
  {
    if (ImGui::TreeNodeEx(std::meta::display_name_of(objectInfo).data(), ImGuiTreeNodeFlags_AllowItemOverlap))
    {
      int i = 0;
      for (auto& val : object)
      {
        // for list of pairs (because pairs are most likely to be maps or map-like)
        if constexpr (std::is_convertible_v<decltype(val.first), std::string>)
        {
          //TODO: make editing the first value not close the dropdown
          if(ImGui::TreeNodeEx(static_cast<std::string>(val.first).c_str(), ImGuiTreeNodeFlags_AllowItemOverlap))
          {
            RunObjectImGuiEditorInterface(val.first, std::meta::info{});
            RunObjectImGuiEditorInterface(val.second, std::meta::info{});
            ImGui::TreePop();
          }
        }
        else
        {
          RunObjectImGuiEditorInterface(val, std::meta::info{});
          i++;
        }
      }
      
      ImGui::TreePop();
    }
  }
};


template <GenerateProperties editorProperties, typename DataType>
void RunObjectImGuiEditorInterface(DataType& object, const std::meta::info& objectInfo)
{
  // If the object is a Property
  if constexpr (std::meta::template_of(std::meta::type_of(objectInfo)) == ^Property)
  {
    // And the Property has an editorFunc
    constexpr auto options = GetPropertyOptions(objectInfo);
    if constexpr (GetPropertyOptions(objectInfo).editorFunc)
    {
      options.editorFunc(objectInfo, objectInfo);
      return;
    }
  }
  
  // else run the default editor
  EditorProperties<DataType>::EditorFunc(object, objectInfo);
}

template <GenerateProperties editorProperties, typename DataType>
void GenerateObjectImGuiEditorInterface(DataType& object, const std::meta::info& objectInfo)
{
  if (ImGui::TreeNodeEx(std::meta::display_name_of(objectInfo).data(), ImGuiTreeNodeFlags_AllowItemOverlap))
  {
    RunFuncOnAllNSDM(object, [&](const std::meta::info& objectInfo)
    {
      RunImGuiValueInterface(object, member);
    }, EditorConditionFunc<editorProperties>);
    ImGui::TreePop();
  }
}

#define EDITOR_AUTO_FLAGS(class, editorProperties)\
template <>\
struct EditorProperties<class>\
{\
  static void EditorFunc(class& object, const std::meta::info& objectInfo)\
  {\
    GenerateObjectImGuiEditorInterface<editorProperties>(object)\
  }\
};

#define EDITOR_AUTO(class) EDITOR_AUTO_FLAGS(class, GenerateProperties{})


// -----------------------------------------
// ------------------Both-------------------
// -----------------------------------------

#define PROPERTY_AUTO_BOTH_FLAGS(class, serializeProperties, editorProperties)\
JSON_SERIALIZE_AUTO_FLAGS(class, serializeProperties)\
EDITOR_AUTO_FLAGS(class, editorProperties)

#define PROPERTY_AUTO_FLAGS(class, properties) PROPERTY_AUTO_BOTH_FLAGS(class, properties, properties)

#define PROPERTY_AUTO(class) PROPERTY_AUTO_FLAGS(class, GenerateProperties{})

