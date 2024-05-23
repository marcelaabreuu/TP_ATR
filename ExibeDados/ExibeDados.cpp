#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <conio.h>  // _getch
#include <conio.h>		// _getch

int main()
{
	SetConsoleTitle("Console Dados");
	std::cout << "Processo ExibeDados";
	_getch();
	return EXIT_SUCCESS;
}