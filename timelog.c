#define	Revision	256

/*

	timelog.c			MS-Windows Time Log Utility
					Copyright (C) 1994, by Marty Ross

*/

/******************************************************************/

#define		DEBUG		0

#include	"wininc.h"

#include	<stdio.h>
#include	<malloc.h>
#include	<memory.h>
#include	<time.h>

#include	"resource.h"

#include	"tldlg.h"
#include	"scanlog.h"
#include	"logfile.h"
#include	"timeutil.h"

#define	MAXSIZE_RPTPROGNAME	MAXSIZE_SYSCMD
#define	MAXSIZE_HELPFILENAME	MAXSIZE_FILENAME

/******************************************************************/

HINSTANCE hInst= NULL ;

/******************************************************************/

#if	DEBUG
static FILE *fplog ;
#endif

static char szLogFile[MAXSIZE_LOGFILENAME+1]	;
static char szRptProg[MAXSIZE_RPTPROGNAME+1]	;
static char szHelpFile[MAXSIZE_HELPFILENAME+1]	;

static char *pszAppName=	"TimeLog"	;
static char *pszAppClass=	"ClsTimeLog"	;

static char *pszProfileSection=	"TimeLog"	;
static char *pszProfileFile=	"timelog.ini"	;
static char *pszHelpFileDefault="timelog.hlp"	;
static char *pszLogFileDefault=	"timelog.tlg"	;
static char *pszRptProgDefault=	"timerpt.exe"	;

static HWND hWndTop= NULL ;

static HOOKPROC lpfnHelpKeyHook ;
static HHOOK hhHelpKeyHook ;

static UINT uiUpdateTimes= 60000 ;      /* update each minute */ 
static int iMaxIniProjects= MAX_INIPROJECTS ;
static char *apszIniProjects[MAX_INIPROJECTS] ;

static BOOL bAutoDialog ;
static int iTimeDisplayFormat ;
static int iTimeScope ;

static time_t tReportPeriod_From ;
static time_t tReportPeriod_To ;

static char szOvhProjectName[MAXSIZE_PROJECTNAME+1]= { '\0' } ; ;
static char szCurrentProjectName[MAXSIZE_PROJECTNAME+1]= { '\0' } ; ;
static char szProjectName[MAXSIZE_PROJECTNAME+1]= { '\0' } ;

static time_t tProjectTimeStart= (time_t) NULL ;
static time_t tProjectTimeStop ;
static LONG lProjectPreviousSecs ;
static int iProjectHours, iProjectMinutes ;

static time_t tSessionTimeStart= (time_t) NULL ;
static time_t tSessionTimeStop ;
static LONG lSessionPreviousSecs ;
static int iSessionHours, iSessionMinutes ;

/******************************************************************/

LRESULT __export CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM) ;
BOOL __export CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM) ;
BOOL __export CALLBACK DialogNewProc(HWND, UINT, WPARAM, LPARAM) ;
BOOL __export CALLBACK DialogAdjProc(HWND, UINT, WPARAM, LPARAM) ;

/******************************************************************/

static BOOL lclInitApplication(HINSTANCE) ;
static BOOL lclInitInstance(LPSTR, int) ;
static BOOL lclFiniInstance(void) ;
static BOOL lclEventCreateWindow(HWND) ;
static BOOL lclEventDestroyWindow(HWND) ;
static BOOL lclEventEndSession() ;
static BOOL lclGetMessageProc(WPARAM *pexitcode) ;

static BOOL lclErrorMsg(HWND hWndParent, char *pszMsg, int rc) ;

#if 0
static BOOL lclMsg(HWND hWndParent, char *pszMsg) ;
static BOOL lclDlgComboSetStr(HWND hWndCombo, LPSTR lpszStr) ;
#endif

static BOOL lclDlgEditSetStr(HWND hWndEdit, LPSTR lpszStr) ;
static BOOL lclDlgEditSetNumber(HWND hWndEdit, int number) ;
static BOOL lclDlgEditGetNumber(HWND hWndEdit, int *piNumber) ;
static BOOL lclDlgTimeSet(HWND hDlg, time_t tTime) ;
static BOOL lclDlgTimeUpdate(HWND hDlg, HWND hDlgCtrl, int nA, time_t FAR *) ;
static BOOL lclDlgSetListSel(HWND hWndCtrl, LPSTR lpStr) ;
static BOOL lclDlgGetListSel(HWND hWndCtrl, LPSTR lpStr) ;
static BOOL lclDlgEditInit(HWND hWndEdit, LPSTR lpStr, int maxlen) ;

static BOOL lclInvokeDialog(HWND hWndParent) ;
static BOOL lclPerformDialog(HWND hWndParent, LPDLG lpdlg, int id) ;
static BOOL lclInitDialog(HWND hDlg, LPDLG lpdlg) ;
static BOOL lclStateDialog(HWND hDlg, LPDLG lpdlg) ;
static BOOL lclReadDialog(HWND hDlg, LPDLG lpdlg) ;
static BOOL lclAdjustDialog(HWND hDlg, LPDLG lpdlg) ;
static BOOL lclDeleteOvhDialog(HWND hDlg, LPDLG lpdlg) ;
static BOOL lclInitDialogCmt(HWND hDlgCmt, LPDLG lpdlg) ;

static BOOL lclInvokeDialogNew(HWND hWndParent, LPDLG lpdlg) ;
static BOOL lclInitDialogNew(HWND hDlgNew, LPDLGNEW lpdlgnew) ;
static BOOL lclReadDialogNew(HWND hDlgNew, LPDLGNEW lpdlgnew) ;
static BOOL lclCheckDialogNew(HWND hDlgNew, LPDLGNEW lpdlgnew) ;

static BOOL lclInvokeDialogRpt(HWND hWndParent) ;

static BOOL lclInvokeDialogAdj(HWND hWndParent, LPDLG lpdlg) ;
static BOOL lclReadDialogAdj(HWND hDlgAdj, LPDLGADJ lpdlgadj) ;
static BOOL lclInitDialogAdj(HWND hDlgAdj, LPDLGADJ lpdlgadj) ;
static BOOL lclScrollDialogAdj(HWND hD, HWND hC, WPARAM, LPDLGADJ) ;

static BOOL lclYesNo(HWND hWndParent, char *pszYesNoMsg) ;

static BOOL lclDoRecalc() ;
static void lclDoRepaint(HWND) ;
static BOOL lclDoReperiod() ;

static BOOL lclStartProjectName(LPSTR) ;
static void lclStopProject(void) ;
static BOOL lclStartSession(void) ;
static void lclStopSession(void) ;

static BOOL lclReadProfile(void) ;
static BOOL lclWriteProfile(void) ;

#if 0
static void lclSetFilePaths(void) ;
#endif

static BOOL lclDoHelp(HWND, long) ;
static long lclSetHelp(long lRef) ;
static long lclGetHelp(void) ;

static BOOL lclWriteLogEntry(
	time_t tTime,
	time_t tTime_a,
	long lval,
	int iaction,
	int icategory,
	char *pszText
) ;

static int lclDlgRBGget(HWND hDlg, int id0, int id1) ;
static void lclDlgRBGset(HWND hDlg, int id0, int id1, int idoff) ;

static BOOL lclDeleteProject(int iProject) ;
static BOOL lclPromoteProject(int iProject) ;
static BOOL lclAddProject(LPSTR lpszProject) ;
static BOOL lclGetProject(int iProject, LPSTR lpszProject) ;
static int lclFindProject(LPSTR lpszProject) ;
static int lclFindProjectName(LPSTR lpszProjectName) ;
static int lclGetFirstProject(LPSTR lpszProject) ;
static int lclGetNextProject(int iProject0, LPSTR lpszProject) ;

/******************************************************************/

int PASCAL WinMain(
	HINSTANCE hInstance,
	HINSTANCE hInstancePrev,
	LPSTR lpszCmdLine,
	int nCmdShow
) {
	WPARAM exitcode ;

#if	DEBUG
	fplog= fopen("timelog.log", "w") ;
#endif	

	hInst= hInstance ;
	if (!lclInitApplication(hInstancePrev)) {
		return(FALSE) ;
	}
	if (!lclInitInstance(lpszCmdLine, nCmdShow)) {
		return(FALSE) ;
	}
	while(lclGetMessageProc(&exitcode)) ;
	lclFiniInstance() ;
	
#if	DEBUG
	fclose(fplog) ;
#endif

	return(exitcode) ;
}

