#include <unordered_map>
#include <iostream>
#include <array>
#include <map>
#include <algorithm>
#include <execution>

using IntVec2 = std::pair<int, int>;

using FloatVec2 = std::pair<float, float>;


struct vec2Hash {
  size_t operator()(const IntVec2& v) const {
    return std::hash<int>()(v.first) ^ std::hash<int>()(v.second);
  }
};

int main() {
  std::array<bool, 8> neighbors = {};

  std::cout << "Neighbors: " << neighbors[5];
  std::unordered_map<IntVec2, float, vec2Hash> myMap;
  // add values and print all key, value pairs
  myMap[IntVec2(1, 1)] = 1.0f;
  myMap[IntVec2(2, 2)] = 2.0f;
  myMap[IntVec2(3, 4)] = 3.0f;
  for (auto& p : myMap) {
    std::cout << p.first.first << "," << p.first.second << " : " << p.second << std::endl;
  }
  // print the value of a specific key
  std::cout << myMap[IntVec2(2, 3)] << std::endl;
  

  std::multimap<IntVec2, FloatVec2> projectiles;
  // add values where the key is an intvec2 that is a tenth of the floatvec2
  projectiles.insert(std::make_pair(IntVec2(0, 0), FloatVec2(10.0f, 10.0f)));
  projectiles.insert(std::make_pair(IntVec2(1, 2), FloatVec2(-15.0f, 21.0f)));
  projectiles.insert(std::make_pair(IntVec2(0, -1), FloatVec2(2.0f, 11.0f)));
  projectiles.insert(std::make_pair(IntVec2(2, 30), FloatVec2(23.0f, 32.0f)));
  projectiles.insert(std::make_pair(IntVec2(3, 2), FloatVec2(-30.0f, 10.0f)));

  
  for (auto it = projectiles.begin(); it != projectiles.end(); ++it)
  {
    IntVec2 newPos = {static_cast<float>(it->second.first / 10), static_cast<float>(it->second.second / 10)};
    if (newPos != it->first) {
      FloatVec2 pos = it->second;
      it = projectiles.erase(it);
      projectiles.emplace(newPos, pos);
    }
  }
  
  /*
  std::for_each(std::execution::par_unseq, projectiles.begin(), projectiles.end(), [](const std::pair<IntVec2, FloatVec2>& entityPair)
  {/*
    entityPair.first = 
    { static_cast<int>(entityPair.second.first / 10), 
      static_cast<int>(entityPair.second.second / 10) };
  });
*/
  // print all
  std::cout << "\n\n\n";
  for (auto& p : projectiles) {
    std::cout << p.first.first << "," << p.first.second << " : " << p.second.first << "," << p.second.second << std::endl;
  }

  return 0;
}






