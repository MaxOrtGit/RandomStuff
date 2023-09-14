#include <iostream>
#include <bitset>
#include <array>


template <int... I>
consteval bool AllTrue(std::bitset<8>& neighbors)
{
  return (neighbors[I % 8] && ...);
}


template <int... I>
consteval bool AnyRotation(std::bitset<8>& neighbors)
{
    if (AllTrue<I...>(neighbors)) return 1;
    if (AllTrue<(I + 2)...>(neighbors)) return 2;
    if (AllTrue<(I + 4)...>(neighbors)) return 3;
    if (AllTrue<(I + 6)...>(neighbors)) return 4;
    return 0; // Return 0 if none of the conditions are met
}


consteval std::array<int, 256> GetPermutation()
{
  std::array<int, 256> permutations {};
  for (int i = 0; i < 256; i++)
  {
    std::bitset<8> n(i);
    // Full Blocks
    if (int x = AnyRotation<1, 5>(n))
    {
      permutations[i] = 16;
      continue;
    }
    if (int x = AnyRotation<1, 3, 6>(n))
    {
      permutations[i] = 16;
      continue;
    }
    if (int x = AnyRotation<0, 2, 5>(n))
    {
      permutations[i] = 16;
      continue;
    }
    if (int x = AnyRotation<0, 2, 4, 6>(n))
    {
      permutations[i] = 16;
      continue;
    }

    // L pieces
    if (int x = AnyRotation<1, 3>(n))
    {
      permutations[i] = 11 + x;
      continue;
    }
    if (int x = AnyRotation<0, 3>(n))
    {
      permutations[i] = 11 + x;
      continue;
    }
    if (int x = AnyRotation<0, 5>(n))
    {
      permutations[i] = 11 + x;
      continue;
    }
    if (int x = AnyRotation<0, 2, 4>(n))
    {
      permutations[i] = 11 + x;
      continue;
    }
    if (int x = AnyRotation<0, 2, 3>(n))
    {
      permutations[i] = 11 + x;
      continue;
    }
    if (int x = AnyRotation<1, 2, 4>(n))
    {
      permutations[i] = 11 + x;
      continue;
    }
    if (int x = AnyRotation<2, 4, 5>(n))
    {
      permutations[i] = 11 + x;
      continue;
    }

    // Edge pieces
    if (int x = AnyRotation<1>(n))
    {
      permutations[i] = 7 + x;
      continue;
    }
    if (int x = AnyRotation<0, 1>(n))
    {
      permutations[i] = 7 + x;
      continue;
    }
    if (int x = AnyRotation<1, 2>(n))
    {
      permutations[i] = 7 + x;
      continue;
    }
    if (int x = AnyRotation<0, 2>(n))
    {
      permutations[i] = 7 + x;
      continue;
    }

    // 2 corner pieces
    if (int x = AnyRotation<0, 4>(n))
    {
      permutations[i] = 3 + x;
      continue;
    }


    // 1 corner pieces
    if (int x = AnyRotation<0>(n))
    {
      permutations[i] = x - 1;
      continue;
    }
    permutations[i] = -1;
  }
  return permutations;
}

constexpr std::array<int, 256> permutations = GetPermutation();



int main()
{
}