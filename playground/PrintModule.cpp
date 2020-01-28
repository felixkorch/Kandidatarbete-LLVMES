#include <stdio.h>

extern "C" {
	int printNumber(int n)
	{
		printf("Number = %d", n);
		return 0;
	}
}