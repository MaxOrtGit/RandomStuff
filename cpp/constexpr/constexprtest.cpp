constexpr int f = 10;

int main()
{
  if constexpr (f == 10)
    return 0;
  else
    return 1;
}
