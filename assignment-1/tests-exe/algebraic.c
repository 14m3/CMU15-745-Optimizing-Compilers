#include <stdlib.h>
#include <stdio.h>

int compute (int a, int b)
{
  int result = (a/a);

  result *= (b/b);
  result += (b-b);
  result /= result;
  result -= result;
  return result;
}

int main()
{
    printf("%d\n", compute(15,745));
}