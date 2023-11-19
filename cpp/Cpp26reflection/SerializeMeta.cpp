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
    j[display_name_of(^dataType)] = radians * 180 / 3.14159265358979323846f;
  },
  .fromJsonFunc = [](const json& j, float& radians)
  {
    radians = j.value(display_name_of(^dataType), radians) * 3.14159265358979323846f / 180;
  }
  .editorFunc = [](float& radians)
  {
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
  JSON_SERIALIZE_AUTO(FloatVec2)
};

struct Transform
{
  FloatVec2 position;
  FloatVec2 scale;
  Property<float, radianSerializeToDegrees> rotation;

  Property<FloatVec2, dontSerialize> privateVar;
  JSON_SERIALIZE_AUTO(Transform)
};

int main()
{
  
}
