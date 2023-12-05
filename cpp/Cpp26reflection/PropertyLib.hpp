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

template<typename DataType, PropertyOptions<DataType> Options = PropertyOptions<DataType>{}, auto... Tags>
using Property = DataType;

// Overload for if you need extra parameters but don't want to specify the options
template <typename DataType, auto... Tags>
using PropertyDefault = Property<DataType, PropertyOptions<DataType>{}, Tags...>;

// can provide a function that either calls on the member or the member info
template <typename T, typename Arg>
consteval void RunFuncOnAllNSDM(T& object, std::function<void(Arg)>& func, std::function<bool(std::meta::info)> conditionFunc = nullptr)
{
  template for (constexpr auto member : std::meta::nonstatic_data_members_of(^T))
  {
    if constexpr (conditionFunc && !conditionFunc(member))
    {
      continue;
    }
    
    // if the type of Arg is info
    if constexpr (std::is_same_v<Arg, std::meta::info>)
    {
      func(member);
    }
    else
    {
      func(object.[:member:]);
    }
  }
}

template <typename T>
consteval T GetTemplateArg(const std::meta::info& member)
{
  constexpr auto templateArgs = std::meta::template_arguments_of(std::meta::type_of(member));
  template for (constexpr auto arg : templateArgs)
  {
    if constexpr (std::meta::template_of(std::meta::type_of(arg)) == ^T)
    {
      return arg;
    }
  }
  return {};
}

template <auto... Args>
using HelperClass = int;

template <typename T, auto... Args>
consteval T GetTemplateArg()
{
  using Helper = HelperClass<Args...>;

  template for (constexpr auto arg : std::meta::template_arguments_of(^Helper))
  {
    if constexpr (std::meta::type_of(arg) == ^T)
    {
      return arg;
    }
  }
  return {};
}

consteval auto GetPropertyOptions(const std::meta::info& member)
{
  return GetTemplateArg<PropertyOptions<typename[:std::meta::type_of(member):]>>(member);
}

struct GenerateProperties
{
  bool publicMembers = true;
  bool privateMembers = true; // Includes protected
  bool explicitTrueMembers = true;   // Should basically always be true
  bool explicitFalseMembers = false; // Should basically always be false
};

// ----------------Property Tags----------------
enum class ClassFormat
{
  Struct,
  Array,
};



// ------------------------------------------------
// -----------------Serialize Code-----------------
// ------------------------------------------------

// ----------------Serialize property Code----------------
template <GenerateProperties generateProperties>
bool SerializeConditionFunc(std::meta::info member)
{
  PropertyOptions options = GetPropertyOptions(member);
  if constexpr (options.serialize)
    return generateProperties.explicitMembers;
    
  if constexpr (options.dontSerialize)
    return generateProperties.explicitFalseMembers;
  
  // normal members
  if constexpr (std::meta::is_public(member))
    return generateProperties.publicMembers;
  
  // private and protected members
  return generateProperties.privateMembers;
}


// ----------------Serialize Main Functions Code----------------
template <GenerateProperties generateProperties, auto... Tags>
void ToJsonAllMembers(json& j, const auto& object)
{
  RunFuncOnAllNSDM(object, [&](const std::meta::info& member)
  {
    if constexpr (GetPropertyOptions(member))
    {
      std::meta::template_arguments_of(std::meta::typeof(member))[1].toJsonFunc(j, object.[:member:]);
    }
    else if constexpr (GetTemplateArg<ClassFormat, Tags...>() == ClassFormat::Array)
    {
      j.push_back(dataType.[:member:]);
    }
    else
    {
      j[std::meta::display_name_of(member)] = object.[:member:];
    }
  }, SerializeConditionFunc<generateProperties>);
}

