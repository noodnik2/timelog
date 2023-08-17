/*	Revision:	1
*/
/*

	tlrptvar.h

*/

#ifndef	TLRPTVAR_H_INC
#define	TLRPTVAR_H_INC

#include	<time.h>
#include	"tlrptdlg.h"

typedef struct tagDLGVAR {
	char szUserName[MAXSIZE_USERNAME+1] ;
	BOOL bUpdate ;
} DLGVAR, FAR *LPDLGVAR ;

BOOL tlrptvar_InvokeDialog(HWND hWndParent, LPDLGVAR lpdlgvar) ;

/* #ifndef TLRPTVAR_H_INC */
#endif

/* end of tlrptvar.h */
