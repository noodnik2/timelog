#define	Revision	23

/*

	tlrptts.c

*/

#include	"wininc.h"

#include	<stdio.h>		/* for sscanf */

#include	"resource.h"
#include	"tlrptts.h"

BOOL CALLBACK __export tlrptts_DialogProc(HWND, UINT, WPARAM, LPARAM) ;

static BOOL lclInitDialogTS(HWND hDlgTS, LPDLGTS lpdlgts) ;
static BOOL lclCommandDialogTS(HWND, LPDLGTS, WPARAM, LPARAM) ;
static BOOL lclDlgTimeSet(HWND hDlg, time_t tTime) ;
static BOOL lclDlgEditSetStr(HWND hWndEdit, LPSTR lpszStr) ;
static BOOL lclDlgEditSetNumber(HWND hWndEdit, int number) ;
static BOOL lclDlgEditGetNumber(HWND hWndEdit, int *pnumber) ;
static BOOL lclScrollDialogTS(HWND hDlgTS, HWND hDlgCtrl, WPARAM, LPDLGTS) ;

BOOL tlrptts_InvokeDialog(HWND hWndParent, LPDLGTS lpdlgts)
{
	DLGPROC lpfntlrptts_DialogProc ;
	int dlgrc ;
	TRHELPREF hrOld ;

	lpdlgts->bUpdate= FALSE ;

	lpfntlrptts_DialogProc= (DLGPROC) MakeProcInstance(
			(FARPROC) tlrptts_DialogProc,
			hInst
	) ;
	hrOld= tlrpthlp_SetHelpRef(REF_TIMESPANCUTOFF) ;
	dlgrc= DialogBoxParam(
		hInst,
		"DLGTS",
		hWndParent,
		lpfntlrptts_DialogProc,
		(LPARAM) lpdlgts
	) ;
	tlrpthlp_SetHelpRef(hrOld) ;
	FreeProcInstance((FARPROC) lpfntlrptts_DialogProc) ;

	if (dlgrc< 0) {
		timerpt_ErrorMsg(hWndParent, "TimeSpan Dialog Error", dlgrc) ;
		return(FALSE) ;
	}

	if (dlgrc == IDOK) lpdlgts->bUpdate= TRUE ;

	return(TRUE) ;
}

