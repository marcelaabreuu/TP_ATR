#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <iostream>
#include <process.h>	
#include <conio.h>  // _getch

using namespace std;

HANDLE hEventESC = CreateEvent(NULL, TRUE, FALSE, "ESC");

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;
bool estado = 0, ESC = 0;

DWORD WINAPI ThreadFunc(LPVOID index)
{
	while (1)
	{
		if (ESC == 0) {
			if (estado == 1) cout << "\nExibe Alarmes\n";
			else cout << "\nExibe Alarmes: Bloqueado\n";
			Sleep(500);
		}
		else _endthreadex(0);
	}
}

HANDLE hEvent;

int main() 
{
	SetConsoleTitle("Console Alarmes");
	hEvent = CreateEvent(NULL, FALSE, FALSE, "Alarme");
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
	HANDLE hThreads[2] = { hEvent, hEventESC };

	while (1) {
		ret = WaitForMultipleObjects(2, hThreads, FALSE, INFINITE);
		int i = ret - WAIT_OBJECT_0;
		if (ret == 0) {
			if (estado == 0) {
				estado = 1;
			}
			else {
				estado = 0;
			}
		}
		else { ESC = 1; break; }
	}
	return EXIT_SUCCESS;
}

