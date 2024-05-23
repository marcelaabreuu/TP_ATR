#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <conio.h>  // _getch
#include <conio.h>		// _getch

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

DWORD WINAPI ThreadFunc(LPVOID);

int main() 
{
	HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, TRUE, "ExieAlarmes");
	HANDLE hThread;
	DWORD dwThreadId;
	int i;
	hThread = (HANDLE) _beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)ThreadFunc,	// casting necess�rio
		(LPVOID)i,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necess�rio
	);
	if (hThread) printf("Thread criada Id= %0x \n", dwThreadId);
	SetConsoleTitle("Console Alarmes");
	printf("Exibi��o de Alarmes em Execu��o");
	_getch();
	return EXIT_SUCCESS;
}