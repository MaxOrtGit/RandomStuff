#include <iostream>
#include <functional>
#include <type_traits>
#include <PropertyLib.hpp>



// ----------------Custom Serializers----------------
// json degrees store radians
constexpr PropertyOptions<float> radianSerializeToDegrees
{
  .toJsonFunc = [](json& j, const float& radians)
  {
    j[std::meta::display_name_of(^dataType)] = radians * 180 / 3.14159265358979323846f;
  },
  .fromJsonFunc = [](const json& j, float& radians)
  {
    radians = j.value(std::meta::display_name_of(^dataType), radians) * 3.14159265358979323846f / 180;
  }
  // Could also just use RadianTag
  .editorFunc = [](float& radians, const std::meta::info& objectInfo)
  {
    //TODO: glm::degrees
    float degrees = radians * 180 / 3.14159265358979323846f;
    EditorNumberField(degrees, ^radians);
    radians = degrees * 3.14159265358979323846f / 180;
  }
};

constexpr PropertyOptions<FloatVec2> dontSerialize
{
  .serialize = false
};


struct FloatVec2
{
  float x = 0;
  float y = 0;

  // Convert Vec2 to JSON without labels
  template <typename T>
  friend void to_json(json& j, const FloatVec2& vec) 
  {
    j = json{ vec.x, vec.y };
  }

  // Convert JSON to Vec2 without labels
  template <typename T>
  friend void from_json(const json& j, FloatVec2& vec) 
  {
    j.at(0).get_to(vec.x);
    j.at(1).get_to(vec.y);
  }

  // Custom editor function for FloatVec2
  friend void EditorFunc(FloatVec2& vec, const std::meta::info& objectInfo)
  {
    // Makes it so the interface is a vector of floats
    EditorArrayOfValuesField(vec, objectInfo);
  }

  // all above is the same as so you dont ned to make the friend functions
  // PROPERTY(FloatVec2, ClassFormat::Array)
};


struct Transform
{
  FloatVec2 position;
  FloatVec2 scale;
  Property<float, radianSerializeToDegrees> rotation;

  Property<FloatVec2, dontSerialize> privateVar;

  // auto generate serialize and editor function
  PROPERTY_AUTO(Transform)
};

// ----------------Key as drop down label----------------
// need to make a format to convert FloatVec2 to a string
template <>
struct std::formatter<FloatVec2> : std::formatter<std::string> 
{
  auto format(const FloatVec2& vec, format_context& ctx) const 
  {
    // formats both points as a string so format options can be used
    return formatter<string>::format(std::format("({}, {})", vec.x, vec.y), ctx);
  }
};

// Have the key be the tree node label by adding the tag DropDownLabel::Key
PropertyDefault<std::map<FloatVec2, Transform>, DropDownLabel::Key> map;

int main()
{
  
}