BOOL __export CALLBACK DialogProc(
	HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	WORD wVal ;
	static LPDLG lpdlg ;

	switch(message) {

		case WM_INITDIALOG:
			lpdlg= (LPDLG) lParam ;
			lclInitDialog(hDlg, lpdlg) ;
			return(TRUE) ;		/* we didn't set focus	*/
			break ;
			
		case UMD_RESTATE:
			lclStateDialog(hDlg, lpdlg) ;
			return(TRUE) ;
			break ;

		case WM_COMMAND:
			switch(wParam) {

				case	IDOK:
				case	IDOVERHEAD:
					lclReadDialog(hDlg, lpdlg) ;
					/*FALLTHRU*/
				case	IDCANCEL:
				case	IDEXIT:
					EndDialog(hDlg, (int) wParam) ;
					return(TRUE) ;
					break ;

				case	IDADJUST:
					lclAdjustDialog(hDlg, lpdlg) ;
					return(TRUE) ;
					break ;

				case	IDREPORT:
					lclInvokeDialogRpt(hDlg) ;
					return(TRUE) ;
					break ;

				case	IDNEWPROJECT:
					lclReadDialog(hDlg, lpdlg) ;
					if (lclInvokeDialogNew(hDlg, lpdlg)) {
						PostMessage(
							hDlg,
							UMD_RESTATE,
							0,
							0
						) ;
					}
					break ;

				case	IDDELOVERHEAD:
					lclDeleteOvhDialog(hDlg, lpdlg) ;
					break ;

				case	IDC_COMBOPROJECT:
				case	IDC_COMBOPROJECTOVH:
					wVal= HIWORD(lParam) ;
					if (wVal == CBN_SELCHANGE) {
						lclDlgGetListSel(
						   (HWND) LOWORD(lParam),
						   (wParam==IDC_COMBOPROJECT)?
						     lpdlg->szSelProjectName :
						     lpdlg->szOvhProjectName
						) ;
						return(TRUE) ;
					}
					if (wVal == CBN_SELENDOK) {
						PostMessage(
							hDlg,
							UMD_RESTATE,
							0,
							0
						) ;
						return(TRUE) ;
					}
					break ;

			}
			break ;
	}
	return(FALSE) ;
}	

BOOL __export CALLBACK DialogNewProc(
	HWND hDlgNew,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	static LPDLGNEW lpdlgnew ;
	switch(message) {

		case WM_INITDIALOG:
			lpdlgnew= (LPDLGNEW) lParam ;
			lclInitDialogNew(hDlgNew, lpdlgnew) ;
			return(FALSE) ;
			break ;

			break ;

		case WM_COMMAND:
			switch(wParam) {
				case	IDOK:
					lclReadDialogNew(hDlgNew, lpdlgnew) ;
					if (!lclCheckDialogNew(
						hDlgNew, lpdlgnew
					)) {
						return(FALSE) ;
					}
					/*FALLTHRU*/

				case	IDCANCEL:
					EndDialog(hDlgNew, (int) wParam) ;
					return(TRUE) ;
					break ;
			}
			break ;
	}
	return(FALSE) ;
}	

BOOL __export CALLBACK DialogAdjProc(
	HWND hDlgAdj,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	static LPDLGADJ lpdlgadj ;

	switch(message) {

		case WM_INITDIALOG:
			lpdlgadj= (LPDLGADJ) lParam ;
			lclInitDialogAdj(hDlgAdj, lpdlgadj) ;
			return(FALSE) ;
			break ;

		case WM_COMMAND:
			switch(wParam) {
				case	IDOK:
					lclReadDialogAdj(hDlgAdj, lpdlgadj) ;
					/*FALLTHRU*/
				case	IDCANCEL:
					EndDialog(hDlgAdj, (int) wParam) ;
					return(TRUE) ;
					break ;
			}
			break ;

		case WM_HSCROLL:
			lclScrollDialogAdj(
				hDlgAdj,
				(HWND) HIWORD(lParam),
				wParam,
				lpdlgadj
			) ;
			return(TRUE) ;
			break ;
	}
	return(FALSE) ;
}	

LRESULT __export CALLBACK WindowProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	switch(message) {

		case	WM_CREATE:
			if (!lclEventCreateWindow(hWnd)) {
				return(-1) ;
				break ;
			}
			return(0) ;
			break ;

		case	WM_DESTROY:
			lclEventDestroyWindow(hWnd) ;
			PostQuitMessage(0) ;
			return(0) ;
			break ;

		case	WM_ENDSESSION:
			if ((BOOL) wParam) {	/* if session ending */
				lclEventEndSession() ;
			}
			return(0) ;
			break ;

		case	WM_QUERYOPEN:
			PostMessage(hWnd, UM_DODIALOG, 0, 0) ;
			return(0) ;
			break ;
			
		case	WM_TIMER:
#if 0
			SendMessage(hWnd, UM_RECALC, 0, 0) ;
#else
			PostMessage(hWnd, UM_RECALC, 0, 0) ;
#endif
			return(0) ;
			break ;

#if 0
		case	WM_PAINT:
			MessageBeep(0) ;
			PostMessage(hWnd, UM_RECALC, 0, 0) ;
			/* NOTE: fall through to default */
			break ;
#endif

		case	UM_DODIALOG:
			lclInvokeDialog(hWnd) ;
			return(0) ;
			break ;
			
		case	UM_RECALC:
			if (wParam & UMRCWPF_REPERIOD) {
				lclDoReperiod() ;
			}
			if (lclDoRecalc() || (wParam & UMRCWPF_REPAINT)) {
				lclDoRepaint(hWnd) ;
			}
			return(0) ;
			break ;

		case	UM_HELP:
#if	DEBUG && TRUE
			fprintf(fplog, "help: ref=%ld\n", (long) lParam) ;
#endif
			return(lclDoHelp(hWnd, (long) lParam)) ;
			break ;
			
		case	WM_SYSKEYDOWN:
			if (wParam == VK_F1) {
				PostMessage(
					hWnd,
					UM_HELP,
					(WPARAM) 0,
					(LPARAM) lclGetHelp()
				) ;
			}
			break ;


	}

	return(DefWindowProc(hWnd, message, wParam, lParam)) ;
}

LRESULT _export CALLBACK HelpKeyHook(int code, WPARAM wParam, LPARAM lParam)
{
#if	DEBUG && TRUE
	fprintf(
		fplog,
		"hook: code=%d wParam=0x%04X lParam=0x%08X\n",
		code, (short) wParam, (long) lParam
	) ;
#endif
	if ((code == MSGF_DIALOGBOX) || (code == MSGF_MENU)) {
		LPMSG lpmsg= (LPMSG) lParam ;
	
		if (
			(lpmsg->message == WM_KEYDOWN)
//		     || (lpmsg->message == WM_SYSKEYDOWN)
		) {
			if (lpmsg->wParam == VK_F1) {
				if (hWndTop != (HWND) NULL) {
					PostMessage(
						hWndTop,
						UM_HELP,
						(WPARAM) code,
						(LPARAM) lclGetHelp()
					) ;
					return(1) ;
				}
			}
		}
	}
	CallNextHookEx(hhHelpKeyHook, code, wParam, lParam) ;
	return(0) ;
}

/******************************************************************/

static BOOL lclExtractProjectName(char* pszProject, char* pszProjectName)
{
	char szProjectLabel[MAXSIZE_PROJECTLABEL+1] ;

	project_disassemble(pszProject, pszProjectName, szProjectLabel) ;
	return(TRUE) ;
}

static BOOL lclGetProjectByName(LPSTR lpszProjectName, char* pszProject)
{
	int iProject ;

	iProject= lclFindProjectName(lpszProjectName) ;
	if (iProject< 0) {
		pszProject[0]= '\0' ;
		return(FALSE) ;
	}
	return(lclGetProject(iProject, pszProject)) ;
}

/******************************************************************/

#if 0
static BOOL lclDlgComboSetStr(HWND hWndCombo, LPSTR lpszStr)
{
	SetWindowText(hWndCombo, lpszStr) ;
	SendMessage(
		hWndCombo,
		CB_SETEDITSEL,
		0,
		MAKELONG(0, lstrlen(lpszStr))
	) ;
	return(TRUE) ;
}
#endif

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
		MessageBeep(0) ;
		fprintf(fplog, "lclDlgTimeUpdate: unknown control\n") ;
#endif
		return(FALSE) ;		/* unknown control */
	}

	*lptTime= timeutil_tTime(*lptTime, tut_id, nTimeAdj) ;
	lclDlgTimeSet(hDlg, *lptTime) ;
	return(TRUE) ;
}

static BOOL lclDlgGetListSel(HWND hDlgCtrl, LPSTR lpStr)
{
	DWORD dwIndex ;

	dwIndex= SendMessage(hDlgCtrl, CB_GETCURSEL, 0, 0) ;
	if (dwIndex == CB_ERR) {
#if	DEBUG
		MessageBeep(0) ;
		fprintf(fplog, "lclDlgGetListSel: CB_ERR returned!\n") ;
#endif
		return(FALSE) ;
	}
	SendMessage(
		hDlgCtrl,
		CB_GETLBTEXT,
		(WPARAM) dwIndex,
		(LPARAM) lpStr
	) ;
	return(TRUE) ;
}

