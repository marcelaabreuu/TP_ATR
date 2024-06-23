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
#include <time.h>
#include <string>

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

bool Interruptores[5] = { 0, 0, 0, 0, 0 };  //Interruptores[4] = ESC


#define LENGTHMAX 29

struct Node
{
	string data;
	Node* link;
};


string topo(LENGTHMAX,0), topoA(LENGTHMAX, 0), topoCLP(LENGTHMAX+1, 0);

Node* top = NULL;
Node* topA = NULL;
Node* topCLP = NULL;

DWORD WINAPI FuncPesagem(LPVOID);
DWORD WINAPI FuncCLPalarme(LPVOID);
DWORD WINAPI FuncCLPdado(LPVOID);
DWORD WINAPI FuncAlarme(LPVOID);
DWORD WINAPI FuncDados(LPVOID);
DWORD WINAPI ConsomeStackPrincipal(LPVOID);

//Arquivo em Disco
HANDLE hFile;
DWORD dwBytesWritten;
DWORD dwBytesRead;
LONG lFilePosLow;
BOOL bStatus;

//Eventos de temporizacao com timeout
HANDLE hTimeOut = CreateEvent(NULL, TRUE, FALSE, "EvTimeOut"); //Evento nunca disparado, usado para temporizacao

//Eventos do Teclado
HANDLE hEventA = CreateEvent(NULL, FALSE, FALSE, "CapturaAlarmes");  //Reset automático e inicializa não-sinalizado
HANDLE hEventB = CreateEvent(NULL, FALSE, FALSE, "Pesagem");
HANDLE hEventC = CreateEvent(NULL, FALSE, FALSE, "LeituraCLP");
HANDLE hEventD = CreateEvent(NULL, FALSE, FALSE, "CapturaDados");

//Eventos Interruptores
HANDLE hIntA = CreateEvent(NULL, TRUE, FALSE, "IntA");
HANDLE hIntB = CreateEvent(NULL, TRUE, FALSE, "IntB");
HANDLE hIntC = CreateEvent(NULL, TRUE, FALSE, "IntC");
HANDLE hIntD = CreateEvent(NULL, TRUE, FALSE, "IntD");
HANDLE hInts[4] = { hIntA, hIntB, hIntC, hIntD };

HANDLE hEventESC = CreateEvent(NULL, TRUE, FALSE, "ESC");

HANDLE hMutex1;
HANDLE hMutexA;
HANDLE hMutexCLP;
HANDLE hThreadPesagem, hThreadCLP, hThreadAlarme, hThreadDados, hThreadCLPdado, hInterruptores, hThreadESC, hThreadAUX;
HANDLE hEventNFull;

HANDLE Mutexes1A[2] = { hMutex1, hMutexA };
HANDLE Mutexes1CLP[2] = { hMutex1, hMutexCLP };
HANDLE hThread[7] = { hThreadPesagem, hThreadCLP, hThreadAlarme, hThreadDados, hThreadCLPdado, hThreadAUX, hInterruptores };
HANDLE pipe;
HANDLE hMutexArquivo;


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
	if (sizestackA <= 100) {
		Node* ptrA = new Node();
		ptrA->data = value;
		ptrA->link = topA;
		topA = ptrA;
		sizestackA++;
	}
}
void pushCLP(string value) {
	if (sizestackCLP <= 100) {
		Node* ptrCLP = new Node();
		ptrCLP->data = value;
		ptrCLP->link = topA;
		topCLP = ptrCLP;
		sizestackCLP++;
	}
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
		//ResetEvent(hEventNFull);
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
		Node* ptrA = topA;
		topA = topA->link;
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
		Node* ptrCLP = topCLP;
		topCLP = topCLP->link;
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
		topo = top->data;
		//cout << "Elemento no topo Lista 1:" << topo << "\n";
	}
}
void showTopA()
{
	if (!isemptyA()) {
		topoA = topA->data;
		//cout << "Elemento no topo alarme:" << topoA << "\n";
	}

}
void showTopCLP()
{
	if (!isemptyCLP()) {
		topoCLP = topCLP->data;
		//cout << "Elemento no topo dado:" << topoCLP << "\n";
	}
}

bool estado = 0, ESC = 0;

