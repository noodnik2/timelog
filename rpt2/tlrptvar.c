#define	Revision	7

/*

	tlrptvar.c

*/

#include	"wininc.h"

#include	<stdio.h>		/* for sscanf */

#include	"resource.h"
#include	"tlrptvar.h"

BOOL CALLBACK __export tlrptvar_DialogProc(HWND, UINT, WPARAM, LPARAM) ;

static BOOL lclInitDialogVAR(HWND hDlgVAR, LPDLGVAR lpdlgvar) ;
static BOOL lclDlgEditSetStr(HWND hWndEdit, LPSTR lpszStr) ;
static BOOL lclDlgEditGetStr(HWND hWndEdit, LPSTR lpszStr, int maxlen) ;
static BOOL lclCommandDialogVAR(HWND, LPDLGVAR, WPARAM, LPARAM) ;

BOOL tlrptvar_InvokeDialog(HWND hWndParent, LPDLGVAR lpdlgvar)
{
	DLGPROC lpfntlrptvar_DialogProc ;
	int dlgrc ;
	TRHELPREF hrOld ;

	lpdlgvar->bUpdate= FALSE ;

	lpfntlrptvar_DialogProc= (DLGPROC) MakeProcInstance(
		(FARPROC) tlrptvar_DialogProc,
		hInst
	) ;
	hrOld= tlrpthlp_SetHelpRef(REF_USERNAME) ;
	dlgrc= DialogBoxParam(
		hInst,
		"DLGVARS",
		hWndParent,
		lpfntlrptvar_DialogProc,
		(LPARAM) lpdlgvar
	) ;
	tlrpthlp_SetHelpRef(hrOld) ;
	FreeProcInstance((FARPROC) lpfntlrptvar_DialogProc) ;

	if (dlgrc< 0) {
		timerpt_ErrorMsg(hWndParent, "Report Var Dialog Error", dlgrc) ;
		return(FALSE) ;
	}

	if (dlgrc == IDOK) lpdlgvar->bUpdate= TRUE ;

	return(TRUE) ;
}

BOOL __export CALLBACK tlrptvar_DialogProc(
	HWND hDlgVAR,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	static LPDLGVAR lpdlgvar ;

#if	DEBUG
	fprintf(fpdbg, "tlrptvar: message=0x%04X\n", message) ;
#endif
	switch(message) {

		case WM_INITDIALOG:
			lpdlgvar= (LPDLGVAR) lParam ;
			lclInitDialogVAR(hDlgVAR, lpdlgvar) ;
			return(TRUE) ;
			break ;

		case WM_COMMAND:
			return(
				lclCommandDialogVAR(
					hDlgVAR,
					lpdlgvar,
					wParam,
					lParam
				)
			) ;
			break ;

	}
	return(FALSE) ;
}	

/*
	static internal functions
*/

static BOOL lclInitDialogVAR(HWND hDlgVAR, LPDLGVAR lpdlgvar)
{
	lclDlgEditSetStr(
		GetDlgItem(hDlgVAR, IDC_USERNAME),
		lpdlgvar->szUserName
	) ;
	return(TRUE) ;
}

static BOOL lclReadDialogVAR(HWND hDlgVAR, LPDLGVAR lpdlgvar)
{
	lclDlgEditGetStr(
		GetDlgItem(hDlgVAR, IDC_USERNAME),
		lpdlgvar->szUserName,
		MAXSIZE_USERNAME+1
	) ;
	return(TRUE) ;
}

/*
	function group: handle VAR dialog commands
*/

static BOOL lclCommandDialogVAR(
	HWND hDlgVAR,
	LPDLGVAR lpdlgvar,
	WPARAM wParam,
	LPARAM lParam
) {
#if	DEBUG
	fprintf(fpdbg, "tlrptvar: command=0x%04X\n", wParam) ;
#endif
	switch(wParam) {

		case	IDOK:
		case	IDCANCEL:
			lclReadDialogVAR(hDlgVAR, lpdlgvar) ;
			EndDialog(hDlgVAR, (int) wParam) ;
			return(TRUE) ;
			break ;

	}

	return(FALSE) ;
}
		
/*
	function group: handle VAR dialog controls
*/

static BOOL lclDlgEditSetStr(HWND hWndEdit, LPSTR lpszStr)
{
	SetWindowText(hWndEdit, lpszStr) ;
	SendMessage(hWndEdit, EM_SETSEL, 0, MAKELONG(0, lstrlen(lpszStr))) ;
	return(TRUE) ;
}

static BOOL lclDlgEditGetStr(HWND hWndEdit, LPSTR lpszStr, int maxlen)
{
	return(
		(BOOL) (GetWindowText(hWndEdit, lpszStr, maxlen)> 0)
	) ;
}

/* end of tlrptvar.c */
