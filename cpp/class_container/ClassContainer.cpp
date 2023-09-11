#include <type_traits>
#include <memory>
#include <vector>
#include <ranges>
#include <utility>
#include <typeindex>

template <typename T, typename Base>
concept Is_Sub_Class = std::is_base_of_v<Base, T> && !std::is_same_v<Base, T>;

// Custom class for ClassList to override the copy constructor 
// You can't copy a unique pointer so this creates new ones
template <typename Base, bool Owning = false>
class ClassList : public std::vector<std::pair<std::type_index, std::unique_ptr<Base>>>
{
  using ClassPair = std::pair<std::type_index, std::unique_ptr<Base>>;
public:
  ClassList() = default; // Empty vector

  // Constructs with the classes in it
  // type of Tuple to retain types
  template <Is_Sub_Class<Base> ...Mods>
  ClassList(const std::tuple<std::unique_ptr<Mods>...>& classes) requires (Owning == false)
  {
    // calls Append for every value in the tuple
    std::apply([this](const auto&... args) {
      (this->Append(args), ...);
      }, classes);
  }
  template <Is_Sub_Class<Base> ...Mods>
  ClassList(std::tuple<std::unique_ptr<Mods>...>& classes) requires (Owning == true)
  {
    // calls Append for every value in the tuple
    std::apply([this](auto&... args) {
      (this->Append(args), ...);
      }, classes);
  }
  template <Is_Sub_Class<Base> ...Mods>
  ClassList(std::tuple<std::unique_ptr<Mods>...>&& classes)
  {
    // calls Append for every value in the tuple
    std::apply([this](const auto&... args) {
      (this->Append(std::move(args)), ...);
      }, classes);
  }

  // Copy constructors
  ClassList(const ClassList& other) requires (Owning == false) // NOLINT(bugprone-copy-constructor-init)
  {
    this->reserve(other.size());
    for (const auto& [index, component] : other)
      this->emplace_back(index, component->Clone());
  }
  ClassList(ClassList&& other) noexcept
  : std::vector<ClassPair>(std::move(other))
  {
  }

  // assignment Constructors
  ClassList& operator=(const ClassList& other) requires (Owning == false)
  {
    if (this == &other)
      return *this;
    for (const auto& [index, component] : other)
      this->emplace_back(index, component->Clone());
    return *this;
  }
  ClassList& operator=(ClassList&& other) noexcept
  {
    if (this == &other)
      return *this;
    std::vector<ClassPair>::operator =(std::move(other));
    return *this;
  }

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
  template <Is_Sub_Class<Base> T>
  void Append(const Base*& obj) requires (Owning == false)
  {
    Append(std::unique_ptr<T>(obj->Clone()));
  }
  template <Is_Sub_Class<Base> T>
  void Append(Base*& obj) requires (Owning == true)
  {
    Append(std::unique_ptr<T>(obj));
    obj = nullptr;
  }
  template <Is_Sub_Class<Base> T>
  void Append(Base*&& obj)
  {
    Append(std::unique_ptr<T>(obj));
  }

  template <Is_Sub_Class<Base> T>
  void Append(const std::unique_ptr<T>& obj) requires (Owning == false)
  {
    this->emplace_back(std::type_index(typeid(T)), obj->Clone());
  }
  template <Is_Sub_Class<Base> T>
  void Append(std::unique_ptr<T>& obj) requires (Owning == true)
  {
    this->emplace_back(std::type_index(typeid(T)), std::move(obj));
  }
  template <Is_Sub_Class<Base> T>
  void Append(std::unique_ptr<T>&& obj)
  {
    this->emplace_back(std::type_index(typeid(T)), std::forward<std::unique_ptr<T>>(obj));
  }
};
