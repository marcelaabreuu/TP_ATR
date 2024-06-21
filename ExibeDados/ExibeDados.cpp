#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <iostream>
#include <process.h>	
#include <conio.h>  // _getch
#define _CHECKERROR	1	// Ativa função CheckForError
#include <tchar.h>

using namespace std;

HANDLE hEventESC = CreateEvent(NULL, TRUE, FALSE, "ESC");
HANDLE hInterruptor = CreateEvent(NULL, TRUE, FALSE, "InterruptorD");
HANDLE hFile;
DWORD dwBytesWritten;
DWORD dwBytesRead;
LONG lFilePosLow;

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;
bool estado = 0, ESC = 0;

DWORD dwBytesLidos;
DWORD WINAPI ThreadFunc(LPVOID index)
{
	while (!ESC)
	{
		WaitForSingleObject(hInterruptor, INFINITE);
		cout << "\nEXIBE DADOS\n";
		Sleep(500);
	}
	return(0);
}

HANDLE hEvent;

int main()
{
	SetConsoleTitle("Console Dados");
	hEvent = CreateEvent(NULL, FALSE, FALSE, "Dados");
	HANDLE hThread;
	DWORD dwThreadId;
	WIN32_FIND_DATA FindFileData;
	int i = 0;

	hFile = CreateFile("processo.txt",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, // abre para leitura e escrita
		NULL,		// atributos de segurança 
		CREATE_ALWAYS,// cria novo arquivo caso ele não exista, abre se já existe
		FILE_ATTRIBUTE_NORMAL,
		NULL);		// Template para atributos e flags

	//WriteFile(hFile, "Escreve no arquivo", sizeof(string), &dwBytesWritten, NULL);
	//printf("Numero de bytes escritos = %d\n", dwBytesWritten);

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

	DeleteFile("processo.txt");
	CloseHandle(hInterruptor);
	CloseHandle(hEvent);
	CloseHandle(hEventESC);
	CloseHandle(hEvents);
	CloseHandle(hFile);
	CloseHandle(hThread);
	return EXIT_SUCCESS;
}