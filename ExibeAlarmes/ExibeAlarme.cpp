#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <iostream>
#include <process.h>	
#include <conio.h>  // _getch
#define _CHECKERROR	1	// Ativa função CheckForError
#include "CheckForError.h"

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;
bool estado = 0;

DWORD WINAPI ThreadFunc(LPVOID index)
{
	while (1)
	{
		if (estado == 1) {
			printf("\nExibe Alarmes\n");
			Sleep(500);
		}
	}
}



int main() 
{
	SetConsoleTitle("Console Alarmes");
	HANDLE hEvent;
	hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "Alarme");
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
	if (hThread) printf("Thread criada Id= %0x \n", dwThreadId);

	Sleep(500);

	DWORD ret;
	int nTipoEvento;

	do {
		printf("Thread inicial esperando evento\n");
		ret = WaitForSingleObject(hEvent, INFINITE);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) printf("Evento acionado\n");
	} while (nTipoEvento == 0);	// Esc foi escolhido
	printf("Thread terminando...\n");

	/*while (1) {
		WaitForSingleObject(hEvent, INFINITE);
		if (estado == 0) {
			estado = 1;
			printf("\nAutorizado\n");
			Sleep(500);
		}
		else {
			estado = 0;
			printf("\nNao Autorizado");
			Sleep(500);
		}
	}*/
	return EXIT_SUCCESS;
}