int main()
{

	int status;
	void* tRetStatus;

	DWORD dwThreadId;
	DWORD dwExitCode;
	DWORD dwRet;
	int Alarme = 1, CLP = 2, Pesagem = 3, Dados = 4, CLPdado = 5, iInterruptores = 6, iESC = 7, iAUX = 8;

	hMutex1 = CreateMutex(NULL, FALSE, "Mutex1");
	hMutexA = CreateMutex(NULL, FALSE, "MutexA");
	hMutexCLP = CreateMutex(NULL, FALSE, "MutexCLP");
	hMutexArquivo = CreateMutex(NULL, FALSE, "MutexArquivo");

	hEventNFull = CreateEvent(NULL, TRUE, TRUE, "EventoNFull");

	//Tarefa de leitura do sistema de pesagem
	hThreadPesagem = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncPesagem,
		(LPVOID)Pesagem,
		0,
		(CAST_LPDWORD)&dwThreadId
	);

	//Tarefa de leitura do CLP
	hThreadCLP = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncCLPalarme,
		(LPVOID)CLP,
		0,
		(CAST_LPDWORD)&dwThreadId
	);

	//Tarefa de leitura do CLP
	hThreadCLPdado = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncCLPdado,
		(LPVOID)CLPdado,
		0,
		(CAST_LPDWORD)&dwThreadId
	);

	//Tarefa de captura de alarmes
	hThreadAlarme = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncAlarme,	// casting necessário
		(LPVOID)Alarme,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necessário
	);

	//Tarefa de captura de dados
	hThreadDados = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)FuncDados,	// casting necessário
		(LPVOID)Dados,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necessário
	);

	//Tarefa AUX
	hThreadAUX = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)ConsomeStackPrincipal,	// casting necessário
		(LPVOID)iAUX,
		0,
		(CAST_LPDWORD)&dwThreadId	// cating necessário
	);

	Sleep(1000);


	HANDLE hEvents[5] = { hEventA, hEventB, hEventC, hEventD, hEventESC };
	DWORD Ret;
	while (1) {
		Ret = WaitForMultipleObjects(5, hEvents, FALSE, INFINITE); //Espera qualquer evento
		int i = Ret - WAIT_OBJECT_0;
		if (i != 4) {
			if (Interruptores[i] == 0) {
				Interruptores[i] = 1;
				SetEvent(hInts[i]);
				cout << "\nTAREFA " << i << " DESBLOQUEADA\n";
			}
			else {
				Interruptores[i] = 0;
				ResetEvent(hInts[i]);
				cout << "\nTAREFA " << i << " BLOQUEADA\n";
			}
		}
		else {
			Interruptores[4] = 1;
			cout << "\n  _ESC reconhecido \n";
			for (int j = 0; j < 4; j++) { //Reseta todos os interruptores
				ResetEvent(hInts[j]);
			}
			WaitForMultipleObjects(4, hInts, TRUE, INFINITE);
			break;
		}
	}

	for (int t = 0; t < 7; ++t) {
		GetExitCodeThread(hThread[t], &dwExitCode);
		printf("thread %d terminou: codigo=%d\n", t, dwExitCode);
		CloseHandle(hThread[t]);	// apaga referência ao objeto
	}

	CloseHandle(hThread);
	CloseHandle(hEventA);
	CloseHandle(hEventB);
	CloseHandle(hEventC);
	CloseHandle(hEventD);
	CloseHandle(hEventESC);
	CloseHandle(hEvents);
	CloseHandle(hMutexArquivo);

	for (int k = 0; k < 4; k++) {
		CloseHandle(hInts[k]);
	}

	CloseHandle(hInts);
	CloseHandle(hMutex1);
	CloseHandle(hMutexA);
	CloseHandle(hMutexCLP);
	CloseHandle(pipe);
	return EXIT_SUCCESS;
}


DWORD WINAPI FuncPesagem(LPVOID id)
{
	HANDLE Events[2] = { hEventNFull, hMutex1 };
	string ORIGEM = "00";
	do {
		for (int p = 0; p <= 999999 && !Interruptores[4]; p++) {

			// Strings para receber os codigos de alarmes definidos
			//Numero de sequencia 0 a 999999
			string NSEQ = to_string(p);
			NSEQ.insert(0, 6 - NSEQ.size(), '0');

			//Codigo de 0 a 99
			string CODIGO = to_string(rand() % 99);
			CODIGO.insert(0, 2 - CODIGO.size(), '0');

			//Tempo do sistema
			char tbuffer[9];
			_strtime(tbuffer);
			string TIMESTAMP = tbuffer;

			//Mensagem completa do alarme 
			string msgPesagem = NSEQ + "#" + ORIGEM + "#" + CODIGO + "#" + TIMESTAMP;

			WaitForSingleObject(hInts[1], INFINITE); //Bloqueia se Sinalizador não-sinalizado	
			WaitForMultipleObjects(2, Events, TRUE, INFINITE);
			// Secao critica
			if (sizestack <= 200) { // Se nao cheia, coloca alarme de pesagem
				push(msgPesagem);
				if (!isempty()) {
					showTop();
				}
			}
			ReleaseMutex(hMutex1);

			WaitForSingleObject(hTimeOut, 1000 * (rand() % 5 + 1)); //Temporizador

		}
	} while (!Interruptores[4]);
	_endthreadex(0);
	return(0);
}

