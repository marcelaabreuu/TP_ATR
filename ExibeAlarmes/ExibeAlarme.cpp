#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <conio.h>  // _getch
#include <conio.h>		// _getch

DWORD WINAPI ThreadFunc(LPVOID);

int main() 
{
	HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, TRUE, "ExieAlarmes");
	HANDLE hThread;
	DWORD dwThreadId;
	int i;
	uintptr_t _beginthreadex( 
		NULL,
		0,
		ThreadFunc,
		(LPVOID)i,
		0,
		&dwThreadId
	);
	SetConsoleTitle("Console Alarmes");
	printf("Exibição de Alarmes em Execução");
	_getch();
	return EXIT_SUCCESS;
}