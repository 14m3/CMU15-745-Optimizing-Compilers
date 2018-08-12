#include <stdlib.h>
#include <stdio.h>

int g;
int g_incr (int c)
{
  g += c;
  return g;
}
int loop (int a, int b, int c)
{
  int i;
  int ret = 0;
  for (i = a; i < b; i++) {
   g_incr (c);
  }
  return ret + g;
}

int main()
{
    g = 15745;
    printf("%d\n", loop(15,74,5));
}