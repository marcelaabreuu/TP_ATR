#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <conio.h>  // _getch
#include <conio.h>		// _getch

int main() 
{
	SetConsoleTitle("Console Alarmes");
	std::cout << "Exibi��o de Alarmes em Execu��o";
	_getch();
	return EXIT_SUCCESS;
}