static BOOL lclDlgSetListSel(HWND hWndCtrl, LPSTR lpStr)
{
	DWORD dwIndex ;

	/*
		Get index of specified entry in list/combo box
	*/
	dwIndex= SendMessage(
		hWndCtrl,
		CB_FINDSTRINGEXACT,
		0,
		(LPARAM) ((LPSTR) lpStr)
	) ;
	if (dwIndex == CB_ERR) return(FALSE) ;

	/*
		We found the specified entry, so set the
		current item in the list/combo box
	*/
	SendMessage(hWndCtrl, CB_SETCURSEL, (WPARAM) dwIndex, 0) ;
	return(TRUE) ;
}

static BOOL lclDlgEditInit(HWND hWndEdit, LPSTR lpStr, int maxlen)
{
	SendMessage(hWndEdit, EM_LIMITTEXT, (WPARAM) maxlen, 0) ;
	SendMessage(hWndEdit, WM_SETTEXT, 0, (LPARAM) lpStr) ;
	return(TRUE) ;
}

/******************************************************************/

static BOOL lclInvokeDialog(HWND hWndParent)
{	
	DLGPROC lpfnDialogProc ;
	DLG dlg ;
	int dlgrc ;
	long lOldHelpRef ;

	dlg.tTime= time(NULL) ;
	dlg.szCmtText[0]= '\0' ;
	dlg.iTimeScope= iTimeScope ;
	dlg.iTimeDisplayFormat= iTimeDisplayFormat ;
	dlg.bAutoDialog= bAutoDialog ;
	lstrcpy(dlg.szOvhProjectName, szOvhProjectName) ;
	lstrcpy(dlg.szSelProjectName, szCurrentProjectName) ;

	lOldHelpRef= lclSetHelp(REF_PROJECT) ;
	lpfnDialogProc= (DLGPROC) MakeProcInstance(
		(FARPROC) DialogProc, hInst
	) ;
 	dlgrc= DialogBoxParam(
		hInst,
		"DLGPROJECT",
		hWndParent,
		lpfnDialogProc,
		(LPARAM) ((LPDLG) &dlg)
	) ;
	FreeProcInstance((FARPROC) lpfnDialogProc) ;
	lclSetHelp(lOldHelpRef) ;
	
	if (dlgrc< 0) {
		lclErrorMsg(hWndParent, "Can't create Dialog", 0) ;
		return(FALSE) ;
	}

	switch(dlgrc) {

		case	IDOK:
		case	IDOVERHEAD:
			if (!lclPerformDialog(hWndParent, &dlg, dlgrc)) {
				return(FALSE) ;
			}
			iTimeScope= dlg.iTimeScope ;
			iTimeDisplayFormat= dlg.iTimeDisplayFormat ;
			bAutoDialog= dlg.bAutoDialog ;
			lstrcpy(szOvhProjectName, dlg.szOvhProjectName) ;
			lstrcpy(szCurrentProjectName, dlg.szSelProjectName) ;
			lclPromoteProject(
				lclFindProjectName(
					szCurrentProjectName
				)
			) ;
			if (dlg.szCmtText[0] != '\0') {
				lclWriteLogEntry(
					time(NULL),
					dlg.tTime,
					0,
					LRA_COMMENT,
					LRC_PROJECT,
					dlg.szCmtText
				) ;
			}
			break ;

		case	IDEXIT:
			PostMessage(hWndTop, WM_CLOSE, 0, 0) ;
			break ;
	}

	return(TRUE) ;
}

static BOOL lclPerformDialog(HWND hWndParent, LPDLG lpdlg, int id)
{
	WPARAM umrcwpf ;
	BOOL bMustReperiod= FALSE ;
	LPSTR lpszNewProjectName ;

	if (id == IDOK) {
		lpszNewProjectName= lpdlg->szSelProjectName ;
	}

#if 1
	if (id == IDOVERHEAD) {			/* this should NEVER happen! */
		if (lpdlg->szOvhProjectName[0] == '\0') {
			lclErrorMsg(hWndParent, "No Overhead Project", 0) ;
			return(FALSE) ;
		}
		lpszNewProjectName= lpdlg->szOvhProjectName ;
	}
#endif

	if (lclStartProjectName(lpszNewProjectName)) {
		bMustReperiod= TRUE ;
	}
	if (lpdlg->iTimeScope != iTimeScope) {
		bMustReperiod= TRUE ;
	}

	umrcwpf= UMRCWPF_REPAINT ;
	if (bMustReperiod) umrcwpf|= UMRCWPF_REPERIOD ;
	PostMessage(hWndTop, UM_RECALC, umrcwpf, 0) ;
	
	return(TRUE) ;
}

static BOOL lclInitDialog(HWND hDlg, LPDLG lpdlg)
{
	char szDialogTitle[MAXSIZE_TITLETEXT+1] ;

	/*
		Set the dialog box's title with active project name
	*/
	wsprintf(
		szDialogTitle,
		"%s - [%s]",
		(LPSTR) pszAppName,
		(LPSTR) szProjectName
	) ;
	SetWindowText(hDlg, szDialogTitle) ;

	/*
		Set the radio-button states
	*/
	lclDlgRBGset(
		hDlg,
		IDC_RADIODAY,
		IDC_RADIOYEAR,
		lpdlg->iTimeScope
	) ;
	lclDlgRBGset(
		hDlg,
		IDC_RADIOHFRACT,
		IDC_RADIOHHMM,
		lpdlg->iTimeDisplayFormat
	) ;

	/*
		Set the check-box states
	*/
	SendDlgItemMessage(
		hDlg,
		IDC_AUTODIALOG,
		BM_SETCHECK,
		lpdlg->bAutoDialog? 1: 0,
		0
	) ;

	/*
		Initialize the "comment" edit-text control.
	*/
	lclInitDialogCmt(GetDlgItem(hDlg, IDC_EDITCOMMENT), lpdlg) ;

	/*
		Set the rest of the dialog "state" values
		from the initial values just set...
	*/
	lclStateDialog(hDlg, lpdlg) ;
	return(TRUE) ;
}

static BOOL lclStateDialog(HWND hDlg, LPDLG lpdlg)
{
	HWND hWndComboPN ;
	HWND hWndComboPNO ;
	BOOL bHaveOverhead ;

	char szProjectTemp[MAXSIZE_PROJECT+1] ;
	char szProjectTempName[MAXSIZE_PROJECTNAME+1] ;
	int i, iProject ;

	/*
		Get window handle of children we will modify
	*/
	hWndComboPN= GetDlgItem(hDlg, IDC_COMBOPROJECT) ;
	hWndComboPNO= GetDlgItem(hDlg, IDC_COMBOPROJECTOVH) ;

	/*
		Re-populate "Project", and "Overhead" combo/list boxes
	*/
	SendMessage(hWndComboPN, CB_RESETCONTENT, 0, 0) ;
	SendMessage(hWndComboPNO, CB_RESETCONTENT, 0, 0) ;
	for (
		i= 0, iProject= lclGetFirstProject(szProjectTemp) ;
		iProject>= 0 ;
		i++, iProject= lclGetNextProject(iProject, szProjectTemp)
	) {
		lclExtractProjectName(szProjectTemp, szProjectTempName) ;
		SendMessage(
			hWndComboPN,
			CB_ADDSTRING,
			0,
			(LONG) ((LPSTR) szProjectTempName)
		) ;
		SendMessage(
			hWndComboPNO,
			CB_ADDSTRING,
			0,
			(LONG) ((LPSTR) szProjectTempName)
		) ;
	}

	/*
		Select the current projects in "Project",
		and "Overhead" list/combo boxes.
	*/
	lclDlgSetListSel(hWndComboPN, lpdlg->szSelProjectName) ;
	lclDlgSetListSel(hWndComboPNO, lpdlg->szOvhProjectName) ;

	/*
		Enable Overhead buttons appropriately
	*/
	bHaveOverhead= (lpdlg->szOvhProjectName[0] != '\0') ;
	EnableWindow(GetDlgItem(hDlg, IDOVERHEAD), bHaveOverhead) ;
	EnableWindow(GetDlgItem(hDlg, IDDELOVERHEAD), bHaveOverhead) ;

	return(TRUE) ;
}

