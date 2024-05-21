//
//	Programa��o Multithreaded em ambiente Windows NT� - uma vis�o de  Automa��o
// 
//	Autores: Constantino Seixas Filho/ Marcelo Szuster
//
//	Fun��o bGetString: Entrada de dados de string
//
//	Vers�o: 1.1	20/01/1999
//

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include <ctype.h>		// _isdigit
#include <conio.h>		// _getch
#include <stdio.h>

#define	ESC		0x1B
#define CR		0x0D
#define BS		0x08

BOOL bGetString(char *Str, int nMaxDig)
{	
	char Palavra[128]=""; 
	int nNumDig = 0;
	int ch;

	do {
		ch=_getch();
		if (isprint(ch)) {
			printf("%c", ch);	// ecoa
			Palavra[nNumDig]= (char) ch;
			nNumDig++;			// incrementa n�mero de d�gitos
		} // if

		else {
				if (ch == ESC) return FALSE;
				if (ch == CR) {
					if (nNumDig > 0) break;
					else return FALSE;  // N�o digitou nada
				}

				if ((ch == BS) && (nNumDig>=1)) {
					printf("%c%c%c", BS, ' ', BS); // apaga �ltimo caracter
					Palavra[--nNumDig] = (char) ch;
				}
			}
	} while (nNumDig<=nMaxDig);

	strcpy(Str, Palavra);
	return TRUE;
	
}	// bGetString


