#include "json.hpp"
#include <iostream>
#include <vector>
#include <utility>
#include <typeindex>
#include <cassert>

using json = nlohmann::json;

// output with the first element as the key and the second as the value
namespace nlohmann 
{
  template <>
  struct adl_serializer<std::vector<std::pair<std::string, std::string>>> {
    static void to_json(json& j, const std::vector<std::pair<std::string, std::string>>& opt) {
      for (auto& p : opt)
      {
        std::cout << "emplacing " << p.first << " " << p.second << std::endl;
        j.emplace(p.first, p.second);
      }
    }
    static void from_json(const json& j, std::vector<std::pair<std::string, std::string>>& opt) {
      for (auto& p : j.items())
      {
        opt.push_back({p.key(), p.value()});
      }
    }
  };
}


int main()
{
  std::vector<std::pair<std::string, std::string>> v;
  v.push_back({"a", "b"});
  v.push_back({"c", "d"});
  json j = v;
  std::cout << j << std::endl;
  for (auto& [key, value] : j.items())
  {
    std::cout << key << " " << value << std::endl;
  }
  std::vector<std::pair<std::string, std::string>> v2 = j;
  for (auto& p : v2)
  {
    std::cout << p.first << " " << p.second << std::endl;
  }

  // parse a string as json
  std::string s = "{\"a\":\"b\",\"c\":\"d\",\"a\":\"d\"}";
  json j2 = json::parse(s);
  std::cout << j2 << std::endl;
  // iterate the json
  for (auto& [key, value] : j2.items())
  {
    std::cout << key << " " << value << std::endl;
  }
  return 0;
}