template <GenerateProperties generateProperties, auto... Tags>
void FromJsonAllMembers(const json& j, auto& object)
{
  int i = 0;
  RunFuncOnAllNSDM(object, [&](const std::meta::info& member)
  {
    if constexpr (GetPropertyOptions(member))
    {
      std::meta::template_arguments_of(std::meta::typeof(member))[1].fromJsonFunc(j, object.[:member:]);
    }
    else if constexpr (GetTemplateArg<ClassFormat, Tags...>() == ClassFormat::Array)
    {
      j.at(i++).get_to(object.[:member:]);
    }
    else
    {
      object.[:member:] = j.value(std::meta::display_name_of(member), object.[:member:]);
    }
  }, SerializeConditionFunc<generateProperties>);
}

// ----------------Serialize Macros----------------
#define JSON_SERIALIZE_AUTO_FLAGS(class, generateProperties, ...)\
friend void to_json(json& j, const class& object)\
{\
  ToJsonAllMembers<generateProperties, __VA_ARGS__>(j, object);\
}\
friend void from_json(const json& j, class& object)\
{\
  FromJsonAllMembers<generateProperties, __VA_ARGS__>(j, object);\
}
#define JSON_SERIALIZE_AUTO(class, ...) JSON_SERIALIZE_AUTO_FLAGS(class, GenerateProperties{}, __VA_ARGS__)


// -----------------------------------------------
// ------------------Editor Code------------------
// -----------------------------------------------

// ----------------Editor Tags----------------
enum class DropDownLabel
{
  Index, // default
  Key,
  Value
};

// ----------------Editor Utils----------------
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
consteval std::string_view MapNumberTypeToFormatString() 
{
  if constexpr (std::is_integral_v<T>)
    return "%d";
  else if constexpr (std::is_floating_point_v<T>)
    return "%g";
  return ""; // If not a number type
}

template <GenerateProperties editorProperties>
bool EditorConditionFunc(std::meta::info member)
{
  PropertyOptions options = GetPropertyOptions(member);
  if constexpr (options.editable)
    return editorProperties.explicitMembers;
    
  if constexpr (options.nonEditable)
    return editorProperties.explicitFalseMembers;

  // normal members
  if constexpr (std::meta::is_public(member))
    return generateProperties.publicMembers;
  
  // private and protected members
  return generateProperties.privateMembers;
}

// ----------------Editor Number Field----------------
// for all numeric and floating point types
template <typename DataType>
bool EditorNumberField(DataType& number, const std::meta::info& objectInfo)
{
  constexpr ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
  constexpr ImGuiDataType dataType = MapTypeToImGuiDataType<DataType>();

  if constexpr (dataType != ImGuiDataType_COUNT)
  {
    constexpr std::string imGuiName = std::format("##{}", std::meta::display_name_of(objectInfo));
    ImGui::InputScalar(imGuiName.c_str(), ScalarInput(dataType), &val, nullptr, nullptr, nullptr, flags)
  }

  return dataType != ImGuiDataType_COUNT;
}

// for classes with all numeric/floating point types
template <typename DataType>
consteval bool IsAllSameNumeric()
{
  ImGuiDataType dataType = static_cast<ImGuiDataType>(-1);
  // loop through all members
  template for (constexpr auto member : std::meta::nonstatic_data_members_of(^DataType))
  {
    // If it is the first member set the dataType
    if (dataType == static_cast<ImGuiDataType>(-1))
    {
      dataType = MapTypeToImGuiDataType<std::meta::typeof(member)>();
    }
    // If the dataType of the member is not the same as the first member return false
    else if (dataType != MapTypeToImGuiDataType<typename[:std::meta::typeof(member):]>())
    {
      return false;
    }
  }

  // If it is a valid dataType return true
  return dataType != ImGuiDataType_COUNT;
}

// for classes with all the same numeric/floating point types
template <typename DataType>
concept AllSameNumeric = IsAllSameNumeric<DataType>();