BOOL __export CALLBACK tlrptts_DialogProc(
	HWND hDlgTS,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	static LPDLGTS lpdlgts ;

#if	DEBUG
	fprintf(fpdbg, "tlrptts: message=0x%04X\n", message) ;
#endif
	switch(message) {

		case WM_INITDIALOG:
			lpdlgts= (LPDLGTS) lParam ;
			lclInitDialogTS(hDlgTS, lpdlgts) ;
			return(TRUE) ;
//			return(FALSE) ;
			break ;

		case WM_VSCROLL:
		case WM_HSCROLL:
			lclScrollDialogTS(
				hDlgTS,
				(HWND) HIWORD(lParam),
				wParam,
				lpdlgts
			) ;
			return(TRUE) ;
			break ;
				
		case WM_COMMAND:
			return(
				lclCommandDialogTS(
					hDlgTS,
					lpdlgts,
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

static BOOL lclInitDialogTS(HWND hDlgTS, LPDLGTS lpdlgts)
{
	lclDlgTimeSet(hDlgTS, lpdlgts->tStop) ;
	lclDlgEditSetNumber(
		GetDlgItem(hDlgTS, IDC_EDITPERIODS),
		lpdlgts->nPeriods
	) ;
	return(TRUE) ;
}

static BOOL lclReadDialogTS(HWND hDlgTS, LPDLGTS lpdlgts)
{
	lclDlgEditGetNumber(
		GetDlgItem(hDlgTS, IDC_EDITPERIODS),
		&lpdlgts->nPeriods
	) ;
	return(TRUE) ;
}

/*
	function group: handle TS dialog commands
*/

static BOOL lclCommandDialogTS(
	HWND hDlgTS,
	LPDLGTS lpdlgts,
	WPARAM wParam,
	LPARAM lParam
) {
#if	DEBUG
	fprintf(fpdbg, "tlrptts: command=0x%04X\n", wParam) ;
#endif
	switch(wParam) {

		case	IDOK:
		case	IDCANCEL:
			lclReadDialogTS(hDlgTS, lpdlgts) ;
			EndDialog(hDlgTS, (int) wParam) ;
			return(TRUE) ;
			break ;

	}

	return(FALSE) ;
}
		
/*
	function group: handle TS dialog controls
*/

static BOOL lclDlgEditSetStr(HWND hWndEdit, LPSTR lpszStr)
{
	SetWindowText(hWndEdit, lpszStr) ;
	SendMessage(hWndEdit, EM_SETSEL, 0, MAKELONG(0, lstrlen(lpszStr))) ;
	return(TRUE) ;
}

static BOOL lclDlgEditSetNumber(HWND hWndEdit, int number)
{
	char szNumber[16] ;

	wsprintf(szNumber, "%d", number) ;
	return(lclDlgEditSetStr(hWndEdit, szNumber)) ;
}

static BOOL lclDlgEditGetNumber(HWND hWndEdit, int *piNumber)
{
	char szNumber[MAXSIZE_EDITNUMBER+1] ;

	GetWindowText(hWndEdit, szNumber, MAXSIZE_EDITNUMBER+1) ;
	return(sscanf(szNumber, "%d", piNumber) == 1) ;
}

static BOOL lclDlgTimeSet(HWND hDlg, time_t tTime)
{
	char szPeriodName[MAXSIZE_TIMESCOPE+1] ;

	timeutil_szLoad(tTime, TUL_MONTH, szPeriodName) ;
	lclDlgEditSetStr(GetDlgItem(hDlg, IDC_EDITMONTH), szPeriodName) ;

	timeutil_szLoad(tTime, TUL_MDAY, szPeriodName) ;
	lclDlgEditSetStr(GetDlgItem(hDlg, IDC_EDITDAY), szPeriodName) ;

	timeutil_szLoad(tTime, TUL_YEAR, szPeriodName) ;
 	lclDlgEditSetStr(GetDlgItem(hDlg, IDC_EDITYEAR), szPeriodName) ;

	timeutil_szLoad(tTime, TUL_HOUR, szPeriodName) ;
	lclDlgEditSetStr(GetDlgItem(hDlg, IDC_EDITHOUR), szPeriodName) ;

	timeutil_szLoad(tTime, TUL_MINUTE, szPeriodName) ;
	lclDlgEditSetStr(GetDlgItem(hDlg, IDC_EDITMINUTE), szPeriodName) ;
	return(TRUE) ;
}

static BOOL lclDlgTimeUpdate(
	HWND hDlg,
	HWND hDlgCtrl,
	int nTimeAdj,
	time_t FAR *lptTime
) {
	int tut_id ;

	if (hDlgCtrl == GetDlgItem(hDlg, IDC_SCROLLMINUTE)) {
		tut_id= TUT_MINUTE ;
	}
	else if (hDlgCtrl == GetDlgItem(hDlg, IDC_SCROLLHOUR)) {
		tut_id= TUT_HOUR ;
	}
	else if (hDlgCtrl == GetDlgItem(hDlg, IDC_SCROLLDAY)) {
		tut_id= TUT_DAY ;
	}
	else if (hDlgCtrl == GetDlgItem(hDlg, IDC_SCROLLMONTH)) {
		tut_id= TUT_MONTH ;
	}
	else if (hDlgCtrl == GetDlgItem(hDlg, IDC_SCROLLYEAR)) {
		tut_id= TUT_YEAR ;
	}
	else {
#if	DEBUG
		fprintf(fpdbg, "lclDlgTimeUpdate: unknown control!\n") ;
		MessageBeep(0) ;
#endif
		return(FALSE) ;		/* unknown control */
	}

	*lptTime= timeutil_tTime(*lptTime, tut_id, nTimeAdj) ;
	lclDlgTimeSet(hDlg, *lptTime) ;
	return(TRUE) ;
}

static BOOL lclScrollDialogTS(
	HWND hDlgTS,
	HWND hDlgCtrl,
	WPARAM wScrollCode,
	LPDLGTS lpdlgts
) {
	HWND hWndEdit ;
	int nPeriods ;

	if (hDlgCtrl == GetDlgItem(hDlgTS, IDC_SCROLLPERIODS)) {
		hWndEdit= GetDlgItem(hDlgTS, IDC_EDITPERIODS) ;
		lclDlgEditGetNumber(hWndEdit, &nPeriods) ;
		switch(wScrollCode) {
			case	SB_LINEUP:
				if (nPeriods> 1) nPeriods-- ;
				else MessageBeep(0) ;
				break ;
			case	SB_LINEDOWN:
				nPeriods++ ;
				break ;
			default:
				return(FALSE) ;	/* unsupported action */
				break ;
		}
		lclDlgEditSetNumber(hWndEdit, nPeriods) ;
		return(TRUE) ;
	}

	switch(wScrollCode) {
		case	SB_LINEUP:	nPeriods= -1 ;		break ;
		case	SB_LINEDOWN:	nPeriods= 1 ;		break ;
		default:		return(FALSE) ;		break ;
	}
	lclDlgTimeUpdate(hDlgTS, hDlgCtrl, nPeriods, &lpdlgts->tStop) ;

	return(TRUE) ;
}

/* end of tlrptts.c */
