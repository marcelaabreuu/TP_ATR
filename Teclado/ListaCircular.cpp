#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <iostream>
#include <process.h>	// _beginthreadex() e _endthreadex()
#include <conio.h>		// _getch

#include <sstream>
#include <vector>
#include <stack>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <ctime>
#include <iomanip>

using namespace std;

#define _CHECKERROR	1	// Ativa função CheckForError


// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

const int NumProducers = 2;
const int NumConsumers = 2;

const int MaxMsgAlarme = 200;
const int MaxNSEQ = 999999;

int j, k, m;

int sizestack, sizestackA, sizestackCLP = 0;

bool Interruptores[5] = {0, 0, 0, 0, 0};  //Interruptores[4] = ESC


struct Node
{
	string data;
	Node* link;
};

string topo, topoA, topoCLP;

Node* top = NULL;
Node* topA = NULL;
Node* topCLP = NULL;

DWORD WINAPI FuncPesagem(LPVOID);
DWORD WINAPI FuncCLPalarme(LPVOID);
DWORD WINAPI FuncCLPdado(LPVOID);
DWORD WINAPI FuncAlarme(LPVOID);
DWORD WINAPI FuncDados(LPVOID);
DWORD WINAPI FuncInterruptores(LPVOID);

HANDLE hEventA = CreateEvent(NULL, FALSE, FALSE, "CapturaAlarmes");  //Reset automático e inicializa não-sinalizado
HANDLE hEventB = CreateEvent(NULL, FALSE, FALSE, "Pesagem");
HANDLE hEventC = CreateEvent(NULL, FALSE, FALSE, "LeituraCLP");
HANDLE hEventD = CreateEvent(NULL, FALSE, FALSE, "CapturaDados");
HANDLE hEventESC = CreateEvent(NULL, TRUE, FALSE, "ESC");

HANDLE hMutex1;
HANDLE hMutexA;
HANDLE hMutexCLP;
HANDLE hThreadPesagem, hThreadCLP, hThreadAlarme, hThreadDados, hThreadCLPdado, hInterruptores, hThreadESC;
HANDLE hEventNFull;

HANDLE Mutexes1A[2] = { hMutex1, hMutexA };
HANDLE Mutexes1CLP[2] = { hMutex1, hMutexCLP };
HANDLE hThread[5] = { hThreadPesagem, hThreadCLP, hThreadAlarme, hThreadDados, hThreadCLPdado };

string teclado;

//Manipulacao de Stacks
BOOL isempty() {
	if (top == NULL)
		return true;
	else
		return false;
}
BOOL isemptyA() {
	if (topA == NULL)
		return true;
	else
		return false;
}
BOOL isemptyCLP() {
	if (topCLP == NULL)
		return true;
	else
		return false;
}
BOOL isnotfull() {
	if (sizestack < MaxMsgAlarme) {
		return true;
		SetEvent(hEventNFull);
	}
	else
		return false;
}

void push(string value) {
	if (sizestack <= 200) {
		Node* ptr = new Node();
		ptr->data = value;
		ptr->link = top;
		top = ptr;
		sizestack++;
	}
}
void pushA(string value) {
	Node* ptrA = new Node();
	ptrA->data = value;
	ptrA->link = topA;
	top = ptrA;
	sizestackA++;

}
void pushCLP(string value) {
	Node* ptrCLP = new Node();
	ptrCLP->data = value;
	ptrCLP->link = topA;
	top = ptrCLP;
	sizestackCLP++;
}
void pop()
{
	if (isempty()) {
		cout << "Stack principal vazia\n";
		sizestack = 0;
	}
	else
	{
		Node* ptr = top;
		top = top->link;
		delete(ptr);
		sizestack--;
		ResetEvent(hEventNFull);
	}
}
void popA()
{
	if (isemptyA()) {
		cout << "Stack A vazia\n";
		sizestackA = 0;
	}
	else
	{
		Node* ptrA = top;
		top = top->link;
		delete(ptrA);
		sizestackA--;
	}
}
void popCLP()
{
	if (isemptyCLP()) {
		//cout << "Stack CLP vazia\n";
		sizestackCLP = 0;
	}
	else
	{
		Node* ptrCLP = top;
		top = top->link;
		delete(ptrCLP);
		sizestackCLP--;
	}
}
int getSize()
{
	return sizeof(top->data);
}
void showTop()
{
	if (!isempty()) {
		string topo = top->data;
		cout << "Elemento no topo:" << topo << "\n";
	}
}
void showTopA()
{
	if (!isemptyA()) {
		string topoA = topA->data;
		cout << "Elemento no topo:" << topo << "\n";
	}

}
void showTopCLP()
{
	if (!isemptyCLP()) {
		string topoCLP = topCLP->data;
		cout << "Elemento no topo:" << topo << "\n";
	}
}

