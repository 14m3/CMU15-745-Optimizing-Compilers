#include <string.h> 
#include <stdlib.h>
#include <stdio.h>
void msqrt(char *str)
{
	double i, r, n;
	int j, l, num, x[1005];
	if (strlen(str) == 1 && str[0] == '0')
	{
		printf("0\n");
		return;
	}
	if (strlen(str) % 2 == 1)
	{
		n = str[0] - 48;
		l = -1;
	}
	else
	{
		n = (str[0] - 48) * 10 + str[1] - 48;
		l = 0;
	}
	r = 0, num = 0;
	while (1)
	{
		i = 0;
		while (i*(i + 20 * r) <= n)
			i++;
		i--;
		n -= i*(i + 20 * r);
		r = r * 10 + i;
		x[num] = (int)i;
		num++;
		l += 2;
		if (l >= strlen(str))
			break;
		n = n * 100 + (double)(str[l] - 48) * 10 + (double)(str[l + 1] - 48);
	}
	for (j = 0; j<num; j++)
		printf("%d",x[j]);
}
int main()
{
	char ch[615];
	for(int i=0;i<615;++i)
	{
		scanf("%c",&ch[i]);
	}
	msqrt(ch);
	return 0;
}