DWORD WINAPI FuncCLPalarme(LPVOID id) //Alarme proveniente do clp, mesmo formato de mensagem do alarme de peso
{
	HANDLE Events[2] = { hEventNFull, hMutex1 };
	string ORIGEM = "55";
	do {
		for (int p = 0; p <= 999999 && !Interruptores[4]; p++) {

			// Strings para receber os codigos de alarmes definidos
			//Numero de sequencia 0 a 999999
			string NSEQ = to_string(p);
			NSEQ.insert(0, 6 - NSEQ.size(), '0');

			//Codigo de 0 a 99
			string CODIGO = to_string(rand() % 99);
			CODIGO.insert(0, 2 - CODIGO.size(), '0');

			//Tempo do sistema
			char tbuffer[9];
			_strtime(tbuffer);
			string TIMESTAMP = tbuffer;

			//Mensagem completa do alarme 
			string msgAlarme = NSEQ + "#" + ORIGEM + "#" + CODIGO + "#" + TIMESTAMP + "       ";

			WaitForSingleObject(hInts[2], INFINITE); //Bloqueia se interruptor não-sinalizado
			WaitForMultipleObjects(2, Events, TRUE, INFINITE);
			if (sizestack <= 200) {  // Se nao vazia, coloca indicador 55 de alarme pesagem
				push(msgAlarme);
				if (!isempty()) {
					showTop();
				}
			}
			ReleaseMutex(hMutex1);

			WaitForSingleObject(hTimeOut, 1000 * (rand() % 5 + 1)); //Temporizador
		}
	} while (!Interruptores[4]);
	_endthreadex(0);
	return(0);
}

DWORD WINAPI FuncCLPdado(LPVOID id)
{
	HANDLE Events[2] = { hEventNFull, hMutex1 };
	string ORIGEM = "99";
	do {
		for (int p = 0; p <= 999999 && !Interruptores[4]; p++) {

			// Strings para receber os codigos de alarmes definidos
			//Numero de sequencia 0 a 999999
			string NSEQ = to_string(p);
			NSEQ.insert(0, 6 - NSEQ.size(), '0');

			//Valor da velocidade 
			float t = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 999.9));
			stringstream ss;
			ss << fixed << setprecision(1) << t;
			string VEL = ss.str();
			VEL.insert(0, 5 - VEL.size(), '0');

			//Estado de 0 ou 1
			string SENSORIC = to_string(rand() % 2);
			string SENSORFC = to_string(rand() % 2);

			//Tempo do sistema
			char tbuffer[9];
			_strtime(tbuffer);
			string TIMESTAMP = tbuffer;

			//Mensagem completa do alarme 
			string msgDado = NSEQ + "#" + ORIGEM + "#" + VEL + "#" + SENSORIC + "#" + SENSORFC + "#" + TIMESTAMP;

			WaitForSingleObject(hInts[2], INFINITE); //Bloqueia se interruptor não-sinalizado
			WaitForMultipleObjects(2, Events, TRUE, INFINITE);
			if (sizestack <= 200) {// Se nao vazia, coloca indicador 55 de alarme pesagem
				push(msgDado);
				if (!isempty()) {
					showTop();
				}

			}
			ReleaseMutex(hMutex1);

			WaitForSingleObject(hTimeOut, 500); //Temporizador
		}
	} while (!Interruptores[4]);
	_endthreadex(0);
	return(0);
}

