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
HANDLE pipe;

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;
bool estado = 0, ESC = 0;

DWORD dwBytesLidos;

DWORD WINAPI ThreadFunc(LPVOID index)
{
	pipe = CreateNamedPipe(
		"\\\\.\\pipe\\my_pipe", // name of the pipe
		PIPE_ACCESS_INBOUND, // 1-way pipe -- send only
		PIPE_TYPE_BYTE, // send data as a byte stream
		2, // only allow 1 instance of this pipe
		0, // no outbound buffer
		0, // no inbound buffer
		0, // use default wait time
		NULL // use default security attributes
	);
		if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
			wcout << "Failed to create outbound pipe instance.";
			// look up error code here using GetLastError()
			system("pause");
			return 1;
		}
	while (!ESC)
	{
		WaitForSingleObject(hInterruptor, INFINITE);

		BOOL result = ConnectNamedPipe(pipe, NULL);
			if (!result) {
				wcout << "Falha ao conectar ao pipe." << endl;
				// look up error code here using GetLastError()
				CloseHandle(pipe); // close the pipe
				system("pause");
				return 1;
			}

		cout << "Lendo dados vindo do pipe..." << endl;

		// The read operation will block until there is data to read
		wchar_t buffer[128];
		DWORD numBytesRead = 0;
		BOOL result2 = ReadFile(
			pipe,
			buffer, // the data from the pipe will be put here
			127 * sizeof(wchar_t), // number of bytes allocated
			&numBytesRead, // this will store number of bytes actually read
			NULL // not using overlapped IO
		);
			if (result2) {
				buffer[numBytesRead / sizeof(wchar_t)] = '\0'; // null terminate the string
				cout << "\nEXIBE ALARMES\n";
				cout << "Number of bytes read: " << numBytesRead << endl;
				cout << "Mensagem lida do pipe: " << buffer << endl;
			}
			else {
				cout << "Falha ao ler mensagem do pipe." << endl;
			}

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
	CloseHandle(pipe);

	return EXIT_SUCCESS;
}



