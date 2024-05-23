#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <conio.h>  // _getch

int main()
{
    BOOL status;
    STARTUPINFO si;                  // StartUpInformation para novo processo
    PROCESS_INFORMATION NewProcess;  // Informações sobre novo processo criado

    SetConsoleTitle("Processo principal de tratamento do teclado");
    std::cout << "Digite uma tecla qualquer para criar uma instancia:\n";
    _getch();

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);  // Tamanho da estrutura em bytes
    ZeroMemory(&NewProcess, sizeof(NewProcess));
    // Processo de leitura de teclado

    //const char* programName = "C:\\Windows\\System32\\cmd.exe";

    status = CreateProcess(
        "..\\x64\\Debug\\ListaCircular.exe",         // Caminho do arquivo executável
        NULL,                // Apontador p/ parâmetros de linha de comando
        NULL,                // Apontador p/ descritor de segurança
        NULL,                // Idem, threads do processo
        FALSE,               // Herança de handles
        CREATE_NEW_CONSOLE,  // Flags de criação
        NULL,                // Herança do ambiente de execução
        "..\\x64\\Debug",                // Diretório do arquivo executável
        &si,                 // lpStartUpInfo
        &NewProcess);        // lpProcessInformation

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
        "..\\x64\\Debug",                // Diretório do arquivo executável
        &si,                 // lpStartUpInfo
        &NewProcess);        // lpProcessInformation

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
