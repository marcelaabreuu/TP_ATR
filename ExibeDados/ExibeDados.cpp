#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <iostream>
#include <process.h>	
#include <conio.h>  // _getch
#define _CHECKERROR	1	// Ativa fun��o CheckForError
#include <tchar.h>

using namespace std;

HANDLE hEventESC = CreateEvent(NULL, TRUE, FALSE, "ESC");
HANDLE hInterruptor = CreateEvent(NULL, TRUE, FALSE, "InterruptorD");
BOOL bStatus;

HANDLE hTimeOut = CreateEvent(NULL, TRUE, FALSE, "EvTimeOut"); //Evento nunca disparado, usado para temporizacao

//Arquivo em disco
HANDLE hFile;
DWORD dwBytesWritten;
DWORD dwBytesRead;
LONG lFilePosLow;
HANDLE hMutexArquivo;

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;
bool estado = 0, ESC = 0;

DWORD dwBytesLidos;
DWORD WINAPI ThreadFunc(LPVOID index)
{
	char MsgLida[30] = {"Sem Dados Lidos              "};
	while (!ESC)
	{
		WaitForSingleObject(hInterruptor, INFINITE);
		WaitForSingleObject(hMutexArquivo, INFINITE);
		bStatus = ReadFile(hFile, &MsgLida, 28, &dwBytesRead, NULL);
		if (bStatus == 0)  std::cerr << "\nErro na leitura Exibe Dados = " << GetLastError() << "\n";
		ReleaseMutex(hMutexArquivo);
		printf("\nDados do processo: ");
			string str(MsgLida);
			string NSEG = str.substr(0, 6);
			string TIME = str.substr(20, 8);
			string VEL = str.substr(10, 5);
			string IC = str.substr(16, 1);
			string FC = str.substr(18, 1);

			if (IC == "1") IC = "Ligado";
			else IC = "Desligado";

			if (FC == "1") FC = "Ligado";
			else FC = "Desligado";

			string msg_total = TIME + " NSEG: " + NSEG + " VEL: " + VEL + " SENSOR IC: " + IC + " SENSOR FC: " + FC;
			cout << msg_total << endl;

				
		Sleep(500);
	}
	_endthreadex(0);
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
	hMutexArquivo = CreateMutex(NULL, FALSE, "MutexArquivo");

	hFile = CreateFile("processo.txt",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, // abre para leitura e escrita
		NULL,		// atributos de seguran�a 
		CREATE_ALWAYS,// cria novo arquivo caso ele n�o exista, abre se j� existe
		FILE_ATTRIBUTE_NORMAL,
		NULL);		// Template para atributos e flags

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