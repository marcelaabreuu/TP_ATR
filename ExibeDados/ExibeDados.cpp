#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <conio.h>  // _getch
#include <conio.h>		// _getch

int main()
{
	SetConsoleTitle("Console Dados");
	std::cout << "Exibi��o de Dados em Execu��o";
	_getch();
	return EXIT_SUCCESS;
}