static BOOL lclReadDialog(HWND hDlg, LPDLG lpdlg)
{
	/*
		Get the "bAutoDialog" flag from the dialog box
	*/
	lpdlg->bAutoDialog= (BOOL) SendDlgItemMessage(
		hDlg,
		IDC_AUTODIALOG,
		BM_GETCHECK,
		0,
		0L
	) ;

	/*
		Get the "iTimeDisplayFormat" value from the dialog box
	*/
	lpdlg->iTimeDisplayFormat= lclDlgRBGget(
		hDlg,
		IDC_RADIOHFRACT,
		IDC_RADIOHHMM
	) ;

	/*
		Get the "iTimeScope" value from the dialog box,
	*/
	lpdlg->iTimeScope= lclDlgRBGget(
		hDlg,
		IDC_RADIODAY,
		IDC_RADIOYEAR
	) ;

	/*
		Get the contents of the Comment edit-text box
	*/
	GetWindowText(
		GetDlgItem(hDlg, IDC_EDITCOMMENT),
		lpdlg->szCmtText,
		sizeof(lpdlg->szCmtText)
	) ;

	return(TRUE) ;
}

static BOOL lclAdjustDialog(HWND hDlg, LPDLG lpdlg)
{
	lclInvokeDialogAdj(hDlg, lpdlg) ;
	return(TRUE) ;
}

static BOOL lclDeleteOvhDialog(HWND hDlg, LPDLG lpdlg)
{
	if (lpdlg->szOvhProjectName[0] == '\0') {
		lclErrorMsg(hDlg, "No Overhead Project", 0) ;
		return(FALSE) ;
	}
	lpdlg->szOvhProjectName[0]= '\0' ;
	PostMessage(hDlg, UMD_RESTATE, 0, 0) ;
	return(TRUE) ;
}

static BOOL lclInitDialogCmt(HWND hDlgCmt, LPDLG lpdlg)
{
	lclDlgEditInit(
		GetDlgItem(hDlgCmt, IDC_EDITCOMMENT), 
		lpdlg->szCmtText,
		sizeof(lpdlg->szCmtText)
	) ;
	return(TRUE) ;
}

/******************************************************************/

static BOOL lclInvokeDialogNew(HWND hWndParent, LPDLG lpdlg)
{
	DLGPROC lpfnDialogNewProc ;
	DLGNEW dlgnew ;
	int dlgrc ;
	char szNewProject[MAXSIZE_PROJECT+1] ;
	long lOldHelpRef ;

	dlgnew.szNewProjectName[0]= '\0' ;
	dlgnew.szNewProjectLabel[0]= '\0' ;

	lOldHelpRef= lclSetHelp(REF_NEWPROJECT) ;
	lpfnDialogNewProc= (DLGPROC) MakeProcInstance(
		(FARPROC) DialogNewProc, hInst
	) ;
	dlgrc= DialogBoxParam(
		hInst,
		"DLGNEWPROJECT",
		hWndParent,
		lpfnDialogNewProc,
		(LPARAM) ((LPDLGNEW) &dlgnew)
	) ;
	FreeProcInstance((FARPROC) lpfnDialogNewProc) ;
	lclSetHelp(lOldHelpRef) ;

	if (dlgrc< 0) {
		lclErrorMsg(
			hWndParent,
			"Can't create New Project Dialog",
			dlgrc
		) ;
		return(FALSE) ;
	}

	switch(dlgrc) {
		case	IDOK:
			project_assemble(
				szNewProject, 
				dlgnew.szNewProjectName,
				dlgnew.szNewProjectLabel
			) ;
			lclAddProject(szNewProject) ;
			return(TRUE) ;	/* new project was added */
			break ;
	}

	/* new project was NOT added */
	return(FALSE) ;
}	

static BOOL lclInitDialogNew(HWND hDlgNew, LPDLGNEW lpdlgnew)
{
	lclDlgEditInit(
		GetDlgItem(hDlgNew, IDC_EDITPROJECTNAME),
		lpdlgnew->szNewProjectName,
		MAXSIZE_PROJECTNAME
	) ;
	lclDlgEditInit(
		GetDlgItem(hDlgNew, IDC_EDITPROJECTLABEL),
		lpdlgnew->szNewProjectLabel,
		MAXSIZE_PROJECTLABEL
	) ;
	return(TRUE) ;
}

static BOOL lclReadDialogNew(HWND hDlgNew, LPDLGNEW lpdlgnew)
{
	GetWindowText(
		GetDlgItem(hDlgNew, IDC_EDITPROJECTNAME),
		lpdlgnew->szNewProjectName,
		MAXSIZE_PROJECTNAME
	) ;
	GetWindowText(
		GetDlgItem(hDlgNew, IDC_EDITPROJECTLABEL),
		lpdlgnew->szNewProjectLabel,
		MAXSIZE_PROJECTLABEL
	) ;
	return(TRUE) ;
}

static BOOL lclCheckDialogNew(HWND hDlgNew, LPDLGNEW lpdlgnew)
{
	HWND hWndEdit ;
	LPARAM lcsel ;

	hWndEdit= GetDlgItem(hDlgNew, IDC_EDITPROJECTNAME) ;
	lcsel= (LPARAM) MAKELONG(0, lstrlen(lpdlgnew->szNewProjectName)) ;

	if (lpdlgnew->szNewProjectName[0] == '\0') {
		lclErrorMsg(hDlgNew, "No Project Name specified", 0) ;
		SendMessage(hWndEdit, EM_SETSEL, 0, lcsel) ;
		SetFocus(hWndEdit) ;
		return(FALSE) ;
	}	
	if (lclFindProjectName(lpdlgnew->szNewProjectName)>= 0) {
		lclErrorMsg(hDlgNew, "Specified Project already exists", 0) ;
		SendMessage(hWndEdit, EM_SETSEL, 0, lcsel) ;
		SetFocus(hWndEdit) ;
		return(FALSE) ;
	}
	return(TRUE) ;
}

/******************************************************************/

static BOOL lclInvokeDialogRpt(HWND hWndParent)
{	
	UINT uiReturn ;
	char szRptProgAndParam[MAXSIZE_SYSCMD+1] ;

	wsprintf(
		szRptProgAndParam,
		"%s %u",
		(LPSTR) szRptProg,
		(unsigned) hWndParent
	) ;

	uiReturn= WinExec(szRptProgAndParam, SW_SHOW) ;
	if (uiReturn< 32) {
		lclErrorMsg(
			hWndParent,
			"Can't run Report Program (WinExec)",
			uiReturn
		) ;
		return(FALSE) ;
	}
	return(TRUE) ;
}

/******************************************************************/

static BOOL lclInvokeDialogAdj(HWND hWndParent, LPDLG lpdlg)
{	
	DLGPROC lpfnDialogAdjProc ;
	DLGADJ dlgadj ;
	int dlgrc ;
	long laSecs ;
	char szAdjProject[MAXSIZE_PROJECT+1] ;
	time_t tNow ;
	long lOldHelpRef ;

	dlgadj.tAdjRef= time(NULL) ;
	dlgadj.iaHours= 0 ;
	dlgadj.iaMinutes= 0 ;
	lstrcpy(dlgadj.szAdjProjectName, lpdlg->szSelProjectName) ;
	dlgadj.szCmtText[0]= '\0' ;
 
 	lOldHelpRef= lclSetHelp(REF_ADJUSTMENTS) ;
	lpfnDialogAdjProc= (DLGPROC) MakeProcInstance(
		(FARPROC) DialogAdjProc, hInst
	) ;
	dlgrc= DialogBoxParam(
		hInst,
		"DLGADJUST",
		hWndParent,
		lpfnDialogAdjProc,
		(LPARAM) ((LPDLGADJ) &dlgadj)
	) ;
	FreeProcInstance((FARPROC) lpfnDialogAdjProc) ;
	lclSetHelp(lOldHelpRef) ;

	if (dlgrc< 0) {
		lclErrorMsg(hWndParent, "Can't create Adjust dialog", 0) ;
		return(FALSE) ;
	}

	switch(dlgrc) {

		case	IDOK:
			laSecs= (long) dlgadj.iaHours ;		/* # hrs   */
			laSecs*= 60L ;				/* # hmins */
			laSecs+= (long) dlgadj.iaMinutes ;	/* # mins  */
			laSecs*= 60L ;				/* # msecs */

			lclGetProjectByName(
				dlgadj.szAdjProjectName,
				szAdjProject
			) ;

			tNow= time(NULL) ;

			lclWriteLogEntry(
				tNow,
				dlgadj.tAdjRef,
				laSecs,
				LRA_ADJUSTMENT,
				LRC_PROJECT,
				szAdjProject
			) ;

			if (dlgadj.szCmtText[0] != '\0') {
				lclWriteLogEntry(
					tNow,
					dlgadj.tAdjRef,
					0,
					LRA_COMMENT,
					LRC_PROJECT,
					dlgadj.szCmtText
				) ;
			}

			PostMessage(
				hWndTop,
				UM_RECALC,
				UMRCWPF_REPAINT | UMRCWPF_REPERIOD,
				0
			) ;

			break ;
	}

	return(TRUE) ;
}

