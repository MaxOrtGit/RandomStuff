#include "json.hpp"
#include <iostream>
#include <vector>
#include <utility>
#include <typeindex>
#include <cassert>

using json = nlohmann::json;

namespace nlohmann {
  template <typename T>
  struct adl_serializer<std::unique_ptr<T>> {
    static void to_json(json& j, const std::unique_ptr<T>& opt) {
      if (opt.get()) {
        j = *opt;
      } else {
        j = nullptr;
      }
    }
    static void from_json(const json& j, std::unique_ptr<T>& opt) {
      opt.reset(new T(j.get<T>()));
    }
  };
  
  template <>
  struct adl_serializer<std::type_index> {
    static void to_json(json& j, const std::type_index& opt) {
      j = opt.name();
    }
    static void from_json(const json& j, std::type_index& opt) {
      opt = std::type_index(typeid(j.get<std::string>()));
    }
  };
  
  template <typename T>
  struct adl_serializer<std::pair<std::type_index, T>> {
    // store T with a key of type_index.name()
    static void to_json(json& j, const std::pair<std::type_index, T>& opt) {
      j = json{{opt.first.name(), opt.second}};
    }
    static void from_json(const json& j, std::pair<std::type_index, T>& opt) {
      opt.first = std::type_index(typeid(T));
      opt.second = j.begin().value().get<T>();
    }
  };
  
}


class BaseC;

std::unordered_map<std::string, std::function<void(json&, std::vector<std::unique_ptr<BaseC>>&)>> addComponent;
class BaseC
{
public:
  BaseC() = default;
  BaseC(int data) : m_data(data) {};
  virtual ~BaseC() = default;
  virtual void Print() const {};
  virtual BaseC* Clone() {assert(false); return nullptr;}

  int m_data;

  virtual void tojson(json& j) const
  {
    j["m_data"] = m_data;
  }
  friend void from_json(const json& j, BaseC& base)
  {
    base.m_data = j.at("m_data").get<int>();
  }

  friend void to_json(json& j, const BaseC& base)
  {
    // put all data into 
    base.tojson(j);
  }

};

class Class11 : public BaseC
{
public:
  Class11() = default;
  Class11(int data, float data2) : BaseC(data), c1_data(data2) {};

  BaseC* Clone() override
  {
    return new Class11(*this);
  }

  void Print() const override
  {
    std::cout << "Class11: " << m_data << std::endl;
  }
  float c1_data = 1;

  void tojson(json& j) const override
  {
    BaseC::tojson(j);
    j["c1_data"] = c1_data;
  }

  friend void from_json(const json& j, Class11& base)
  {
    std::cout << "there" << std::endl;
    from_json(j, static_cast<BaseC&>(base));
    base.c1_data = j.at("c1_data").get<float>();
  }
};

template<typename Base, typename T, template <typename, typename> class Map, typename ...Args>
auto constexpr impl_MakeMapOfCalls_Generic()
{
  using Func = decltype(std::function(T::template Run<Base>));
  return Map<std::string, Func>{ {typeid(Args).name() + 6, (T::template Run<Args>)}...};
}

// Default with Component as base and MODIFIERS_LIST as Args
template<typename T, template <typename, typename> class Map = std::unordered_map>
auto constexpr MakeComponentMapOfCalls()
{
  // automatically put CLASSES_LIST
  return impl_MakeMapOfCalls_Generic<BaseC, T, Map, Class11>();
}

// Class for creating the addComponent map
// the Run function it iw will template the
struct AddComponentInstance
{
  // this version of MSVC doesn't have static call operators cus it is dumb
  // remind Max later to check if he can make it a call operator
  template <typename T>
  constexpr static void Run(json& j, std::vector<std::unique_ptr<BaseC>>& entity)
  {
    std::unique_ptr<T> c = std::make_unique<T>();
    from_json(j, *c);
    entity.emplace_back(std::move(c));
  }
};


int main()
{
  addComponent = MakeComponentMapOfCalls<AddComponentInstance>();
  std::vector<std::unique_ptr<BaseC>> vec;
  vec.emplace_back(new Class11(1, 2.0f));
  vec.emplace_back(new Class11(2, 3.0f));

  json j = vec;
  std::cout << j << std::endl;

  std::vector<std::unique_ptr<BaseC>> vec2 = j;

  std::vector<std::pair<std::type_index, std::unique_ptr<BaseC>>> vec3;
  vec3.emplace_back(std::type_index(typeid(Class11)), std::make_unique<Class11>(1, 2.0f));
  vec3.emplace_back(std::type_index(typeid(Class11)), std::make_unique<Class11>(2, 3.0f));

  json j2 = vec3;
  std::cout << j2 << std::endl;
  return 0;
}