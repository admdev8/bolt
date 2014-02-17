#include <windows.h>
#include <stdio.h>

void func1 ()
{
	printf ("%s:%s()\n", __FILE__, __FUNCTION__);
};

void func2 ()
{
	printf ("%s:%s()\n", __FILE__, __FUNCTION__);
};

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
};