// if all members are the same numeric type then it will be an array of pointers to the same value
template <GenerateProperties editorProperties = GenerateProperties{}>
void EditorArrayOfValuesField(AllSameNumeric auto& object, const std::meta::info& objectInfo)
{
  using DataType = decltype(object);
  using ValueType = typename[:GetFirstMember(objectInfo):];

  // array of pointers to the same value
  std::array<ValueType*, GetMemberCount(objectInfo)> pointerVec;
  
  RunFuncOnAllNSDM(object, [&](const std::meta::info& objectInfo)
  {
    pointerVec.push_back(&object.[:member:]);
  }, EditorConditionFunc<editorProperties>);

  constexpr std::string imGuiName = std::format("##{}", std::meta::display_name_of(objectInfo));
  constexpr ImGuiDataType dataType = MapTypeToImGuiDataType<ValueType>();
  void** data = reinterpret_cast<void**>(pointerArray.data());
  int size = static_cast<int>(pointerArray.size());
  constexpr std::string_view formatTag = MapNumberTypeToFormatString<ValueType>();
  constexpr ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

  ImGui::InputScalarNPTR(imGuiName.c_str(), dataType, data, size, 0, 0, formatTag.data(), ImGuiInputTextFlags_EnterReturnsTrue);
}

template <GenerateProperties editorProperties = GenerateProperties{}>
void EditorStructFields(DataType auto& object, const std::meta::info& objectInfo)
{
  RunFuncOnAllNSDM(object, [&](const std::meta::info& member)
  {
    RunObjectImGuiEditorInterface(object.[:member:], member);
  }, EditorConditionFunc<editorProperties>);
}

// ----------------Editor Functions----------------

// Default EditorFunc is general for numeric and floating point types and gives a No editor implemented for other types
template <typename DataType>
void EditorFunc(DataType& object, const std::meta::info& objectInfo)
{
  // For all not specialized (std::string, bool, int, float, etc.)
  if (!objectInfo.is_nsdm())
  {
    ImGui::Text(std::meta::display_name_of(objectInfo).c_str());
  }

  // General for all numeric and floating point types
  if (!EditorNumberField(object, objectInfo))
  {
    // No editor implemented
    consteval std::string msg = "No editor implemented for type: " + display_name_of(^DataType);
    ImGui::Text(msg.c_str());
  }
}

// ----------------Editors for ranges----------------

// ----------------Setup for ranges----------------
template<typename T>
concept Insertable = requires(T object) { object.insert({}); } || requires(T object) { object.push_back({}); };

template <Insertable DataType>
void UniversalAddNew(DataType& object)
{
  if constexpr (requires(DataType object) { object.push_back({}); })
  {
    object.push_back({});
  }
  else if constexpr (requires(DataType object) { object.insert({}); })
  {
    object.insert({});
  }
}

template <typename DataType>
concept Reorderable = requires(DataType object) { {std::swap(object.at(0), object.at(1))}; };

template <typename DataType>
concept Removable = requires(DataType object, decltype(std::begin(object)) it) 
{ 
  // can erase iterator
  { object.erase(std::begin(object)++) };
};

template <Reorderable DataType>
void RunReorderButtons(DataType& range, int index, float startX)
{
  // if first don't show up button
  if (!index == 0)
  {
    ImGui::SameLine(startX);
    // move item up
    constexpr std::string upButtonName = std::format("##ItemUp{}", index);
    if (ImGui::ArrowButton(upButtonName.c_str(), ImGuiDir_Up))
    {
      // swap with previous
      using std::swap;
      // uses advance to be universal for vectors and maps
      auto it = std::advance(std::begin(range), index - 1);
      swap(it, it++);
    }
  }

  // if last don't show down button
  if (index == range.size() - 1)
  {
    // 60 if both, 40 if not removable
    ImGui::SameLine(startX - 20);
    // move item down
    constexpr std::string downButtonName = std::format("##ItemDown{}", index);
    if (ImGui::ArrowButton(downButtonName.c_str(), ImGuiDir_Down))
    {
      // swap with next
      using std::swap;
      // uses advance to be universal for vectors and maps
      auto it = std::advance(std::begin(range), index);
      swap(it, it++);
    }
  }
}

