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
      opt = std::type_index(j.get<std::string>());
    }
  };
}

template <typename T, typename BaseC>
concept Is_Derived_Class = std::is_base_of_v<BaseC, T> && !std::is_same_v<BaseC, T>;

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
  virtual void fromjson(const json& j)
  {
    m_data = j.at("m_data").get<int>();
  }

  friend void to_json(json& j, const BaseC& base)
  {
    base.tojson(j);
  }
  friend void from_json(const json& j, BaseC& base)
  {
    base.fromjson(j);
  }

};

class Class1 : public BaseC
{
public:
  Class1() = default;
  Class1(int data, float data2) : BaseC(data), c1_data(data2) {};

  BaseC* Clone() override
  {
    return new Class1(*this);
  }

  void Print() const override
  {
    std::cout << "Class1: " << m_data << std::endl;
  }
  float c1_data = 1;

  void tojson(json& j) const override
  {
    BaseC::tojson(j);
    j["c1_data"] = c1_data;
  }
  void fromjson(const json& j) override
  {
    BaseC::fromjson(j);
    c1_data = j.at("c1_data").get<float>();
  }
};



template <typename Base, bool Cloneable = true>
class ClassList : public std::vector<std::pair<std::type_index, std::unique_ptr<Base>>>
{
  using ClassPair = std::pair<std::type_index, std::unique_ptr<Base>>;
public:
  ClassList() = default; // Empty vector

  // Constructs with the classes in it
  // Call with ClassList(std::tuple{ new Class1(), new Class2 }};
  // type of Tuple to retain types.
  // If it is not Cloneable you need to pass as a unique_ptr with std::move()
  template <Is_Derived_Class<Base> ...Objs>
  ClassList(std::tuple<Objs*...>&& classes);

  // Copy constructors
  ClassList(const ClassList& other) requires (Cloneable == true); // NOLINT(bugprone-copy-constructor-init)
  ClassList(ClassList&& other) noexcept;

  // short hand for "this | std::views::keys" to just get the typeindex
  auto Keys() const
  {
    return (*this) | std::views::keys;
  }

  // short hand for "this | std::views::values" to just get the unique_ptrs
  auto Values() const
  {
    return (*this) | std::views::values;
  }

  // Functions for adding to list from pointer
  // list.Append(new Class());
  // If it is not Cloneable you need to pass as a unique_ptr with std::move()
  template <Is_Derived_Class<Base> T>
  void Append(T*&& obj);

  template <Is_Derived_Class<Base> T>
  void Append(const std::unique_ptr<T>& obj) requires (Cloneable == true);
  template <Is_Derived_Class<Base> T>
  void Append(std::unique_ptr<T>&& obj);

  // called with list.Get<Class>()
  // returns the pointer to the class if found, else it returns nullptr
  template <Is_Derived_Class<Base> Type>
  Type* Get();
  template <Is_Derived_Class<Base> Type>
  const Type* Get() const;

  friend void to_json(json& j, const ClassList& list)
  {
    nlohmann::to_json(j, static_cast<const std::vector<ClassPair>&>(list));
  }
  friend void from_json(const json& j, ClassList& list)
  {
    nlohmann::from_json(j, static_cast<std::vector<ClassPair>&>(list));
  }
};


template <typename Base, bool Cloneable>
template <Is_Derived_Class<Base> ... Classes>
ClassList<Base, Cloneable>::ClassList(std::tuple<Classes*...>&& classes)
{
  // calls Append for every value in the tuple
  std::apply([this](auto&&... args) {
    (this->Append(std::move(args)), ...);
    }, classes);
}


template <typename Base, bool Cloneable>
ClassList<Base, Cloneable>::ClassList(const ClassList& other) requires (Cloneable == true)
{
  this->reserve(other.size());
  for (const auto& [index, component] : other)
    this->emplace_back(index, component->Clone());
}

template <typename Base, bool Cloneable>
ClassList<Base, Cloneable>::ClassList(ClassList&& other) noexcept : std::vector<ClassPair>(std::move(other))
{
}

template <typename Base, bool Cloneable>
template <Is_Derived_Class<Base> T>
void ClassList<Base, Cloneable>::Append(T*&& obj)
{
  Append(std::unique_ptr<T>(obj));
}

template <typename Base, bool Cloneable>
template <Is_Derived_Class<Base> T>
void ClassList<Base, Cloneable>::Append(const std::unique_ptr<T>& obj) requires (Cloneable == true)
{
  this->emplace_back(std::type_index(typeid(T)), obj->Clone());
}

template <typename Base, bool Cloneable>
template <Is_Derived_Class<Base> T>
void ClassList<Base, Cloneable>::Append(std::unique_ptr<T>&& obj)
{
  this->emplace_back(std::type_index(typeid(T)), std::move(obj));
}

template <typename Base, bool Cloneable>
template <Is_Derived_Class<Base> Type>
Type* ClassList<Base, Cloneable>::Get()
{
  for (auto& [id, obj] : *this)
  {
    if (std::type_index(typeid(Type)) == id)
    {
      return static_cast<Type*>(obj.get());
    }
  }
  return nullptr;
}

template <typename Base, bool Cloneable>
template <Is_Derived_Class<Base> Type>
const Type* ClassList<Base, Cloneable>::Get() const
{
  for (auto& [id, obj] : *this)
  {
    if (std::type_index(typeid(Type)) == id)
    {
      return static_cast<const Type*>(obj.get());
    }
  }
  return nullptr;
}

int main()
{
  //using type = std::pair<int, std::unique_ptr<int>>;
  //std::vector<type> v = {type{1,new int(1)}, type{2,new int(2)}, type{3,new int(3)}};
  
  //using type = std::unique_ptr<int>;
  //std::vector<type> v = {type{new int(1)}, type{new int(2)}, type{new int(3)}};
  ClassList<BaseC, false> v;
  v.Append(new Class1(1, 2));

  json j(v);
  std::cout << j << std::endl;

  ClassList<BaseC, false> l = j.get<ClassList<BaseC, false>>();

  return 0;
}