static BOOL lclInitDialogAdj(HWND hDlgAdj, LPDLGADJ lpdlgadj)
{
	char szIniProject[MAXSIZE_PROJECT+1] ;
	char szIniProjectName[MAXSIZE_PROJECTNAME+1] ;
	HWND hWndComboPN ;
	int i, iProject ;
	DWORD dwIndex ;

	/*
		Set current time as reference time for adjustment
	*/
	lclDlgTimeSet(hDlgAdj, lpdlgadj->tAdjRef) ;

	/*
		Load list box (combo-box) with list of projects.
	*/
	hWndComboPN= GetDlgItem(hDlgAdj, IDC_COMBOPROJECT) ;
	for (
		i= 0, iProject= lclGetFirstProject(szIniProject) ;
		iProject>= 0 ;
		i++, iProject= lclGetNextProject(iProject, szIniProject)
	) {
		lclExtractProjectName(szIniProject, szIniProjectName) ;
		SendMessage(
			hWndComboPN,
			CB_ADDSTRING,
			0,
			(LONG) ((LPSTR) szIniProjectName)
		) ;
	}
	
	/*
		Get index of selected project in newly populated combo box
	*/
	dwIndex= SendMessage(
		hWndComboPN,
		CB_FINDSTRINGEXACT,
		0,
		(LPARAM) lpdlgadj->szAdjProjectName
	) ;

	/*
		If we found the current project in the list, then
		we set the current item in the list box (combo-box)
	*/
	if (dwIndex != CB_ERR) {
		SendMessage(
			hWndComboPN,
			CB_SETCURSEL,
			(WPARAM) dwIndex,
			0
		) ;
	}

	/*
		Set the initial adjustment value
	*/
	lclDlgEditSetNumber(
		GetDlgItem(hDlgAdj, IDC_EDITHOURADJ),
		lpdlgadj->iaHours
	) ;
	lclDlgEditSetNumber(
		GetDlgItem(hDlgAdj, IDC_EDITMINUTEADJ),
		lpdlgadj->iaMinutes
	) ;

	/*
		Initialize the comment edit text box
	*/
	lclDlgEditInit(
		GetDlgItem(hDlgAdj, IDC_EDITCOMMENT), 
		lpdlgadj->szCmtText,
		sizeof(lpdlgadj->szCmtText)
	) ;

	return(TRUE) ;
}

static BOOL lclReadDialogAdj(HWND hDlgAdj, LPDLGADJ lpdlgadj)
{
	int i ;

	/*
		Get Selected project
	*/
	lclDlgGetListSel(
		GetDlgItem(hDlgAdj, IDC_COMBOPROJECT),
		lpdlgadj->szAdjProjectName
	) ;

	/*
		Get Adjustment value
	*/
	lclDlgEditGetNumber(GetDlgItem(hDlgAdj, IDC_EDITHOURADJ), &i) ;
	lpdlgadj->iaHours= i ;
	lclDlgEditGetNumber(GetDlgItem(hDlgAdj, IDC_EDITMINUTEADJ), &i) ;
	lpdlgadj->iaMinutes= i ;

	/*
		Get the contents of the Comment edit-text box
	*/
	GetWindowText(
		GetDlgItem(hDlgAdj, IDC_EDITCOMMENT),
		lpdlgadj->szCmtText,
		sizeof(lpdlgadj->szCmtText)
	) ;

	return(TRUE) ;
}

static BOOL lclScrollDialogAdj(
	HWND hDlgAdj,
	HWND hDlgCtrl,
	WPARAM wScrollCode,
	LPDLGADJ lpdlgadj
) {
	HWND hWndEdit ;
	int nTimeAdj ;

	hWndEdit= (HWND) NULL ;
	if (hDlgCtrl == GetDlgItem(hDlgAdj, IDC_SCROLLHOURADJ)) {
		hWndEdit= GetDlgItem(hDlgAdj, IDC_EDITHOURADJ) ;
	}
	else if (hDlgCtrl == GetDlgItem(hDlgAdj, IDC_SCROLLMINUTEADJ)) {
		hWndEdit= GetDlgItem(hDlgAdj, IDC_EDITMINUTEADJ) ;
	}

	/*
		If the scroll is for the reference time, handle it
	*/
	if (hWndEdit != (HWND) NULL) {
		lclDlgEditGetNumber(hWndEdit, &nTimeAdj) ;
		switch(wScrollCode) {
			case	SB_LINEDOWN:	nTimeAdj++ ;	break ;
			case	SB_LINEUP:	nTimeAdj-- ;	break ;
			default:	return(FALSE) ;		break ;
		}
		lclDlgEditSetNumber(hWndEdit, nTimeAdj) ;
		return(TRUE) ;
	}

	/*
		else the scroll is for the adjustment value: handle it
	*/
	switch(wScrollCode) {
		case	SB_LINEDOWN:	nTimeAdj= 1 ;		break ;
		case	SB_LINEUP:	nTimeAdj= -1 ;		break ;
		default:		return(FALSE) ;		break ;
	}
	lclDlgTimeUpdate(hDlgAdj, hDlgCtrl, nTimeAdj, &lpdlgadj->tAdjRef) ;

	return(TRUE) ;
}

/******************************************************************/

static BOOL lclYesNo(HWND hWndParent, char *pszYesNoMsg)
{
	BOOL bYesNo ;

	EnableWindow(hWndParent, FALSE) ;
	bYesNo= (
		MessageBox(
			hWndParent,
			pszYesNoMsg,
			pszAppName,
			MB_YESNO | MB_ICONQUESTION
		) == IDYES
	)? TRUE: FALSE ;
	EnableWindow(hWndParent, TRUE) ;
	return(bYesNo) ;
}

/******************************************************************/

static void lclDlgRBGset(HWND hDlg, int id0, int id1, int idoff)
{
	HWND hWndRB ;
	int i, iset ;

	iset= (id0 + idoff) ;
	for (i= id0; i<= id1; i++) {
		hWndRB= GetDlgItem(hDlg, i) ;
		SendMessage(hWndRB, BM_SETCHECK, (i==iset)? 1: 0, 0L) ;
	}
}

static int lclDlgRBGget(HWND hDlg, int id0, int id1)
{
	HWND hWndRB ;
	int i ;

	for (i= id0; i<= id1; i++) {
		hWndRB= GetDlgItem(hDlg, i) ;
		if (SendMessage(hWndRB, BM_GETCHECK, 0, 0L) != 0) {
			return(i - id0) ;
		}
	}
	return(-1) ;
}

/******************************************************************/

static BOOL lclGetMessageProc(WPARAM *pexitcode)
{
	MSG msg ;
	static bExited= FALSE ;
	static WPARAM exitcode ;
	
	if (!bExited) {
		if (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg) ;
			DispatchMessage(&msg) ;
			return(TRUE) ;
		}
		exitcode= msg.wParam ;
		bExited= TRUE ;
	}
	if (pexitcode != (WPARAM *) NULL) {
		(*pexitcode)= msg.wParam ;
	}
	return(FALSE) ;			
}

static BOOL lclInitApplication(HINSTANCE hInstancePrev)
{
	WNDCLASS wc ;
	
	if (hInstancePrev != (HINSTANCE) NULL) {
#if 0
		HWND hWndTopPrev ;

		hWndTopPrev= FindWindow(pszAppClass, NULL) ;
		if (hWndTopPrev != (HWND) NULL) {
			SetActiveWindow(hWndTopPrev) ;
			return(FALSE) ;
		}
#endif
		lclErrorMsg(NULL, "TimeLog already active", 0) ;
		return(FALSE) ;
	}

#if 0	
	wc.style	= (CS_HREDRAW | CS_VREDRAW) ;
#else
	wc.style	= 0 ;
#endif
	wc.lpfnWndProc	= (WNDPROC) WindowProc ;
	wc.cbClsExtra	= 0 ;
	wc.cbWndExtra	= 0 ;
	wc.hInstance	= hInst ;
	wc.hIcon	= LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONLOG)) ;
	wc.hCursor	= LoadCursor(NULL, IDC_ARROW) ;
	wc.hbrBackground= (HBRUSH) (COLOR_BACKGROUND + 1) ;
	wc.lpszMenuName	= NULL ;
	wc.lpszClassName= pszAppClass ;
	
	if (!RegisterClass(&wc)) {
		lclErrorMsg(NULL, "Can't register TimeLog class", 0) ;
		return(FALSE) ;
	}
	
	return(TRUE) ;
}

