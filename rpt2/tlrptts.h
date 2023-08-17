/*	Revision:	4
*/
/*

	tlrptts.h

*/

#ifndef	TLRPTTS_H_INC
#define	TLRPTTS_H_INC

#include	<time.h>
#include	"tlrptdlg.h"
#include	"timeutil.h"

typedef struct tagDLGTS {
	time_t tStop ;
	int nPeriods ;
	BOOL bUpdate ;
} DLGTS, FAR *LPDLGTS ;

BOOL tlrptts_InvokeDialog(HWND hWndParent, LPDLGTS lpdlgts) ;

/* #ifndef TLRPTTS_H_INC */
#endif

/* end of tlrptts.h */
