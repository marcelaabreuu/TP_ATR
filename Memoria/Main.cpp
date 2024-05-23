#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <conio.h>  // _getch

int main()
{
    BOOL status;
    STARTUPINFO si;                  // StartUpInformation para novo processo
    PROCESS_INFORMATION NewProcess;  // Informa��es sobre novo processo criado

    SetConsoleTitle("Processo principal de tratamento do teclado");
    std::cout << "Digite uma tecla qualquer para criar uma instancia:\n";
    _getch();

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
        &NewProcess);        // lpProcessInformation

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
        &NewProcess);        // lpProcessInformation

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
        &NewProcess);        // lpProcessInformation

    if (!status)
        std::cerr << "Erro na abertura Exibe Dados = " << GetLastError() << "\n";

    //Espera do processo ser aberto 
    WaitForSingleObject(NewProcess.hProcess, INFINITE);

    // Fechar handles
    CloseHandle(NewProcess.hProcess);
    CloseHandle(NewProcess.hThread);

    return EXIT_SUCCESS;
}