static BOOL lclInitInstance(LPSTR lpszCmdLine, int nCmdShow)
{
	HWND hWnd ;
	struct wincrparms wcrp ;
	
	wcrp.nCmdShow= nCmdShow ;
	wcrp.lpszCmdLine= lpszCmdLine ;

	if (!lclReadProfile()) {
		lclErrorMsg(NULL, "Can't read TimeLog profile", 0) ;
		return(FALSE) ;
	}

#if 0
	lclSetFilePaths() ;
#endif

	logfile_Init(szLogFile) ;

	hWnd= CreateWindow(
		pszAppClass,
		pszAppName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInst,
		&wcrp
	) ;
	if (hWnd == NULL) {
		lclErrorMsg(NULL, "Can't create TimeLog window", 0) ;
		return(FALSE) ;
	}

	lpfnHelpKeyHook= (HOOKPROC) MakeProcInstance((FARPROC) HelpKeyHook, hInst) ;
	if (lpfnHelpKeyHook != (HOOKPROC) NULL) {
		hhHelpKeyHook= SetWindowsHookEx(
			WH_MSGFILTER,
			lpfnHelpKeyHook,
			hInst,
			GetCurrentTask()
		) ;
	}

	ShowWindow(hWnd, SW_SHOWMINNOACTIVE) ;
	UpdateWindow(hWnd) ;

	if (bAutoDialog) {
		SendMessage(hWnd, UM_DODIALOG, 0, 0) ;
	}

	return(TRUE) ;
}

static BOOL lclFiniInstance()
{
	if (lpfnHelpKeyHook != (HOOKPROC) NULL) {
		UnhookWindowsHookEx(hhHelpKeyHook) ;
		FreeProcInstance((FARPROC) lpfnHelpKeyHook) ;
	}
	logfile_Fini() ;
	if (!lclWriteProfile()) {
		lclErrorMsg(NULL, "Can't write TimeLog profile", 0) ;
		return(FALSE) ;
	}
	return(TRUE) ;
}

#if 0
static BOOL lclMsg(HWND hWndParent, char *pszMsg)
{
	EnableWindow(hWndParent, FALSE) ;
	MessageBox(
		hWndParent,
		pszMsg,
		pszAppName,
		MB_OK | MB_ICONINFORMATION
	) ;
	EnableWindow(hWndParent, TRUE) ;
	return(FALSE) ;
}
#endif

#if 0
/*
	begin of file name normalization
*/

static void lclParseFilePath(char *pszName, char *pszPath)
{
	int i, j ;

/*
	E:helpme.exe		==>		"E:"
	E:doc\helpme.exe	==>		"E:doc\"
	junk.exe		==>		""
*/

	for (i= lstrlen(pszName); i> 0; i--) {
		if (pszName[i-1] == '\\') break ;
		if (pszName[i-1] == ':') break ;
	}
	for (j= 0; j< i; j++) {
		pszPath[j]= pszName[j] ;
	}
	pszPath[j]= '\0' ;
}

static BOOL lclAbsoluteFileName(char *pszFileName)
{
	int l ;

	l= lstrlen(pszFileName) ;
	if (l< 1) return(TRUE) ;				/* ""?	*/
	if (l> 1) {
		if (pszFileName[1] == ':') return(TRUE) ;	/* C:xx	*/
	}
	return((BOOL) (pszFileName[0] == '\\')) ;
}

static void lclResolveFileName(char *pszFile, char *pszPath)
{
	char szFileWithPath[MAXSIZE_FILENAME+1] ;

	if ((lstrlen(pszFile) + lstrlen(pszPath))<= MAXSIZE_FILENAME) {
		lstrcpy(szFileWithPath, pszPath) ;
		lstrcat(szFileWithPath, pszFile) ;
		lstrcpy(pszFile, szFileWithPath) ;
	}
}

static void lclSetFilePaths()
{
	char szModuleName[MAXSIZE_FILENAME+1] ;
	char szModulePath[MAXSIZE_FILENAME+1] ;

	GetModuleFileName(hInst, szModuleName, MAXSIZE_FILENAME+1) ;
	lclParseFilePath(szModuleName, szModulePath) ;
	if (!lclAbsoluteFileName(szLogFile)) {
		lclResolveFileName(szLogFile, szModulePath) ;
	}
	if (!lclAbsoluteFileName(szRptProg)) {
		lclResolveFileName(szRptProg, szModulePath) ;
	}
	if (!lclAbsoluteFileName(szHelpFile)) {
		lclResolveFileName(szHelpFile, szModulePath) ;
	}
}

/*
	end of file name normalization
*/
#endif

static BOOL lclDoHelp(HWND hWnd, long lRef)
{
	BOOL bOk ;

#if	DEBUG && TRUE
	fprintf(fplog, "dohelp: ref=%lu\n", lRef) ;
#endif
	bOk= WinHelp(hWnd, szHelpFile, HELP_CONTEXT, (DWORD) lRef) ;
#if	DEBUG && TRUE
	fprintf(fplog, "  => bOk=%d\n", (int) bOk) ;
#endif
	return(bOk) ;
}

static long lHelpRef_lcl= 0 ;
static long lclSetHelp(long lRef)
{
	long lRefOld ;
	
	lRefOld= lHelpRef_lcl ;
	lHelpRef_lcl= lRef ;
	return(lRefOld) ;
}

static long lclGetHelp()
{
	return(lHelpRef_lcl) ;
}

static BOOL lclErrorMsg(HWND hWndParent, char *pszErrorMsg, int rc)
{
	char emsg[256] ;

	lstrcpy(emsg, pszErrorMsg) ;
	if (rc != 0) {
		wsprintf(
			(LPSTR)(emsg + lstrlen(emsg)),
			"\n(error code == %d)",
			rc
		) ;
	}
	EnableWindow(hWndParent, FALSE) ;
	MessageBox(
		hWndParent,
		emsg,
		pszAppName,
		MB_OK | MB_ICONEXCLAMATION
	) ;
	EnableWindow(hWndParent, TRUE) ;
	return(FALSE) ;
}

/******************************************************************/

static BOOL lclStartProjectName(LPSTR lpszProjectName)
{
	char szProject[MAXSIZE_PROJECT+1] ;

	/* if there's an active project, */
	if (tProjectTimeStart != (time_t) NULL) {
		/* and if this project is it, */
		if (lstrcmp(lpszProjectName, szProjectName) == 0) {
			/* then don't do anything */
			return(FALSE) ;
		}
	}

	lclGetProjectByName(lpszProjectName, szProject) ;
	if (tProjectTimeStart != (time_t) NULL) lclStopProject() ;
		/* copy to the global variable the now active project name */
	lstrcpy(szProjectName, lpszProjectName) ;
	tProjectTimeStart= time(NULL) ;
	lclWriteLogEntry(
		tProjectTimeStart,
		tProjectTimeStart,
		0,
		LRA_START,
		LRC_PROJECT,
		szProject
	) ;
	return(TRUE) ;
}

static void lclStopProject()
{
	char szProject[MAXSIZE_PROJECT+1] ;

	if (tProjectTimeStart == (time_t) NULL) return ;
		/* get the active project by the global active project name */
	lclGetProjectByName(szProjectName, szProject) ;
	tProjectTimeStop= time(NULL) ;
	lclWriteLogEntry(
		tProjectTimeStop,
		tProjectTimeStop,
		0,
		LRA_STOP,
		LRC_PROJECT,
		szProject		/* stop the active project */
	) ;
	tProjectTimeStart= (time_t) NULL ;
}

static BOOL lclStartSession()
{
	tSessionTimeStart= time(NULL) ;
	lclWriteLogEntry(
		tSessionTimeStart,
		tSessionTimeStart,
		0,
		LRA_START,
		LRC_SESSION,
		NULL
	) ;
	return(
		lclStartProjectName(
			(szOvhProjectName[0] != '\0')	?
			szOvhProjectName		:
			szCurrentProjectName
		)
	) ;
}

static void lclStopSession()
{
	if (tSessionTimeStart == (time_t) NULL) return ;
	lclStopProject() ;
	tSessionTimeStop= time(NULL) ;
	lclWriteLogEntry(
		tSessionTimeStop,
		tSessionTimeStop,
		0,
		LRA_STOP,
		LRC_SESSION,
		NULL
	) ;
	tSessionTimeStart= (time_t) NULL ;
}

/******************************************************************/

static BOOL lclEventCreateWindow(HWND hWnd)
{
	hWndTop= hWnd ;

	lclSetHelp(REF_CONTENTS) ;
	lclStartSession() ;
	SetTimer(hWnd, 1, uiUpdateTimes, NULL) ;

	/*	post a message to recalculate * redraw everything 	*/
	PostMessage(
		hWndTop,
		UM_RECALC,
		UMRCWPF_REPAINT | UMRCWPF_REPERIOD,
		0
	) ;
	return(TRUE) ;
}

static BOOL lclEventDestroyWindow(HWND hWnd)
{
	KillTimer(hWnd, 1) ;
	lclStopSession() ;
	hWndTop= NULL ;
	return(TRUE) ;
}

static BOOL lclEventEndSession()
{
	lclStopSession() ;
	lclWriteProfile() ;
	return(TRUE) ;
}

/******************************************************************/