// Consome Stack principal e deposita nas respectivas stacks de alarme de pesagem ou clp
void ConsomeStackPrincipal() {

	WaitForMultipleObjects(2, Mutexes1A, TRUE, INFINITE);

	if (!isempty()) {
		showTop();
		if (topo == "00") {
			pushA(topo);
			pop();
		}
		else if (topo == "55" || topo == "99") {
			pushCLP(topo);
			pop();
		}
	}

	ReleaseMutex(hMutex1);
	ReleaseMutex(hMutexA);

}

int main()
{

	int status;
	void* tRetStatus;

	DWORD dwThreadId;
	DWORD dwExitCode;
	DWORD dwRet;
	int Alarme = 1, CLP = 2, Pesagem = 3, Dados = 4, CLPdado = 5, iInterruptores = 6, iESC = 7;

	SetConsoleTitle("Processo Lista Circular");
	cout << "Processo Lista Circular\nAperte ESC para finalizar.\n";

	hMutex1 = CreateMutex(NULL, FALSE, "Mutex1");
	hMutexA = CreateMutex(NULL, FALSE, "MutexA");
	hMutexCLP = CreateMutex(NULL, FALSE, "MutexCLP");

	hEventNFull = CreateEvent(NULL, TRUE, TRUE, "EventoNFull");

	//Interruptores
	hInterruptores = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncInterruptores,
		(LPVOID)iInterruptores,
		0,
		(CAST_LPDWORD)&dwThreadId
	);
	if (hThreadPesagem) printf("Thread criada Id= %0x \n", dwThreadId);



	//Tarefa de leitura do sistema de pesagem
	hThreadPesagem = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncPesagem,	
		(LPVOID)Pesagem,
		0,
		(CAST_LPDWORD)&dwThreadId	
	);
	if (hThreadPesagem) printf("Thread Interruptores criada Id= %0x \n", dwThreadId);

	//Tarefa de leitura do CLP
	hThreadCLP = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncCLPalarme,	
		(LPVOID)CLP,
		0,
		(CAST_LPDWORD)&dwThreadId	
	);
	if (hThreadCLP) printf("Thread criada Id= %0x \n", dwThreadId);

	//Tarefa de leitura do CLP
	hThreadCLPdado = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncCLPdado,	
		(LPVOID)CLPdado,
		0,
		(CAST_LPDWORD)&dwThreadId	
	);
	if (hThreadCLP) printf("Thread criada Id= %0x \n", dwThreadId);

	//Tarefa de captura de alarmes
	hThreadAlarme = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncAlarme,	// casting necessário
		(LPVOID)Alarme,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necessário
	);
	if (hThreadAlarme) printf("Thread criada Id= %0x \n", dwThreadId);

	//Tarefa de captura de dados
	hThreadDados = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncDados,	// casting necessário
		(LPVOID)Dados,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necessário
	);
	if (hThreadDados) printf("Thread criada Id= %0x \n", dwThreadId);

	do {
		ConsomeStackPrincipal();
		Sleep(1000);
	} while (!Interruptores[4]);

	dwRet = WaitForMultipleObjects(5, hThread, TRUE, INFINITE);

	for (int t = 0; t <= 4; ++t) {
		GetExitCodeThread(hThread[t], &dwExitCode);
		printf("thread %d terminou: codigo=%d\n", t, dwExitCode);
		CloseHandle(hThread[t]);	// apaga referência ao objeto
	}  // for 

	cout << "\nAcione uma tecla para terminar\n";

	return EXIT_SUCCESS;
}