DWORD WINAPI FuncAlarme(LPVOID id)
{
	while (1) // Espera conexão
	{
		pipe = CreateFile(
			"\\\\.\\pipe\\myPipe",
			GENERIC_READ |  // acesso para leitura e escrita 
			GENERIC_WRITE, // only need read access
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		// Se o handle é válido pode usar o pipe
			if (pipe != INVALID_HANDLE_VALUE)
				break;

		// Todas as instancias estão ocupadas, então espere pelo tempo default 
			/*if (WaitNamedPipe("\\\\.\\pipe\\my_pipe", NMPWAIT_USE_DEFAULT_WAIT) == 0)
			cout<<"\nEsperando por uma instancia do pipe..."; // Temporização abortada: o pipe ainda não foi criado*/
	}
	
	do {
		WaitForSingleObject(hInts[0], INFINITE); //Bloqueia se interruptor não sinalizado
		WaitForSingleObject(hMutexA, INFINITE);
		if (!isemptyA() && !Interruptores[4]) {

			showTopA();
			
			const char* char_msg = topoA.c_str();
			int nChars = MultiByteToWideChar(CP_ACP, 0, char_msg, -1, NULL, 0);
			const WCHAR* mensagem;
			mensagem = new WCHAR[nChars];
			MultiByteToWideChar(CP_ACP, 0, char_msg, -1, (LPWSTR)mensagem, nChars);

			DWORD numBytesWritten = 0;
			BOOL result = WriteFile(
				pipe, // handle to our outbound pipe
				char_msg, // data to send
				wcslen(mensagem) * sizeof(wchar_t), // length of data to send (bytes)
				&numBytesWritten, // will store actual amount of data sent
				NULL // not using overlapped IO
			);

			if (result) {
				wcout << "Number of bytes sent: " << numBytesWritten << endl;
				wcout << "Message sent: " << char_msg << endl;

			}
			else {
				wcout << "Failed to send data." << endl;
				// look up error code here using GetLastError()
			}
			
			popA();
		}
		ReleaseMutex(hMutexA);

	} while (!Interruptores[4]);
	_endthreadex(0);
	return(0);
}

DWORD WINAPI FuncDados(LPVOID id) //Captura os dados do processo da lista circular e envia para a tarefa "ExibeDados"
{
	hFile = CreateFile("processo.txt",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, // abre para leitura e escrita
		NULL,		// atributos de segurança
		OPEN_ALWAYS,// cria novo arquivo caso ele não exista, abre se já existe
		FILE_ATTRIBUTE_NORMAL,
		NULL);		// Template para atributos e flags
	if (hFile == INVALID_HANDLE_VALUE)  std::cerr << "\nErro na criação do arquivo = " << GetLastError() << "\n";
	
	int indice = 0;
	WaitForSingleObject(hTimeOut, 20);
	do {
		WaitForSingleObject(hInts[3], INFINITE); //Bloqueia se interruptor não sinalizado
		WaitForSingleObject(hMutexCLP, INFINITE);
		if (!isemptyCLP() && !Interruptores[4]) {
			showTopCLP();

			// Transforma a string em char
			const char* char_dado = topoCLP.c_str(); //tamanho 28

			//Escreve no arquivo
			WaitForSingleObject(hMutexArquivo, INFINITE);
			bStatus = WriteFile(hFile, char_dado, 28, &dwBytesWritten, NULL);
			if (bStatus == 0)  std::cerr << "\nErro na escrita de Dados = " << GetLastError() << "\n";

			// Atualiza a posição para leitura
			if (indice == 10) { indice = 0;  SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
			};
			indice += 1;
			ReleaseMutex(hMutexArquivo);
		}
		ReleaseMutex(hMutexCLP);
		WaitForSingleObject(hTimeOut, 500);

	} while (!Interruptores[4]);
	_endthreadex(0);
	return(0);
}

// Consome Stack principal e deposita nas respectivas stacks de alarme de pesagem ou clp
DWORD WINAPI ConsomeStackPrincipal(LPVOID id) {
	do {

		if (!isempty()) {

			if (topo.compare(7, 2, "55") == 0 || topo.compare(7, 2, "00") == 0) {
				if (sizestackA <= 100) {
					WaitForSingleObject(hMutex1, INFINITE);
					WaitForSingleObject(hMutexA, INFINITE);
					pushA(topo);
					showTopA();
					pop();
					ReleaseMutex(hMutex1);
					ReleaseMutex(hMutexA);
				}
			}
			else if (topo.compare(7, 2, "99") == 0) {
				if (sizestackCLP <= 100) {
					WaitForSingleObject(hMutex1, INFINITE);
					WaitForSingleObject(hMutexCLP, INFINITE);
					pushCLP(topo);
					showTopCLP();
					pop();
					ReleaseMutex(hMutex1);
					ReleaseMutex(hMutexCLP);
				}
			}
		}
	} while (!Interruptores[4]);

	return (0);
}


/*WriteFile(hFile, topoCLP.c_str(), sizeof(topoCLP.c_str()), &dwBytesWritten, NULL);
			lFilePosLow = indice * sizeof(topoCLP.c_str());
			indice+=1;
			SetFilePointer(hFile, lFilePosLow, NULL, FILE_BEGIN);

			//printf("Numero de bytes escritos = %d\n", dwBytesWritten);
			string LeArquivo;
			printf("File Pointer = %d\n", lFilePosLow);
			bStatus = ReadFile(hFile, &LeArquivo, sizeof(topoCLP.c_str()), &dwBytesRead, NULL);
			if (bStatus == 0)  std::cerr << "\nErro na abertura Exibe Dados = " << GetLastError() << "\n";
			cout << "\nFoi lido do arquivo: " << LeArquivo << endl;
			printf("Numero de bytes lidos = %d\n", &dwBytesRead);*/