template <Removable DataType>
void RunDeleteButton(DataType& range, int index, float startX)
{
  ImGui::SameLine(startX);
  // delete item
  constexpr std::string deleteButtonName = std::format("##ItemDelete{}", index);
  if (ImGui::Button(deleteButtonName.c_str()))
  {
    // uses advance to be universal for vectors and maps
    auto it = std::advance(std::begin(range), index);
    // so it works for both vectors and maps
    range.erase(it);
  }
}

template <typename DataType> requires Reorderable<DataType> || Removable<DataType>
void RunRangeValueButtons(DataType& range, int index)
{
  constexpr bool reorderable = Reorderable<DataType>;
  constexpr bool removable = Removable<DataType>;

  float widthX = ImGui::GetWindowContentRegionMax().x;
  
  // if reorderble do up and down buttons
  if constexpr (reorderable)
  {
    // if removable move buttons over
    RunReorderButtons(range, index, widthX - (60 + 20 * removable))
  }

  // if removable do delete button
  if constexpr (removable)
  {
    RunDeleteButton(range, index, widthX - 40);
  }
}

template <typename T>
concept TupleLike = requires(T object) { std::tuple_size<T>::value; };

template <typename T>
concept IterableTuple = std::ranges::range<T> && requires(T object) { 
  TupleLike<decltype(*std::begin(object))>;
};


// ----------------Editor Functions for ranges----------------

// Can change the label of the drop down to be the index, key, or value
// do it by doing PropertyDefault<T, DropDownLabel::Key>
template <std::ranges::range DataType>
void EditorFunc(DataType& object, const std::meta::info& objectInfo)
{
  using dropDownLabel = GetTemplateArg<DropDownLabel>(objectInfo);

  using RangeValueType = decltype(*std::begin(object));

  if (ImGui::TreeNodeEx(std::meta::display_name_of(objectInfo).data(), ImGuiTreeNodeFlags_AllowItemOverlap))
  {
    int i = 0;
    for (auto& val : object)
    {
      std::string tag;
      if constexpr (dropDownLabel == DropDownLabel::Index)
      {
        tag = std::format("{}##{}", i, i);
      }
      else if constexpr (dropDownLabel == DropDownLabel::Key)
      {
        using KeyType = decltype(std::begin(object)->first);
        if constexpr (std::formattable<KeyType>)
        {
          tag = std::format("{}##{}", std::get<0>(val), i)
        }
      }
      else if constexpr (dropDownLabel == DropDownLabel::Value && std::formattable<RangeValueType>)
      {
        tag = std::format("{}##{}", val, i);
      }

      if (ImGui::TreeNodeEx(tag.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap))
      {
        RunObjectImGuiEditorInterface(val, std::meta::info{});
        ImGui::TreePop();
      }
      i++;
    }
    if constexpr (Insertable<DataType>)
    {
      static TypeInRange newObject{};
      if (ImGui::Button("Add new with below"))
      {
        UniversalAddNew(newObject)
      }
      // TODO: see if I can change the variable name to be new object
      EditorFunc(newObject, ^newObject);
    }
    ImGui::TreePop();
  }
}

// ----------------Editor Functions for tuples----------------

// for tuple-like types
template <TupleLike DataType>
void EditorFunc(DataType& object, const std::meta::info& objectInfo)
{
  if (ImGui::TreeNodeEx(std::meta::display_name_of(objectInfo).data(), ImGuiTreeNodeFlags_AllowItemOverlap))
  {
    std::apply([&](auto&... args) { (RunObjectImGuiEditorInterface(args, std::meta::info{}), ...); }, object);
    ImGui::TreePop();
  }
}

// ----------------Editor Function for bool----------------
void EditorFunc(bool& object, const std::meta::info& objectInfo)
{
  if (!objectInfo.is_nsdm())
  {
    ImGui::Text(std::meta::display_name_of(objectInfo).c_str());
  }

  ImGui::SameLine();
  ImGui::Checkbox(std::format("##{}", std::meta::display_name_of(objectInfo)).c_str(), &object);
}

