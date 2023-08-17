#define	Revision	3

/*

	tlrptabt.c

*/

#include	"wininc.h"
#include	"resource.h"
#include	"tlrptabt.h"
#include	"tlrpthlp.h"

BOOL CALLBACK __export tlrptabt_DialogProc(HWND, UINT, WPARAM, LPARAM) ;

BOOL tlrptabt_InvokeDialog(HWND hWndParent)
{
	DLGPROC lpfntlrptabt_DialogProc ;
	TRHELPREF hrOld ;
	int dlgrc ;

	lpfntlrptabt_DialogProc= (DLGPROC) MakeProcInstance(
		(FARPROC) tlrptabt_DialogProc,
		hInst
	) ;
	hrOld= tlrpthlp_SetHelpRef(TRHELPREF_NONE) ;
	dlgrc= DialogBoxParam(
		hInst,
		"DLGABOUT",
		hWndParent,
		lpfntlrptabt_DialogProc,
		(LPARAM) 0
	) ;
	tlrpthlp_SetHelpRef(hrOld) ;
	FreeProcInstance((FARPROC) lpfntlrptabt_DialogProc) ;

	if (dlgrc< 0) {
		timerpt_ErrorMsg(hWndParent, "About Dialog Error", dlgrc) ;
		return(FALSE) ;
	}

	return(TRUE) ;
}

BOOL __export CALLBACK tlrptabt_DialogProc(
	HWND hDlgAbt,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
#if	DEBUG
	fprintf(fpdbg, "tlrptabt: message=0x%04X\n", message) ;
#endif
	switch(message) {

		case WM_INITDIALOG:
			return(TRUE) ;
			break ;

		case WM_COMMAND:
			switch(wParam) {	
				case	IDOK:
				case	IDCANCEL:
					EndDialog(hDlgAbt, (int) wParam) ;
					return(TRUE) ;
					break ;
			}
			break ;

	}
	return(FALSE) ;
}	

/*
	static internal functions
*/

/* end of tlrptabt.c */
