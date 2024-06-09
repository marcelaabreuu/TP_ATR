#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <iostream>
#include <process.h>	
#include <conio.h>  // _getch
#define _CHECKERROR	1	// Ativa função CheckForError
#include <tchar.h>

using namespace std;

HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, "Alarme");
HANDLE hEventESC = CreateEvent(NULL, TRUE, FALSE, "ESC");
HANDLE hInterruptor = CreateEvent(NULL, TRUE, FALSE, "InterruptorA");
HANDLE hMailslot;

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;
bool estado = 0, ESC = 0;

DWORD dwBytesLidos;
DWORD WINAPI ThreadFunc(LPVOID index)
{
	while (!ESC)
	{
		WaitForSingleObject(hInterruptor, INFINITE);
		cout << "\nEXIBE ALARMES\n";
		string Msg;
		ReadFile(hMailslot, &Msg, sizeof(string), &dwBytesLidos, NULL);
		cout << "\nMensagem lida do mailsot: " << Msg << endl;
		Sleep(500);
	}
	return(0);
}

int main()
{
	SetConsoleTitle("Console Alarmes");
	HANDLE hThread;
	DWORD dwThreadId;
	int i = 0;
	hThread = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)ThreadFunc,
		(LPVOID)i,
		0,
		(CAST_LPDWORD)&dwThreadId
	);

	DWORD ret;
	HANDLE hEvents[2] = { hEvent, hEventESC };

	//Criando o mailslot (servidor)
	hMailslot = CreateMailslot(
		"\\\\.\\mailslot\\MyMailslot",
		0,
		MAILSLOT_WAIT_FOREVER,
		NULL);

	while (1) {
		ret = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		int i = ret - WAIT_OBJECT_0;
		if (ret == 0) {
			if (estado == 0) {
				estado = 1;
				SetEvent(hInterruptor);
				cout << "\nTarefa desbloqueada\n";
			}
			else {
				estado = 0;
				ResetEvent(hInterruptor);
				cout << "\nTarefa bloqueada\n";
			}
		}
		else { ESC = 1; break; }
	}

	CloseHandle(hInterruptor);
	CloseHandle(hEvent);
	CloseHandle(hEventESC);
	CloseHandle(hEvents);
	CloseHandle(hThread);
	CloseHandle(hMailslot);
	return EXIT_SUCCESS;
}

