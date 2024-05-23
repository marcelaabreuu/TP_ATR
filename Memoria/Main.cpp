#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <conio.h>  // _getch

int main()
{
    BOOL status;
    STARTUPINFO si;                  // StartUpInformation para novo processo
    PROCESS_INFORMATION NewProcess[3];  // Informa��es sobre novo processo criado
    HANDLE hEventA, hEventB, hEventC, hEventD, hEvent1, hEvent2 ;

    SetConsoleTitle("Console Principal");

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);  // Tamanho da estrutura em bytes
    ZeroMemory(&NewProcess, sizeof(NewProcess));
    // Processo de leitura de teclado

    //const char* programName = "C:\\Windows\\System32\\cmd.exe";

    status = CreateProcess(
        "..\\x64\\Debug\\ListaCircular.exe",         // Caminho do arquivo execut�vel
        NULL,                // Apontador p/ par�metros de linha de comando
        NULL,                // Apontador p/ descritor de seguran�a
        NULL,                // Idem, threads do processo
        FALSE,               // Heran�a de handles
        CREATE_NEW_CONSOLE,  // Flags de cria��o
        NULL,                // Heran�a do ambiente de execu��o
        "..\\x64\\Debug",                // Diret�rio do arquivo execut�vel
        &si,                 // lpStartUpInfo
        &NewProcess[0]);        // lpProcessInformation

    if (!status)
        std::cerr << "Erro na abertura cmd teclado = " << GetLastError() << "\n";

    //Cria��o de Console de exibi��o de Alarmes
    status = CreateProcess(
        "..\\x64\\Debug\\ExibeAlarmes.exe",         // Caminho do arquivo execut�vel
        NULL,                // Apontador p/ par�metros de linha de comando
        NULL,                // Apontador p/ descritor de seguran�a
        NULL,                // Idem, threads do processo
        FALSE,               // Heran�a de handles
        CREATE_NEW_CONSOLE,  // Flags de cria��o
        NULL,                // Heran�a do ambiente de execu��o
        "..\\x64\\Debug",                // Diret�rio do arquivo execut�vel
        &si,                 // lpStartUpInfo
        &NewProcess[1]);        // lpProcessInformation

    if (!status)
        std::cerr << "Erro na abertura Exibe Alarmes = " << GetLastError() << "\n";

    //Cria��o de Console de exibi��o de Dados
    status = CreateProcess(
        "..\\x64\\Debug\\ExibeDados.exe",         // Caminho do arquivo execut�vel
        NULL,                // Apontador p/ par�metros de linha de comando
        NULL,                // Apontador p/ descritor de seguran�a
        NULL,                // Idem, threads do processo
        FALSE,               // Heran�a de handles
        CREATE_NEW_CONSOLE,  // Flags de cria��o
        NULL,                // Heran�a do ambiente de execu��o
        "..\\x64\\Debug",                // Diret�rio do arquivo execut�vel
        &si,                 // lpStartUpInfo
        &NewProcess[2]);        // lpProcessInformation

    if (!status)
        std::cerr << "Erro na abertura Exibe Dados = " << GetLastError() << "\n";

    //Espera todos os processos serem abertos 
    WaitForMultipleObjects(3, &NewProcess->hProcess, TRUE, INFINITE);

    
    std::cout << "Selecione:\n";
    std::cout << "a: Iniciar/Pausar captura de alrmes\n";
    std::cout << "b: Iniciar/Pausar sistema de pesagem de alrmes\n";
    std::cout << "c: Iniciar/Pausar leitura do CLP de alrmes\n";
    std::cout << "d: Iniciar/Pausar captura de dados\n";
    std::cout << "1: Iniciar/Pausar exibi��o de alrmes\n";
    std::cout << "2: Iniciar/Pausar exibi��o de dados\n";

    char action = _getch();
    std::cout << "\nFoi selecionado: " << action << std::endl;

    //Cria os eventos que acordam as threads
    
    hEvent1 = CreateEvent(NULL, FALSE, FALSE, "ExieAlarmes"); //Reset autom�tico e inicializa n�o-sinalizado
    hEvent2 = CreateEvent(NULL, FALSE, FALSE, "ExieDados"); 

    switch (action) {
    case '1':
        PulseEvent(hEvent1);
        break;
    case '2':
        PulseEvent(hEvent2);
        break;
    default:
        // Instru��es para o caso default (opcional)
    }

    // Fechar handles dos processos
    for (int i = 0; i < 3; i++) {
        CloseHandle(NewProcess[i].hProcess);
    }

    return EXIT_SUCCESS;
}
