#include <iostream>
#include <memory>
#include <unordered_map>

std::unordered_map<std::string, std::weak_ptr<char>> weakPtrTable;

std::shared_ptr<char> GetTexture(std::string name)
{
  auto it = weakPtrTable.find(name);
  if (it != weakPtrTable.end())
  {
    if (auto ptr = it->second.lock())
    {
      std::cout << "Getting " << name << " from table\n";
      // Getting shared_ptr from weak_ptr
      return std::move(ptr);
    }
  }

  std::cout << "Adding " << name << " to table\n";
  // Creating new shared_ptr
  auto ptr = std::make_shared<char>(name[0]);
  weakPtrTable[name] = ptr;
  return std::move(ptr);
}

void RemoveExpiredSharedPtrs() {
  auto it = weakPtrTable.begin();
  while (it != weakPtrTable.end()) {
    if (it->second.expired()) {
      it = weakPtrTable.erase(it);
    } else {
      ++it;
    }
  }
}

struct Sprite
{
  std::shared_ptr<char> texture;
  Sprite(std::string name) : texture(GetTexture(name)) {}

  ~Sprite()
  {
    // If it is the last entity using the texture, remove it from the table
    if (texture.use_count() == 1)
    {
      RemoveExpiredSharedPtrs();
    }
  }
};

void PrintTable() {

  std::cout << "Number of shared pointers in the table: " << weakPtrTable.size() << "\ncounts are:\n";
  for (auto& [key, value] : weakPtrTable) {
    std::cout << key << ": "<< value.use_count() << "\n";
  }
    std::cout << "\n";
}

int main() {
  Sprite e("n1");
  {
    Sprite e1("n1");
    Sprite e2("n2");
    PrintTable();
  }
  PrintTable();
  Sprite e2("n2");

  return 0;
}
