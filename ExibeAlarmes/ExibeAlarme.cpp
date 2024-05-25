#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <iostream>
#include <process.h>	
#include <conio.h>  // _getch

using namespace std;

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;
bool estado = 0;

DWORD WINAPI ThreadFunc(LPVOID index)
{
	while (1)
	{
		if (estado == 1) cout << "\nExibe Alarmes\n";
		else cout << "\nExibe Alarmes: Bloqueado\n";
		Sleep(500);
	}
}

HANDLE hEvent1;

int main() 
{
	SetConsoleTitle("Console Alarmes");
	hEvent1 = CreateEvent(NULL, FALSE, FALSE, "Alarme");
	HANDLE hThread;
	DWORD dwThreadId;
	int i=0;
	hThread = (HANDLE) _beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)ThreadFunc,
		(LPVOID)i,
		0,
		(CAST_LPDWORD)&dwThreadId	
	);

	Sleep(500);

	DWORD ret;

	while (1) {
		ret = WaitForSingleObject(hEvent1, INFINITE);
		if (estado == 0) {
			estado = 1;
		}
		else {
			estado = 0;
		}
	}
	return EXIT_SUCCESS;
}

