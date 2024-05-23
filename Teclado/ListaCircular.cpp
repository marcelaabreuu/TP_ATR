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
#include "CheckForError.h"

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

const int NumProducers = 2;
const int NumConsumers = 2;

const int MaxMsgAlarme = 200;
const int MaxNSEQ = 999999;

int i, j, k;



stack<char> products; // Mensagem na lista 
mutex xmutex;

condition_variable is_not_full;             // Indicando que stack nao esta cheia 
condition_variable is_not_empty;			// Indicando que stack nao esta vazia 

// print function for "thread safe" printing using a stringstream
void print(ostream& s) { cout << s.rdbuf(); cout.flush(); s.clear(); }

int main()
{


	int i;
	int status;
	void* tRetStatus;
	HANDLE hThreadPesagem, hThreadCLP, hThreadAlarme, hThreadDados;
	DWORD dwThreadId;

	SetConsoleTitle("Processo Lista Circular");
	cout << "Processo Lista Circular\n";
	//_getch();

	//Tarefa de leitura do sistema de pesagem
	hThreadPesagem = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncPesagem,	// casting necessário
		(LPVOID)i,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necessário
	);
	if (hThreadCLP) printf("Thread criada Id= %0x \n", dwThreadId);

	//Tarefa de leitura do CLP
	hThreadCLP = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncCLP,	// casting necessário
		(LPVOID)i,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necessário
	); 
	if (hThreadCLP) printf("Thread criada Id= %0x \n", dwThreadId);

	//Tarefa de captura de alarmes
	hThreadAlarme = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncAlarme,	// casting necessário
		(LPVOID)i,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necessário
	);
	if (hThreadAlarme) printf("Thread criada Id= %0x \n", dwThreadId);

	//Tarefa de captura de dados
	hThreadDados = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncDados,	// casting necessário
		(LPVOID)i,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necessário
	);
	if (hThreadDados) printf("Thread criada Id= %0x \n", dwThreadId);

	return EXIT_SUCCESS;
}

// Produz dado
void produce(char msg)
{
	unique_lock<mutex> lock(xmutex);
	//int product;

	is_not_full.wait(lock, [] { return products.size() != MaxMsgAlarme; });
	//product = products.size();
	products.push(msg);

	cout << "Producer depositou a mensagem" << msg << "\n";
	is_not_empty.notify_all();

}

// Consome dado
void consume(int consumer_id)
{
	unique_lock<mutex> lock(xmutex);
	int product;

	if (is_not_empty.wait_for(lock, chrono::milliseconds(INFINITE),
		[] { return products.size() > 0; }))
	{
		product = products.top();
		products.pop();

		cout << "Consumed:" << product << "\n";
		is_not_full.notify_all();
	}
}

DWORD WINAPI FuncPesagem(LPVOID id) 
{	
	do {
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

		Sleep(1000 * (rand() % 5 + 1));

		produce(i);
		i++;
	} while (TRUE);
}

DWORD WINAPI FuncCLP(LPVOID id)
{
	Sleep(500);
}

DWORD WINAPI FuncAlarme(LPVOID id)
{
}

DWORD WINAPI FuncDados(LPVOID id)
{
}