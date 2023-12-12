#include <fstream>
#include <vector>
#include <iostream>
#include <string>


void SkipVoidSpace(std::istream& stream)
{
  constexpr std::string_view voidSpace = " \n\t";
  char buffer;
  while (stream >> buffer && voidSpace.contains(buffer));
  stream.putback(buffer);
}


template <typename T>
std::istream& operator>>(std::istream& stream, std::vector<T>& vec) 
{
  char buffer;
  if (stream >> buffer && buffer != '{')
    stream.putback(buffer);
  while(1)
  {
    SkipVoidSpace(stream);
    if (stream >> buffer && buffer == '}')
      break;
    else
      stream.putback(buffer);
      
    T var;
    stream >> var;
    vec.push_back(var);

    SkipVoidSpace(stream);

    if (stream >> buffer && buffer == '}')
      break;
  };
  return stream;
}

int main()
{
  std::ifstream file("file.entity");
  std::vector<int> vec;
  char c;

  while (file >> c && c != '{')
  {
  }
  file >> vec;
  std::cout << "\n\n" ;

  for (auto i : vec)
    std::cout << i << '\n';
    
  std::vector<std::vector<int>> vec2;
  
  while (file >> c && c != '{')
  {
    std::cout << c << ", bad\n";
  }
    std::cout << c << ", good\n";
  
  file >> vec2;
  std::cout << "\n\n";
  for (auto v : vec2)
  {
    for (auto i : v)
      std::cout << i << ", ";
    std::cout << '\n';
  }
  std::cout << "\n\n" ;

  return 0;
}