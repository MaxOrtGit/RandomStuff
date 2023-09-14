#include <iostream>
#include <bitset>


template <int... I>
bool AllTrue(std::bitset<8>& neighbors)
{
  return (neighbors[I % 8] && ...);
}


template <int... I>
bool AnyRotation(std::bitset<8>& neighbors)
{
  return AllTrue<I...>(neighbors)     || AllTrue<(I + 2)...>(neighbors) || 
         AllTrue<(I + 4)...>(neighbors) || AllTrue<(I + 6)...>(neighbors);
}

int main()
{
  int j = 0;
  // create a loop that gives every posible std::bitset<8>
  for (int i = 0; i < 256; i++)
  {
    std::bitset<8> n(i);
    if (n[1] && n[6] || n[3] && n[4] || // opposites
        AnyRotation<1, 3, 6>(n) || // 2 edge one corner
        AnyRotation<0, 2, 5>(n) || 
        n[0] && n[2] && n[4] && n[6])
    {continue;}

    // L pieces
    if (AnyRotation<1, 3>(n) || AnyRotation<0, 3>(n) || AnyRotation<0, 5>(n) || 
        AnyRotation<0, 2, 4>(n) || AnyRotation<0, 2, 3>(n) || AnyRotation<1, 2, 4>(n) ||
        AnyRotation<2, 5, 4>(n))
    {continue;}

    // Edge pieces
    if (AnyRotation<1>(n) || AnyRotation<0, 1>(n) || AnyRotation<1, 2>(n) || AnyRotation<0, 2>(n))
    {continue;
    }

    // 2 corner pieces
    if (AnyRotation<0, 4>(n))
    {continue;
    }

    // 1 corner pieces
    if (AnyRotation<0>(n))
    {continue;
    }
    std::cout << n << std::endl;
    j++;
  }
  std::cout << j << std::endl;
}