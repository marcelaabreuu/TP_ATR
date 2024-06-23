#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <iostream>
#include <conio.h>  // _getch
#include <stdio.h>
#include <stdlib.h>

#define	ESC			0x1B

HANDLE hEventA, hEventB, hEventC, hEventD, hEvent1, hEvent2, hEventESC;

int main()
{
    BOOL status;
    STARTUPINFO si;                  // StartUpInformation para novo processo
    PROCESS_INFORMATION NewProcess[3];  // Informações sobre novo processo criado


    SetConsoleTitle("Console Principal");

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);  // Tamanho da estrutura em bytes
    ZeroMemory(&NewProcess, sizeof(NewProcess));
   
    //ListaCircular
    status = CreateProcess(
        "..\\x64\\Debug\\ListaCircular.exe",         // Caminho do arquivo executável
        NULL,                // Apontador p/ parâmetros de linha de comando
        NULL,                // Apontador p/ descritor de segurança
        NULL,                // Idem, threads do processo
        FALSE,               // Herança de handles
        0,                   // Flags de criação
        NULL,                // Herança do ambiente de execução
        "..\\x64\\Debug",    // Diretório do arquivo executável
        &si,                 // lpStartUpInfo
        &NewProcess[0]);     // lpProcessInformation

    if (!status)
        std::cerr << "Erro na abertura cmd teclado = " << GetLastError() << "\n";

    //Criação de Console de exibição de Alarmes
    status = CreateProcess(
        "..\\x64\\Debug\\ExibeAlarmes.exe",         // Caminho do arquivo executável
        NULL,                // Apontador p/ parâmetros de linha de comando
        NULL,                // Apontador p/ descritor de segurança
        NULL,                // Idem, threads do processo
        FALSE,               // Herança de handles
        CREATE_NEW_CONSOLE,  // Flags de criação
        NULL,                // Herança do ambiente de execução
        "..\\x64\\Debug",    // Diretório do arquivo executável
        &si,                 // lpStartUpInfo
        &NewProcess[1]);     // lpProcessInformation

    if (!status)
        std::cerr << "Erro na abertura Exibe Alarmes = " << GetLastError() << "\n";

    //Criação de Console de exibição de Dados
    status = CreateProcess(
        "..\\x64\\Debug\\ExibeDados.exe",         // Caminho do arquivo executável
        NULL,                // Apontador p/ parâmetros de linha de comando
        NULL,                // Apontador p/ descritor de segurança
        NULL,                // Idem, threads do processo
        FALSE,               // Herança de handles
        CREATE_NEW_CONSOLE,  // Flags de criação
        NULL,                // Herança do ambiente de execução
        "..\\x64\\Debug",                // Diretório do arquivo executável
        &si,                 // lpStartUpInfo
        &NewProcess[2]);        // lpProcessInformation

    if (!status)
        std::cerr << "Erro na abertura Exibe Dados = " << GetLastError() << "\n";

    //Espera todos os processos serem abertos 
    WaitForMultipleObjects(3, &NewProcess->hProcess, TRUE, INFINITE);

    //Cria os eventos que acordam as threads
    hEventA = CreateEvent(NULL, FALSE, FALSE, "CapturaAlarmes");  //Reset automático e inicializa não-sinalizado
    hEventB = CreateEvent(NULL, FALSE, FALSE, "Pesagem");
    hEventC = CreateEvent(NULL, FALSE, FALSE, "LeituraCLP");
    hEventD = CreateEvent(NULL, FALSE, FALSE, "CapturaDados");
    hEvent1 = CreateEvent(NULL, FALSE, FALSE, "Alarme");
    hEvent2 = CreateEvent(NULL, FALSE, FALSE, "Dados");
    hEventESC = CreateEvent(NULL, TRUE, FALSE, "ESC");            //Reset manual e inicializa não-sinalizado

    std::cout << "Selecione:\n";
    std::cout << "a: Iniciar/Pausar captura de alarmes\n";
    std::cout << "b: Iniciar/Pausar sistema de pesagem\n";
    std::cout << "c: Iniciar/Pausar leitura do CLP\n";
    std::cout << "d: Iniciar/Pausar captura de dados\n";
    std::cout << "1: Iniciar/Pausar exibicao de alrmes\n";
    std::cout << "2: Iniciar/Pausar exibicao de dados\n";

    char action;

    do {
        action = _getch();
        std::cout << "\nFoi selecionado: " << action << std::endl;
        
        switch (action) {
        case '1':
            SetEvent(hEvent1);
            break;
        case '2':
            SetEvent(hEvent2);
            break;
        case 'a':
            SetEvent(hEventA);
            break;
        case 'b':
            SetEvent(hEventB);
            break;
        case 'c':
            SetEvent(hEventC);
            break;
        case 'd':
            SetEvent(hEventD);
            break;
        default:
                break;
        }
    } while (action != ESC);

    SetEvent(hEventESC);

    CloseHandle(hEventA);
    CloseHandle(hEventB);
    CloseHandle(hEventC);
    CloseHandle(hEventD);
    CloseHandle(hEvent1);
    CloseHandle(hEvent2);
    CloseHandle(hEventESC);

    // Fechar handles dos processos
    for (int i = 0; i < 3; i++) {
        
        CloseHandle(NewProcess[i].hProcess);
    }

    return EXIT_SUCCESS;
}
