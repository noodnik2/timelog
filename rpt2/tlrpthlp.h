/*	Revision:	5
*/

/*
	tlrpthlp.h
	
*/

#ifndef	TLRPTHLP_H_INC
#define	TLRPTHLP_H_INC

#define	MAXSIZE_HELPFILENAME		128

typedef	long TRHELPREF ;
#define	TRHELPREF_NONE			((TRHELPREF) ~0)

#include "help/timerpt.map"

LRESULT _export CALLBACK tlrpthlp_HelpKeyHook(int code, WPARAM wParam, LPARAM lParam) ;
BOOL tlrpthlp_Init(void) ;
void tlrpthlp_SetHelpMsg(HWND hWnd, UINT helpmsg) ;
void tlrpthlp_Fini(void) ;
TRHELPREF tlrpthlp_SetHelpRef(TRHELPREF lRef) ;
TRHELPREF tlrpthlp_GetHelpRef(void) ;

/* #ifndef TLRPTHLP_H_INC */
#endif

/* end of tlrpthlp.h */
