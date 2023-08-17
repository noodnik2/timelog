#define	Revision	55

/*

	timerpt.c			MS-Windows TimeLog Report Program
					Copyright (C) 1994, by Marty Ross


*/

/******************************************************************/

#include	"wininc.h"

#include	<stdio.h>		/* for sscanf */
#include	<memory.h>

#include	"resource.h"
#include	"timerpt.h"
#include	"tlrptdlg.h"

/******************************************************************/

HINSTANCE hInst= NULL ;

#if	DEBUG
FILE *fpdbg ;
#endif

/******************************************************************/

static char *pszAppName=	"TimeRpt"	;
static char *pszAppClass=	"ClsTimeRpt"	;

static char *pszProfileSection=	"TimeRpt"	;
static char *pszHelpFileDefault="timerpt.hlp"	;
static char *pszProfileFile=	"timelog.ini"	;
static char *pszLogFileDefault=	"timelog.tlg"	;

static HWND hWndCaller= NULL ;

/******************************************************************/

static BOOL lclParseCmdline(LPSTR) ;
static BOOL lclInitApplication(HINSTANCE, HINSTANCE) ;
static BOOL lclInitInstance(HINSTANCE, int) ;
static BOOL lclFiniInstance(void) ;

static BOOL lclInvokeDialogRpt(HWND) ;

static BOOL lclReadDialogRptProfile(LPDLGRPT lpdlgrpt) ;
static BOOL lclWriteDialogRptProfile(LPDLGRPT lpdlgrpt) ;

/******************************************************************/

int PASCAL WinMain(
	HINSTANCE hInstance,
	HINSTANCE hInstancePrev,
	LPSTR lpszCmdLine,
	int nCmdShow
) {
	int rc ;
	
#if	DEBUG
	fpdbg= fopen("timerpt.log", "w") ;
	if (fpdbg == (FILE *) NULL) {
		timerpt_ErrorMsg((HWND) NULL, "Can't open log file", 0) ;
		return(0) ;
	}
	fprintf(fpdbg, "** start of timerpt.log: Revision=%d\n", Revision) ;
#endif
	
	rc= -1 ;
	if (lclParseCmdline(lpszCmdLine)) {
		if (lclInitApplication(hInstance, hInstancePrev)) {
			if (lclInitInstance(hInstance, nCmdShow)) {
				lclInvokeDialogRpt(hWndCaller) ;
				lclFiniInstance() ;
				rc= 0 ;
			}
		}
	}

#if	DEBUG
	fprintf(fpdbg, "** end of timerpt.log: rc=%d\n", rc) ;
#endif
	
	return(0) ;
}

/*
	function gorup: stuff that should be placed in a library
*/

BOOL timerpt_ErrorMsg(HWND hWndParent, LPSTR lpszErrorMsg, int rc)
{
	char emsg[256] ;

	lstrcpy(emsg, lpszErrorMsg) ;
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

static BOOL lclParseCmdline(LPSTR lpszCmdLine)
{
	unsigned ui ;

	hWndCaller= (HWND) NULL ;

	if (lpszCmdLine[0] != 0) {
		if (sscanf(lpszCmdLine, "%u", &ui) != 1) {
			timerpt_ErrorMsg((HWND) NULL, "invalid parameter", 0) ;
			return(FALSE) ;
		}
		hWndCaller= (HWND) ui ;
	}

	return(TRUE) ;
}

static BOOL lclInitApplication(HINSTANCE hInstance, HINSTANCE hInstancePrev)
{
	if (hInstancePrev != (HINSTANCE) NULL) {
		timerpt_ErrorMsg(hWndCaller, "Application already active", 0) ;
		return(FALSE) ;
	}
	return(TRUE) ;
}

static BOOL lclInitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst= hInstance ;
	return(TRUE) ;
}

static BOOL lclFiniInstance()
{
	return(TRUE) ;
}

/******************************************************************/

static BOOL lclInvokeDialogRpt(HWND hWndParent)
{	
	DLGRPT dlgrpt ;

	memset((void *) &dlgrpt, '\0', sizeof(DLGRPT)) ;
	dlgrpt.tRptRef= time(NULL) ;
	lclReadDialogRptProfile(&dlgrpt) ;
	if (!tlrptdlg_InvokeDialog(hWndParent, &dlgrpt)) return(FALSE) ;
	if (dlgrpt.bUpdate) lclWriteDialogRptProfile(&dlgrpt) ;
	return(TRUE) ;
}

/*
	function section:	reading profile
*/

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

static BOOL lclReadDialogRptProfile(LPDLGRPT lpdlgrpt)
{
						/* no entry for this */
	lpdlgrpt->tRptRef= time(NULL) ;

						/* get help file name */
	lclGetProfileString(
		"Help File",
		lpdlgrpt->szHelpFile,
		pszHelpFileDefault,
		MAXSIZE_HELPFILENAME
	) ;

						/* get log file name */
	lclGetProfileString(
		"Log File",
		lpdlgrpt->szLogFile,
		pszLogFileDefault,
		MAXSIZE_LOGFILENAME
	) ;

						/* get log user name */
	lclGetProfileString(
		"User Name",
		lpdlgrpt->szUserName,
		"",
		MAXSIZE_USERNAME
	) ;

						/* get options */
	lclGetProfileInt("Time Unit", &lpdlgrpt->iTimeScope, TS_DAY) ;

	lclGetProfileInt(
		"Time Format",
		&lpdlgrpt->tlrptdsp.iTimeDisplayFormat,
		TDF_HHMM
	) ;
	lclGetProfileInt("Entry Format", &lpdlgrpt->iEntryType, LRDET_ALL) ;
	lclGetProfileInt("Number of Periods", &lpdlgrpt->nPeriods, 1) ;
	lclGetProfileBool("Totals", &lpdlgrpt->bTotals, TRUE) ;
	lclGetProfileBool("Allocations", &lpdlgrpt->bAlloc, FALSE) ;
	lclGetProfileBool("Adjustments", &lpdlgrpt->bAdjust, FALSE) ;
	lclGetProfileBool("Comments", &lpdlgrpt->bComments, FALSE) ;
	lclGetProfileBool("Sparse", &lpdlgrpt->tlrptdsp.bSparse, FALSE) ;

	return(TRUE) ;
}

/******************************************************************/

/*
	function section:	writing profile
*/

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

static BOOL lclWriteDialogRptProfile(LPDLGRPT lpdlgrpt)
{
						/* save log user name */
	lclWriteProfileString("User Name", lpdlgrpt->szUserName) ;

						/* save report options */
	lclWriteProfileInt("Time Unit", lpdlgrpt->iTimeScope) ;
	lclWriteProfileInt("Time Format", lpdlgrpt->tlrptdsp.iTimeDisplayFormat) ;
	lclWriteProfileInt("Entry Format", lpdlgrpt->iEntryType) ;
	lclWriteProfileInt("Number of Periods", lpdlgrpt->nPeriods) ;
	lclWriteProfileBool("Totals", lpdlgrpt->bTotals) ;
	lclWriteProfileBool("Comments", lpdlgrpt->bComments) ;
	lclWriteProfileBool("Allocations", lpdlgrpt->bAlloc) ;
	lclWriteProfileBool("Adjustments", lpdlgrpt->bAdjust) ;
	lclWriteProfileBool("Sparse", lpdlgrpt->tlrptdsp.bSparse) ;
	return(TRUE) ;
}

/******************************************************************/

/* end of timerpt.c */
