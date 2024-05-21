//
//	Programa��o Multithreaded em ambiente Windows NT� - uma vis�o de  Automa��o
// 
//	Autores: Constantino Seixas Filho/ Marcelo Szuster
//
//	Fun��o bGetFloat: Entrada de dados de valor em Floating Point
//
//	Vers�o: 1.1	20/01/1999
//

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include <ctype.h>		// _isdigit
#include <conio.h>		// _getch
#include <stdio.h>

#define	ESC			0x1B
#define CR			0x0D
#define BS			0x08

BOOL bGetFloat(double *result, int iMaxDig)
{	
	DWORD dwNum = 0;
	DWORD dwDivFrac = 1;
	int iNumDig = 0;
	int ch;
	int Estado = 0; // Estado = 0: ponto n�o entrou.

	do {
		ch=_getch();
		if (isdigit(ch)) {
			printf("%c", ch); // ecoa
			dwNum = dwNum*10 + (ch - 0x30);
			iNumDig++;  // incrementa n�mero de d�gitos
			if (Estado == 1) dwDivFrac *= 10;
		} // if

		else {
			if (ch == ESC) return FALSE;
			if (ch == CR) {
				if (iNumDig > 0) break;
				else return FALSE;  // N�o digitou nada
			}

			if ((ch == BS) && (iNumDig>=1)) {
				printf("%c%c%c", BS, ' ', BS); // apaga �ltimo caracter
				
				if (Estado == 0) {
					dwNum /= 10;
					iNumDig--;
				}
				else if (dwDivFrac >1) {
							dwNum /= 10;
							iNumDig--;
							dwDivFrac /= 10;
						}
					  else Estado = 0;
			}
			
			if ((ch == '.') && (Estado == 0)) {
				Estado = 1;
				printf("%c", '.');
			}
		}	 
	} while (iNumDig<=iMaxDig);

	*result = (double)dwNum / dwDivFrac;
	return TRUE;
	
}	// bGetFloat