static void lclDoRepaint(HWND hWnd)
{
	char szTitleText[MAXSIZE_TITLETEXT+1] ;
	char szSessionTime[MAXSIZE_TIMESCOPE+1] ;
	char szProjectTime[MAXSIZE_TIMESCOPE+1] ;
	char szTS[MAXSIZE_TIMESCOPE+1] ; 
	char *pszFmt ;
	float fSessionHours, fProjectHours ;
	int tul_id ;

	if (hWnd == (HWND) NULL) return ;

	switch(iTimeScope) {
		case	TS_YEAR:	tul_id= TUL_YEAR ;		break ;
		case	TS_MONTH:	tul_id= TUL_MONTH ;		break ;
		default:		tul_id= TUL_WDAY ;		break ;
	}

	timeutil_szLoad(tProjectTimeStart, tul_id, szTS) ;

	if (iTimeDisplayFormat == TDF_HFRACT) {
		fSessionHours= (float) iSessionMinutes ;
		fSessionHours/= (float) 60 ;
		fSessionHours+= (float) iSessionHours ;
		fProjectHours= (float) iProjectMinutes ;
		fProjectHours/= (float) 60 ;
		fProjectHours+= (float) iProjectHours ;
		pszFmt= "%4.2f" ;
		sprintf(szSessionTime, pszFmt, fSessionHours) ;
		sprintf(szProjectTime, pszFmt, fProjectHours) ;
	}
	else {
		pszFmt= "%d:%02d" ;
		sprintf(
			szSessionTime,
			pszFmt,
			iSessionHours,
			iSessionMinutes
		) ;
		sprintf(
			szProjectTime,
			pszFmt,
			iProjectHours,
			iProjectMinutes
		) ;
	}

	if (szProjectName[0] != '\0') {
		pszFmt= "%s - Total:%s, %s:%s" ;
		sprintf(
			szTitleText,
			pszFmt,
			szTS,
			szSessionTime,
			szProjectName,
			szProjectTime
		) ;
	}
	else {
		pszFmt= "%s - Total:%s" ;
		sprintf(
			szTitleText,
			pszFmt,
			szTS,
			szSessionTime
		) ;
	}

	SetWindowText(hWnd, szTitleText) ;
}

static BOOL lclDoRecalc()
{
	time_t tNow ;
	LONG lSessionTimeCount ;
	LONG lProjectTimeCount ;
	int iNewSessionHours, iNewSessionMinutes ;
	int iNewProjectHours, iNewProjectMinutes ;

	time(&tNow) ;

	lProjectTimeCount= (LONG) (tNow - tProjectTimeStart) ;
#if 0
	lSessionTimeCount= (LONG) (tNow - tSessionTimeStart) ;
#else
	lSessionTimeCount= lProjectTimeCount ;
#endif

	lProjectTimeCount+= lProjectPreviousSecs ;
	lSessionTimeCount+= lSessionPreviousSecs ;

#if	DEBUG && FALSE
	fprintf(
		fplog,
		"ps=%ld pp=%ld\n",
		lSessionPreviousSecs,
		lProjectPreviousSecs
	) ;
#endif

	iNewSessionHours= (int) (lSessionTimeCount / (60L * 60L)) ;
	iNewSessionMinutes= (int) (
		(lSessionTimeCount % (60L * 60L))
	      / (60L)
	) ;

	iNewProjectHours= (int) (lProjectTimeCount / (60L * 60L)) ;
	iNewProjectMinutes= (int) (
		(lProjectTimeCount % (60L * 60L))
	      / (60L)
	) ;
	
	if (
		(iNewSessionHours	!= iSessionHours)
	     || (iNewSessionMinutes	!= iSessionMinutes)
	     ||	(iNewProjectHours	!= iProjectHours)
	     || (iNewProjectMinutes	!= iProjectMinutes)
	) {
		iSessionHours		= iNewSessionHours ;
		iSessionMinutes		= iNewSessionMinutes ;
		iProjectHours		= iNewProjectHours ;
		iProjectMinutes		= iNewProjectMinutes ;
		return(TRUE) ;
	}

	return(FALSE) ;
}

static BOOL lclDoReperiod()
{
	int tut_id ;
	char szProject[MAXSIZE_PROJECT+1] ;

	switch(iTimeScope) {
		case	TS_MONTH:	tut_id= TUT_STARTOFMONTH ;	break ;
		case	TS_YEAR:	tut_id= TUT_STARTOFYEAR ;	break ;
		default:		tut_id= TUT_STARTOFDAY ;	break ;
	}

	tReportPeriod_From= timeutil_tTime(tProjectTimeStart, tut_id, 0) ;
	tReportPeriod_To= timeutil_tTime(tProjectTimeStart, TUT_FOREVER, 0) ;

	/*
		this way scans all projects: it is better than
		scanning all sesions in the case of a crash --
		all "closed" projects within the current session
		are reported.  Also, adjustments are reported,
		because adjustments are made to projects, and
		not to sessions.
	*/
	lSessionPreviousSecs= scanlog_lTotalTime(
		tReportPeriod_From,
		tReportPeriod_To,
		LRC_PROJECT,
		NULL
	) ;

			/* get active project by global active project name */
	lclGetProjectByName(szProjectName, szProject) ;
	lProjectPreviousSecs= scanlog_lTotalTime(
		tReportPeriod_From,
		tReportPeriod_To,
		LRC_PROJECT,
		szProject
	) ;

#if	DEBUG && FALSE
	fprintf(fplog, "found %ld secs for project %s\n",
		lProjectPreviousSecs, szProject) ;
#endif

	return(TRUE) ;
}

/******************************************************************/

#define	MAXSIZE_PROJECTVAR	32

static BOOL lclGetProfileString(
	char *pszVar,
	char *pszStr,
	char *pszStrDef,
	int maxsize
) {
	GetPrivateProfileString(
		pszProfileSection,
		pszVar,
		pszStrDef,
		pszStr,
		maxsize,
		pszProfileFile
	) ;
	return(TRUE) ;
}

static BOOL lclGetProfileProject(char *pszVar, char *pszProject)
{
	char szIniVar[MAXSIZE_PROJECTVAR+1] ;
	char szIniProjectName[MAXSIZE_PROJECTNAME+1] ;
	char szIniProjectLabel[MAXSIZE_PROJECTLABEL+1] ;

	wsprintf(szIniVar, "%s Name", (LPSTR) pszVar) ;
	lclGetProfileString(
		szIniVar, szIniProjectName, "", MAXSIZE_PROJECTNAME+1
	) ;
	wsprintf(szIniVar, "%s Label", (LPSTR) pszVar) ;
	lclGetProfileString(
		szIniVar, szIniProjectLabel, "", MAXSIZE_PROJECTLABEL+1
	) ;
	project_assemble(pszProject, szIniProjectName, szIniProjectLabel) ;
	return(TRUE) ;
}

static BOOL lclGetProfileInt(char *pszVar, int *piValue, int iValueDef)
{
	(*piValue)= GetPrivateProfileInt(
		pszProfileSection,
		pszVar,
		iValueDef,
		pszProfileFile
	) ;
	return(TRUE) ;
}

static BOOL lclGetProfileBool(char *pszVar, BOOL *pbValue, BOOL bValueDef)
{
	BOOL bOk ;
	int iValue ;

	bOk= lclGetProfileInt(pszVar, &iValue, (int) bValueDef) ;
	(*pbValue)= (BOOL) iValue ;
	return(bOk) ;
}

static BOOL lclReadProfile()
{
	int i ;
	char szProjectVar[MAXSIZE_PROJECTVAR+1] ;
	char szIniProject[MAXSIZE_PROJECT+1] ;
	
						/* get help file name */
	lclGetProfileString(
		"Help File",
		szHelpFile,
		pszHelpFileDefault,
		MAXSIZE_HELPFILENAME
	) ;

						/* get log file name */
	lclGetProfileString(
		"Log File",
		szLogFile,
		pszLogFileDefault,
		MAXSIZE_LOGFILENAME
	) ;

						/* get rpt prog name */
	lclGetProfileString(
		"Report Program",
		szRptProg,
		pszRptProgDefault,
		MAXSIZE_RPTPROGNAME
	) ;

						/* get project list */
	for (i= MAX_INIPROJECTS - 1; i>= 0; i--) {
		wsprintf(szProjectVar, "Project %d", i) ;
		lclGetProfileProject(szProjectVar, szIniProject) ;
		lclAddProject(szIniProject) ;
	}
					/* get current project name */
	lclGetProfileString(
		"Current Project Name",
		szCurrentProjectName,
		"",
		MAXSIZE_PROJECTNAME+1
	) ;

					/* get overhead project name */
	lclGetProfileString(
		"Overhead Project Name",
		szOvhProjectName,
		"",
		MAXSIZE_PROJECTNAME+1
	) ;

						/* get options */
	lclGetProfileBool("Auto Dialog", &bAutoDialog, FALSE) ;
	lclGetProfileInt(
		"Time Display Format", &iTimeDisplayFormat, TDF_HFRACT
	) ;
	lclGetProfileInt("Time Scope", &iTimeScope, TS_DAY) ;

	return(TRUE) ;
}

