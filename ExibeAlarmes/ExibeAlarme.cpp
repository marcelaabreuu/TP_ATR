#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <iostream>
#include <process.h>	
#include <conio.h>  // _getch
#include <tchar.h>
#define _CHECKERROR	1	// Ativa função CheckForError
#include "CheckForError.h"


using namespace std;

HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, "Alarme");
HANDLE hEventESC = CreateEvent(NULL, TRUE, FALSE, "ESC");
HANDLE hInterruptor = CreateEvent(NULL, TRUE, FALSE, "InterruptorA");
HANDLE pipe;

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;
bool estado = 0, ESC = 0;
DWORD dwBytesLidos;
const char* textos[10] = { "Sobrecarga no motor principal. Reduzir carga ou desligar o equipamento.", 
						   "Pressao alta no tanque de armazenamento. Verificar valvulas de alivio.", 
						   "Nivel baixo no reservatorio de agua. Reabastecer imediatamente.", 
						   "Parada de emergencia acionada. Verificar sistema de seguranca e reiniciar.",
						   "Obstrucao detectada na esteira transportadora. Remover obstaculo e reiniciar.",
						   "Motor sobreaquecido no setor C. Desligar e verificar ventilacao.",
						   "Baixa pressao na linha de ar comprimido. Verificar compressor e tubulacoes.",
						   "Sistema offline. Verificar conectividade e reiniciar servidores.",
						   "Perda de comunicacao com PLC. Checar cabos e reiniciar controlador.",
						   "Interferencia eletrica no sensor de proximidade. Verificar cabos e fonte de ruido."};



DWORD WINAPI ThreadFunc(LPVOID index)
{
	DWORD dwErrorCode;			// Código de erro retornado por GetLastError()
	BOOL bStatus;
	char MyBuffer[128];
	DWORD dwLength;
	DWORD dwBytesRead;
	DWORD numBytesRead = 0;
	string NSEGant, NSEG;

	pipe = CreateNamedPipe(
		"\\\\.\\pipe\\myPipe",
		PIPE_ACCESS_DUPLEX,	// Comunicação Full Duplex
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1,			// Número de instâncias
		0,			// nOutBufferSize
		0,			// nInBufferSize
		15000,		// Timeout default para esperar por cliente
		NULL);		// Atributos de segurança
	
	while (!ESC)
	{
		WaitForSingleObject(hInterruptor, INFINITE);

		bStatus = ConnectNamedPipe(pipe, NULL); // Fica preso esperando conexão
		if (bStatus) {
			printf("\nCliente se conectou com sucesso");
		}
		else {
			dwErrorCode = GetLastError();
			if (dwErrorCode == ERROR_PIPE_CONNECTED) {
				//printf("\nCliente já havia se conectado");
			}  // if
			else if (dwErrorCode == ERROR_NO_DATA) {
				printf("\nCliente fechou seu handle");
				return 0;
			} // if
			else
				cout << "Erro checkerror\n";
		}  // else

		
		
		BOOL result2 = ReadFile(
			pipe,
			&MyBuffer, 
			127 * sizeof(wchar_t),
			&dwBytesRead, // this will store number of bytes actually read
			NULL // not using overlapped IO
		);
			if (result2) {				
				MyBuffer[dwBytesRead / sizeof(wchar_t)] = '\0';

					string str(MyBuffer);
					NSEG = str.substr(0, 6);
					string TIME = str.substr(13, 8);
					string ORIGEM = str.substr(10, 2);
					//cout << "\nEXIBE ALARMES\n";
				if (NSEGant != NSEG) {
					string msg_total = TIME + " NSEG: " + NSEG + " ORIGEM: " + ORIGEM + " " + textos[rand() % 10];
					//cout << "Number of bytes read: " << dwBytesRead << endl;
					cout << msg_total << endl;
				}
			}
			else {
				cout << "Falha ao ler mensagem do pipe." << endl;
			}

			NSEGant = NSEG; 
		
	}
	_endthreadex(0);
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



