#include <unordered_map>
#include <iostream>
#include <array>

using vec2 = std::pair<int, int>;


struct vec2Hash {
  size_t operator()(const vec2& v) const {
    return std::hash<int>()(v.first) ^ std::hash<int>()(v.second);
  }
};

int main() {
  std::array<bool, 8> neighbors = {};

  std::cout << "Neighbors: " << neighbors[5];
  std::unordered_map<vec2, float, vec2Hash> myMap;
  // add values and print all key, value pairs
  myMap[vec2(1, 1)] = 1.0f;
  myMap[vec2(2, 2)] = 2.0f;
  myMap[vec2(3, 4)] = 3.0f;
  for (auto& p : myMap) {
    std::cout << p.first.first << "," << p.first.second << " : " << p.second << std::endl;
  }
  // print the value of a specific key
  std::cout << myMap[vec2(2, 3)] << std::endl;
  
  return 0;
}