DWORD WINAPI FuncPesagem(LPVOID id)
{
	HANDLE Events[2] = { hEventNFull, hMutex1 };
	do {
		// Tempo corrente do PC
		/*do {
			char ORIGEM = '00';
			// Cria um buffer para armazenar o horário formatado
			char TIMESTAMP[9];  // HH:MM:SS é 8 caracteres + 1 para o null terminator

			int CODIGO = rand() % 99 + 1;

			auto now = chrono::system_clock::now();
			// Converte o tempo para time_t, que é um formato de tempo legível por humanos
			std::time_t currentTime = chrono::system_clock::to_time_t(now);
			// Converte time_t para tm, que é uma estrutura de tempo detalhada
			std::tm* localTime = localtime(&currentTime);
			// Formata e armazena o tempo local em formato HH:MM:SS no buffer
			strftime(TIMESTAMP, sizeof(TIMESTAMP), "%H:%M:%S", localTime);
		*/
		for (int p = 0; p <= 999999; p++) {

			if (Interruptores[1] == 1) {
				WaitForMultipleObjects(2, Events, TRUE, INFINITE);

				// Secao critica
				if (sizestack <= 200) {// Se nao vazia, coloca indicador 00 de alarme pesagem
					push("00");
					if (!isempty()) {
						showTop();
						cout << "\nPESAGEM: " << topo << "\n";
					}
				}
			}
			else if (Interruptores[1] == 0) cout << "\nPesagem bloqueada\n";

			ReleaseMutex(hMutex1);

			Sleep(1000 * (rand() % 5 + 1));
		}
	} while (!Interruptores[4]);
	return(0);
}

DWORD WINAPI FuncCLPalarme(LPVOID id)
{
	HANDLE Events[2] = { hEventNFull, hMutex1 };
	do {
		for (int p = 0; p <= 999999; p++) {

			if (Interruptores[2] == 1) {
				WaitForMultipleObjects(2, Events, TRUE, INFINITE);
				if (sizestack <= 200) {// Se nao vazia, coloca indicador 55 de alarme pesagem
					push("55");
					if (!isempty()) {
						showTop();
						cout << "\nLÊ ALARME: " << topo << "\n";
					}
				}
			}
			else if (Interruptores[2] == 0) cout << "\nLeitura CLP bloqueada\n";
			ReleaseMutex(hMutex1);

			Sleep(500);
		}
	} while (!Interruptores[4]);

	return(0);
}

DWORD WINAPI FuncCLPdado(LPVOID id)
{
	HANDLE Events[2] = { hEventNFull, hMutex1 };
	do {
		for (int p = 0; p <= 999999; p++) {
			WaitForMultipleObjects(2, Events, TRUE, INFINITE);

			if (sizestack <= 200 && Interruptores[2] == 1) {// Se nao vazia, coloca indicador 55 de alarme pesagem
				push("99");
				if (!isempty()) {
					showTop();
					cout << "LÊ DADO: " << topo << "\n";
				}
			}

			ReleaseMutex(hMutex1);

			Sleep(1000 * (rand() % 5 + 1));
		}
	} while (!Interruptores[4]);

	return(0);
}

DWORD WINAPI FuncAlarme(LPVOID id)
{
	string teste;
		do {
			if (Interruptores[0] == 1) {
				WaitForSingleObject(hMutexA, INFINITE);
				if (!isemptyA()) {
					showTopA();
					cout << "\nEXIBE ALARME: " << topoA << "\n\n";
					popA();
				}
				ReleaseMutex(hMutexA);
			}
			else if (Interruptores[0] == 0) cout << "\nCaptura de Alarmes Bloqueada";
			Sleep(1000);
		} while (!Interruptores[4]);
		return(0);
}

DWORD WINAPI FuncDados(LPVOID id)
{
	string teste;

	do {
		if (Interruptores[3] == 1) {
			WaitForSingleObject(hMutexCLP, INFINITE);
			if (!isempty()) {
				showTopCLP();
				cout << "\nEXIBE DADO: " << topoCLP << "\n";
				popCLP();
			}
			ReleaseMutex(hMutexCLP);
		}
		else if (Interruptores[3] == 0) cout << "\nCaptura de dados bloqueada\n";
		Sleep(1000);
	} while (!Interruptores[4]);
	return(0);
}

DWORD WINAPI FuncInterruptores(LPVOID id)
{
	HANDLE hThreads[5] = {hEventA, hEventB, hEventC, hEventD, hEventESC};
	DWORD dwRet;
	while(true) {
		dwRet = WaitForMultipleObjects(4, hThreads, FALSE, INFINITE); //Espera qualquer evento
		int i = dwRet - WAIT_OBJECT_0;
		if (i != 4) {
			if (Interruptores[i] == 0) Interruptores[i] = 1;
			else Interruptores[i] = 0;
		}
		else Interruptores[4] = 1;
	} 
	return(0);
}
