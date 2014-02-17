#include "stdio.h"

void func1 ()
{
	printf ("%s:%s()\n", __FILE__, __FUNCTION__);
};

void func2 ()
{
	printf ("%s:%s()\n", __FILE__, __FUNCTION__);
};

void func3 ()
{
	printf ("%s:%s()\n", __FILE__, __FUNCTION__);
};

void func4 ()
{
	printf ("%s:%s()\n", __FILE__, __FUNCTION__);
};

int main()
{
	printf ("%s:%s()\n", __FILE__, __FUNCTION__);
	func1();
	func2();
	func3();
	func4();
	return 0;
};