// ----------------Editor Function for std::string----------------
void EditorFunc(std::string& object, const std::meta::info& objectInfo)
{
  if (!objectInfo.is_nsdm())
  {
    ImGui::Text(std::meta::display_name_of(objectInfo).c_str());
  }

  ImGui::SameLine();
  ImGui::InputText(std::format("##{}", std::meta::display_name_of(objectInfo)).c_str(), &object);
}

// ----------------Editor Function for enums----------------
// TODO: make it so you can change the enum to a drop down

// ----------------Editor Function for ranges----------------
template <IterableTuple DataType>
void EditorFunc(DataType& object, const std::meta::info& objectInfo)
{
  if (ImGui::TreeNodeEx(std::meta::display_name_of(objectInfo).data(), ImGuiTreeNodeFlags_AllowItemOverlap))
  {
    std::apply([&](auto&... args) { (RunObjectImGuiEditorInterface(args, std::meta::info{}), ...); }, object);
    ImGui::TreePop();
  }
}

template <std::ranges::range DataType>
void EditorFunc(DataType& object, const std::meta::info& objectInfo)
{
  if (ImGui::TreeNodeEx(std::meta::display_name_of(objectInfo).data(), ImGuiTreeNodeFlags_AllowItemOverlap))
  {
    int i = 0;
    for (auto& val : object)
    {
      std::string tag = std::format("{}##{}", i, i);
      if (ImGui::TreeNodeEx(tag.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap))
      {
        RunObjectImGuiEditorInterface(val, std::meta::info{});
        ImGui::TreePop();
      }
      i++;
    }
    if constexpr (Insertable<DataType>)
    {
      static TypeInRange newObject{};
      if (ImGui::Button("Add new with below"))
      {
        UniversalAddNew(newObject)
      }


// ----------------Runs interface for any object----------------
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
      options.editorFunc(object, objectInfo);
      return;
    }
  }
  
  // else run the default editor
  EditorFunc<DataType>(object, objectInfo);
}



// ----------------Editor Generate Code----------------
template <GenerateProperties editorProperties, auto... Tags>
void GenerateObjectImGuiEditorInterface(auto& object, const std::meta::info& objectInfo)
{
  // For struct like types
  if (ImGui::TreeNodeEx(std::meta::display_name_of(objectInfo).data(), ImGuiTreeNodeFlags_AllowItemOverlap))
  {
    // If it has the Array tag and they are all the same numeric type 
    if constexpr (GetTemplateArg<ClassFormat, Tags...>() == ClassFormat::Array && AllSameNumeric<decltype(object)>)
    {
      EditorArrayOfValuesField<editorProperties>(object, objectInfo);
    }
    else
    {
      EditorStructFields<editorProperties>(object, objectInfo);
    }
    ImGui::TreePop();
  }
}

#define EDITOR_AUTO_FLAGS(class, editorProperties, ...)\
friend void EditorFunc(class& object, const std::meta::info& objectInfo)\
{\
  GenerateObjectImGuiEditorInterface<editorProperties, __VA_ARGS__>(object)\
}

#define EDITOR_AUTO(class, ...) EDITOR_AUTO_FLAGS(class, GenerateProperties{}, __VA_ARGS__)


// -----------------------------------------
// ------------------Both-------------------
// -----------------------------------------

#define PROPERTY_AUTO_BOTH_FLAGS(class, generateProperties, editorProperties, ...)\
JSON_SERIALIZE_AUTO_FLAGS(class, generateProperties, __VA_ARGS__)\
EDITOR_AUTO_FLAGS(class, editorProperties, __VA_ARGS__)

#define PROPERTY_AUTO_FLAGS(class, properties, ...) PROPERTY_AUTO_BOTH_FLAGS(class, properties, properties, __VA_ARGS__)

#define PROPERTY_AUTO(class, ...) PROPERTY_AUTO_FLAGS(class, GenerateProperties{}, __VA_ARGS__)