/******************************************************************/

static BOOL lclWriteProfileString(char *pszVar, char *pszValue)
{
	LPSTR lpszValue ;

	/*
		I convert "pszValue" into (LPSTR) because the compiler
		doesn't make the proper conversion when it's NULL.
	*/
	lpszValue= (pszValue == (char *) NULL)	?
		(LPSTR) NULL 			:
		(LPSTR) pszValue		;

	if (lpszValue != (LPSTR) NULL) {
		if (lpszValue[0] == '\0') lpszValue= (LPSTR) NULL ;
	}

	WritePrivateProfileString(
		pszProfileSection,
		pszVar,
		lpszValue,
		pszProfileFile
	) ;

	return(TRUE) ;
}

static BOOL lclWriteProfileInt(char *pszVar, int iValue)
{
	char profileintstrbuf[16] ;

	wsprintf(profileintstrbuf, "%d", iValue) ;
	return(lclWriteProfileString(pszVar, profileintstrbuf)) ;
}

static BOOL lclWriteProfileBool(char *pszVar, BOOL bValue)
{
	return(lclWriteProfileInt(pszVar, (int) bValue)) ;
}

static BOOL lclWriteProfileProject(char *pszVar, char *pszProject)
{
	char szIniVar[MAXSIZE_PROJECTVAR+1] ;
	char szIniProjectName[MAXSIZE_PROJECTNAME+1] ;
	char szIniProjectLabel[MAXSIZE_PROJECTLABEL+1] ;

	project_disassemble(pszProject, szIniProjectName, szIniProjectLabel) ;
	wsprintf(szIniVar, "%s Name", (LPSTR) pszVar) ;
	lclWriteProfileString(szIniVar, szIniProjectName) ;
	wsprintf(szIniVar, "%s Label", (LPSTR) pszVar) ;
	return(lclWriteProfileString(szIniVar, szIniProjectLabel)) ;
}

static BOOL lclWriteProfile()
{
	int i, iProject ;
	char szIniProject[MAXSIZE_PROJECT+1] ;
	char szProjectVar[MAXSIZE_PROJECTVAR+1] ;

	/*
		write out the current project list
	*/
	for (
		i= 0, iProject= lclGetFirstProject(szIniProject) ;
		iProject>= 0 ;
		i++, iProject= lclGetNextProject(iProject, szIniProject)
	) {
		wsprintf(szProjectVar, "Project %d", i) ;
		lclWriteProfileProject(szProjectVar, szIniProject) ;
	}

	/*
		delete any old projects above current list
	*/
	for (; i< MAX_INIPROJECTS; i++) {
		wsprintf(szProjectVar, "Project %d", i) ;
		lclWriteProfileProject(szProjectVar, NULL) ;
	}

						/* save current project */
	lclWriteProfileString(
		"Current Project Name", szCurrentProjectName
	) ;

						/* Save overhead project */
	lclWriteProfileString(
		"Overhead Project Name", szOvhProjectName
	) ;

						/* save general options */
	lclWriteProfileBool("Auto Dialog", bAutoDialog) ;
	lclWriteProfileInt("Time Display Format", iTimeDisplayFormat) ;
	lclWriteProfileInt("Time Scope", iTimeScope) ;

	return(TRUE) ;
}

/******************************************************************/

static BOOL lclWriteLogEntry(
	time_t tTime,
	time_t tTime_a,
	long lval,
	int iaction,
	int icategory,
	char *pszText
) {
	BOOL bOk ;

	if (logfile_OpenForWrite(NULL)) {
		lclErrorMsg(
			hWndTop,
			"Error opening log file",
			logfile_errno()
		) ;
		return(FALSE) ;
	}

	bOk= TRUE ;
	if (logfile_WriteEntry(
		(long) tTime,
		(long) tTime_a,
		lval,
		iaction,
		icategory,
		pszText
	)) {
		lclErrorMsg(
			hWndTop,
			"Error writing to log file",
			logfile_errno()
		) ;
		bOk= FALSE ;
	}

	logfile_Close() ;
	return(bOk) ;
}

/******************************************************************/

static BOOL lclDeleteProject(int iProject)
{
	free(apszIniProjects[iProject]) ;
	apszIniProjects[iProject]= (char *) NULL ;
	return(TRUE) ;
}

static BOOL lclPromoteProject(int iProject)
{
	int i ;
	char *pszChosenProject ;

	/*
		1.	very parameter
	*/
	if ((iProject< 0) || (iProject>= MAX_INIPROJECTS)) {
		return(FALSE) ;
	}

	/*
		2.	save project name
	*/
	pszChosenProject= apszIniProjects[iProject] ;

	/*
		3.	move projects above project name "down one"
	*/
	for (i= iProject; i> 0; i--) {
		apszIniProjects[i]= apszIniProjects[i-1] ;
	}

	/*
		4.	put project name in first place
	*/
	apszIniProjects[0]= pszChosenProject ;

	return(TRUE) ;
}

static int lclFindProjectName(LPSTR lpszProjectName)
{
	int i ;
	char szProjectTempName[MAXSIZE_PROJECTNAME+1] ;

	for (i= 0; i< MAX_INIPROJECTS; i++) {
		if (apszIniProjects[i] == (char *) NULL) continue ;
		lclExtractProjectName(
			apszIniProjects[i],
			szProjectTempName
		) ;
		if (lstrcmp(szProjectTempName, lpszProjectName) == 0) {
			return(i) ;
		}
	}
	return(-1) ;
}

static int lclFindProject(LPSTR lpszProject)
{
	int i ;

	for (i= 0; i< MAX_INIPROJECTS; i++) {
		if (apszIniProjects[i] == (char *) NULL) continue ;
		if (lstrcmp(apszIniProjects[i], lpszProject) == 0) {
			return(i) ;
		}
	}
	return(-1) ;
}

static BOOL lclIsOkToDeleteProject(char *pszProject)
{
	char szProjectName[MAXSIZE_PROJECTNAME+1] ;
	char question_msg[MAXSIZE_PROJECTNAME+100] ;

	lclExtractProjectName(pszProject, szProjectName) ;
	wsprintf(
		question_msg,
		"Project list is full.\n"
		"Ok to delete least recently used project,\n"
		"\"%s\"?",
		(LPSTR) szProjectName
	) ;
	return(lclYesNo(hWndTop, question_msg)) ;
}

static BOOL lclAddProject(LPSTR lpszProject)
{
	int i ;

	/*
		1.	check parameters.
	*/
	if (lpszProject == (LPSTR) NULL) return(FALSE) ;
	if (lpszProject[0] == '\0') return(FALSE) ;

	/*
		2.	if the project already exists,
			simply move it to the top.
	*/
	if ((i= lclFindProject(lpszProject))>= 0) {
		return(lclPromoteProject(i)) ;
	}

	/*
		3.	find the first unused slot
			slot in the project list.
	*/
	for (i= 0; i< MAX_INIPROJECTS; i++) {
		if (apszIniProjects[i] == (char *) NULL) break ;
	}

	/*
		4.	if list is full (no unused slot found),
			then delete last entry in list.
			17.8.94:	ask if ok, and if not, fail.
	*/
	if (i == MAX_INIPROJECTS) {
		i= MAX_INIPROJECTS - 1 ;	/* use last slot in proj list */
		if (!lclIsOkToDeleteProject(apszIniProjects[i])) {
			return(FALSE) ;
		}
		lclDeleteProject(i) ;
	}

	/*
		5.	allocate the project, attached to slot 'i',
			and promote it to the top of the list.
	*/
	apszIniProjects[i]= (char *) malloc(lstrlen(lpszProject)+1) ;
	if (apszIniProjects[i] == (char *) NULL) return(FALSE) ;
	lstrcpy(apszIniProjects[i], lpszProject) ;
	return(lclPromoteProject(i)) ;
}

static int lclGetFirstProject(LPSTR lpszProject)
{
	return(lclGetNextProject(0, lpszProject)) ;
}

static int lclGetNextProject(int iProject0, LPSTR lpszProject)
{
	int i ;

	for (i= iProject0; i< MAX_INIPROJECTS; i++) {
		if (lclGetProject(i, lpszProject)) {
			return(i+1) ;
		}
	}
	return(-1) ;
}

static BOOL lclGetProject(int iProject, LPSTR lpszProject)
{
	if (apszIniProjects[iProject] == (char *) NULL) return(FALSE) ;
	lstrcpy(lpszProject, apszIniProjects[iProject]) ;
	return(TRUE) ;
}

/******************************************************************/

/* end of timelog.c */
