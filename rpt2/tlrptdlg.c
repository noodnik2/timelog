#define	Revision	96

/*

	tlrptdlg.c

*/

/******************************************************************/

#include	"wininc.h"

#include	<memory.h>

#include	"resource.h"
#include	"tlrptdlg.h"
#include	"tlrptdsp.h"
#include	"tlrptvar.h"
#include	"tlrptts.h"
#include	"tlrptabt.h"

/******************************************************************/
				
#define		UM_FINIDIALOG	(UM_TLRPTDSP_USER+1)
#define		UM_REDRAWRPT	(UM_TLRPTDSP_USER+2)
#define		UM_HELP		(UM_TLRPTDSP_USER+3)

/******************************************************************/

BOOL __export CALLBACK tlrptdlg_DialogProc(HWND, UINT, WPARAM, LPARAM) ;

/******************************************************************/

static BOOL lclInitDialogRpt(HWND, LPDLGRPT) ;
static BOOL lclReadDialogRpt(HWND, LPDLGRPT) ;
static BOOL lclCommandDialogRpt(HWND, LPDLGRPT, WPARAM, LPARAM) ;
static BOOL lclOpenDialogRpt(HWND hWndRpt, LPDLGRPT lpdlgrpt) ;
static BOOL lclCloseDialogRpt(HWND hWndRpt, LPDLGRPT lpdlgrpt) ;

static BOOL lclGetPrinterCompatibleLogFont(HWND hWnd, LPDLGRPT lpdlgrpt) ;

static BOOL lclInvokeDialogVAR(HWND hWndParent, LPDLGRPT lpdlgrpt) ;
static BOOL lclInvokeDialogTS(HWND hWndParent, LPDLGRPT lpdlgrpt) ;
static BOOL lclInvokeDialogPS(HWND hWndParent, LPDLGRPT lpdlgrpt) ;
static BOOL lclInvokeDialogSF(HWND hWndParent, LPDLGRPT lpdlgrpt) ;
static BOOL lclInvokeDialogAbout(HWND hWndParent) ;

static BOOL lclDlgMenuFindItem(HMENU, UINT, HMENU *, UINT *) ;
static BOOL lclDlgMenuSetCheck(HMENU hMenu, UINT id, BOOL bCheck) ;
static BOOL lclDlgMenuToggleCheck(HMENU hMenu, UINT id) ;
static BOOL lclDlgMenuGetCheck(HMENU hMenu, UINT id) ;
static BOOL lclDlgMenuSetCheckByPos(HMENU hMenu, UINT pos, BOOL bCheck) ;
static BOOL lclDlgMenuGetCheckByPos(HMENU hMenu, UINT pos) ;
static BOOL lclDlgMenuSetListSel(HMENU hMenu, UINT id0, UINT id1, UINT isel) ;
static int lclDlgMenuGetListSel(HMENU hMenu, UINT id0, UINT id1) ;

/******************************************************************/

/*
	function: invoke the rpt dialog
*/

BOOL tlrptdlg_InvokeDialog(HWND hWndParent, LPDLGRPT lpdlgrpt)
{	
	DLGPROC lpfntlrptdlg_DialogProc ;
	DWORD PDFlags ;
	int dlgrc ;
	TRHELPREF hrOld ;

	/*
		initialize the printer dialog info structure
	*/

	memset(&lpdlgrpt->pd, 0, sizeof(PRINTDLG)) ;

	PDFlags=  PD_USEDEVMODECOPIES
		| PD_ALLPAGES
		| PD_NOPAGENUMS
		| PD_NOSELECTION
	;

	lpdlgrpt->pd.lStructSize= sizeof(PRINTDLG) ;
	lpdlgrpt->pd.hwndOwner= hWndParent ;
	lpdlgrpt->pd.Flags= PDFlags | PD_RETURNDEFAULT ;
	if (!PrintDlg(&lpdlgrpt->pd)) {
		timerpt_ErrorMsg(hWndParent, "Can't get printer setup", 0) ;
		return(FALSE) ;
	}
	lpdlgrpt->pd.Flags= PDFlags ;

	/*
		initialize other, non-dialog info variables
	*/
	lpdlgrpt->bUpdate= FALSE ;

	/*
		probably we should check the return from these Init calls
	*/
	logfile_Init(lpdlgrpt->szLogFile) ;
	logrpt_Init() ;
	tlrptdsp_Init() ;
	tlrpthlp_Init() ;

	lpfntlrptdlg_DialogProc= (DLGPROC) MakeProcInstance(
		(FARPROC) tlrptdlg_DialogProc,
		hInst
	) ;

	hrOld= tlrpthlp_SetHelpRef(REF_REPORT) ;
	dlgrc= DialogBoxParam(
		hInst,
		"DLGRPT",
		hWndParent,
		lpfntlrptdlg_DialogProc,
		(LPARAM) lpdlgrpt
	) ;
	tlrpthlp_SetHelpRef(hrOld) ;

	FreeProcInstance((FARPROC) lpfntlrptdlg_DialogProc) ;

	tlrpthlp_Fini() ;
	tlrptdsp_Fini() ;
	logrpt_Fini() ;
	logfile_Fini() ;

	/*
		Free the data associated with lpdlgrpt->pd
	*/
	if (lpdlgrpt->pd.hDevMode != NULL) {
		GlobalFree(lpdlgrpt->pd.hDevMode) ;
	}
	if (lpdlgrpt->pd.hDevNames != NULL) {
		GlobalFree(lpdlgrpt->pd.hDevNames) ;
	}

	/*
		return based upon result from dialog
	*/
	if (dlgrc< 0) {
		timerpt_ErrorMsg(hWndParent, "Report Dialog Error", dlgrc) ;
		return(FALSE) ;
	}

	if (dlgrc == IDOK) lpdlgrpt->bUpdate= TRUE ;

	return(TRUE) ;
}

/******************************************************************/

BOOL __export CALLBACK tlrptdlg_DialogProc(
	HWND hDlgRpt,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	static LPDLGRPT lpdlgrpt= (LPDLGRPT) NULL ;

#if	DEBUG
//	fprintf(fpdbg, "tlrpdlg: message=0x%04X\n", message) ;
#endif

	switch(message) {

		case	WM_INITDIALOG:
			lpdlgrpt= (LPDLGRPT) lParam ;
			if (!lclInitDialogRpt(hDlgRpt, lpdlgrpt)) {
				EndDialog(hDlgRpt, IDCANCEL) ;
			}
			tlrpthlp_SetHelpMsg(hDlgRpt, UM_HELP) ;
			PostMessage(hDlgRpt, UM_REDRAWRPT, 0, 0) ;
			return(FALSE) ;
			break ;
		
		case	UM_FINIDIALOG:
			tlrpthlp_SetHelpMsg((HWND) NULL, 0) ;
			lclCloseDialogRpt(hDlgRpt, lpdlgrpt) ;
			lpdlgrpt= (LPDLGRPT) NULL ;
			EndDialog(hDlgRpt, (int) wParam) ;
			break ;

		case	UM_HELP:
			WinHelp(
				hDlgRpt,
				lpdlgrpt->szHelpFile,
				HELP_CONTEXT,
				(DWORD) lParam
			) ;
			return(FALSE) ;
			break ;

		case	UM_REDRAWRPT:
#if	DEBUG
			fprintf(fpdbg, "tlrpdlg: redrawing\n") ;
#endif
			lclCloseDialogRpt(hDlgRpt, lpdlgrpt) ;
			lclReadDialogRpt(hDlgRpt, lpdlgrpt) ;
			lclOpenDialogRpt(hDlgRpt, lpdlgrpt) ;
			InvalidateRect(hDlgRpt, NULL, TRUE) ;
			break ;

		case	WM_COMMAND:
			return(
				lclCommandDialogRpt(
					hDlgRpt,
					lpdlgrpt,
					wParam,
					lParam
				)
			) ;
			break ;

	}

	if (lpdlgrpt != (LPDLGRPT) NULL) {
		if (lpdlgrpt->hRpt != (HRPT) NULL) {
			return(
				tlrptdsp_Link(
					lpdlgrpt->hRpt,
					hDlgRpt,
					message,
					wParam,
					lParam
				)
			) ;
		}
	}

	return(FALSE) ;
}	

/******************************************************************/

/*
	function group:	put and get data from the rpt dialog
*/

static BOOL lclInitDialogRpt(HWND hDlgRpt, LPDLGRPT lpdlgrpt)
{
	HMENU hDlgMenu ;

	/*
		Initialize printer-compatible logical font structure
	*/
	if (!lclGetPrinterCompatibleLogFont(hDlgRpt, lpdlgrpt)) {
		timerpt_ErrorMsg(
			hDlgRpt,
			"Can't get printer-compatible logical font",
			0
		) ;
		return(FALSE) ;
	}

	/*
		initialize the dialog menu
	*/

	hDlgMenu= GetMenu(hDlgRpt) ;

	lclDlgMenuSetListSel(
		hDlgMenu,
		ID_OPTIONS_TIMEPERIOD_DAYS,
		ID_OPTIONS_TIMEPERIOD_YEARS,
		lpdlgrpt->iTimeScope
	) ;
	lclDlgMenuSetListSel(
		hDlgMenu,
		ID_OPTIONS_TIMEDISPLAY_FRACTION,
		ID_OPTIONS_TIMEDISPLAY_HHMM,
		lpdlgrpt->tlrptdsp.iTimeDisplayFormat
	) ;
	lclDlgMenuSetListSel(
		hDlgMenu,
		ID_OPTIONS_ENTRYTYPE_NAME,
		ID_OPTIONS_ENTRYTYPE_NAMEANDLABEL,
		lpdlgrpt->iEntryType
	) ;

	lclDlgMenuSetCheck(hDlgMenu,ID_OPTIONS_ALLOCATIONS, lpdlgrpt->bAlloc) ;
	lclDlgMenuSetCheck(hDlgMenu,ID_OPTIONS_ADJUSTMENTS, lpdlgrpt->bAdjust) ;
	lclDlgMenuSetCheck(hDlgMenu,ID_OPTIONS_TOTALS, lpdlgrpt->bTotals) ;
	lclDlgMenuSetCheck(hDlgMenu,ID_OPTIONS_COMMENTS, lpdlgrpt->bComments) ;
	lclDlgMenuSetCheck(hDlgMenu,ID_OPTIONS_SPARSE,lpdlgrpt->tlrptdsp.bSparse) ;

	return(TRUE) ;
}

static BOOL lclReadDialogRpt(HWND hDlgRpt, LPDLGRPT lpdlgrpt)
{
	HMENU hDlgMenu ;

	hDlgMenu= GetMenu(hDlgRpt) ;

	/*
		we don't need to read "tRptRef", or "nPeriods", as
		they are kept up-to-date by the change mechanism.
	*/

	lpdlgrpt->iTimeScope= lclDlgMenuGetListSel(
		hDlgMenu,
		ID_OPTIONS_TIMEPERIOD_DAYS,
		ID_OPTIONS_TIMEPERIOD_YEARS
	) ;
	lpdlgrpt->tlrptdsp.iTimeDisplayFormat= lclDlgMenuGetListSel(
		hDlgMenu,
		ID_OPTIONS_TIMEDISPLAY_FRACTION,
		ID_OPTIONS_TIMEDISPLAY_HHMM
	) ;
	lpdlgrpt->iEntryType= lclDlgMenuGetListSel(
		hDlgMenu,
		ID_OPTIONS_ENTRYTYPE_NAME,
		ID_OPTIONS_ENTRYTYPE_NAMEANDLABEL
	) ;

	lpdlgrpt->bAlloc= lclDlgMenuGetCheck(hDlgMenu, ID_OPTIONS_ALLOCATIONS) ;
	lpdlgrpt->bAdjust= lclDlgMenuGetCheck(hDlgMenu,ID_OPTIONS_ADJUSTMENTS) ;
	lpdlgrpt->bTotals= lclDlgMenuGetCheck(hDlgMenu, ID_OPTIONS_TOTALS) ;
	lpdlgrpt->bComments= lclDlgMenuGetCheck(hDlgMenu, ID_OPTIONS_COMMENTS) ;
	lpdlgrpt->tlrptdsp.bSparse= lclDlgMenuGetCheck(hDlgMenu, ID_OPTIONS_SPARSE) ;

	return(TRUE) ;
}

/*
	function group: handle rpt dialog commands
*/

static BOOL lclCommandDialogRpt(
	HWND hDlgRpt,
	LPDLGRPT lpdlgrpt,
	WPARAM wParam,
	LPARAM lParam
) {

#if	DEBUG
//	fprintf(fpdbg, "tlrptdlg: command=0x%04X\n", wParam) ;
#endif

	switch(wParam) {

		case	ID_FILE_EXIT:
			PostMessage(hDlgRpt, WM_COMMAND, IDOK, 0) ;
			return(TRUE) ;
			break ;

		case	ID_HELP_CONTENTS:
			PostMessage(
				hDlgRpt,
				UM_HELP,
				(WPARAM) -1,
				(LPARAM) REF_CONTENTS
			) ;
			break ;

		case	ID_HELP_ABOUT:
			lclInvokeDialogAbout(hDlgRpt) ;
			return(TRUE) ;

#if	DEBUG
		case	WM_DEVMODECHANGE:
//			timerpt_ErrorMsg(hDlgRpt, "dev mode change signal!", 0) ;
			fprintf(fpdbg, "tlrptdlg: devmodechange!\n") ;
			return(TRUE) ;
			break ;
#endif

		case	ID_FILE_PRINT:
			if (lpdlgrpt->plr == (LOGRPT *) NULL) {
#if	DEBUG
				MessageBeep(0) ;
				fprintf(
					fpdbg,
					"tlrptdlg: FILE_PRINT: no report!\n"
				) ;
#endif
				return(TRUE) ;
			}
			if (!tlrptdsp_Print(
				hDlgRpt,
				lpdlgrpt->plr,
				&lpdlgrpt->tlrptdsp,
				&lpdlgrpt->pd
			)) {
				int errno ;
			
				errno= tlrptdsp_errno() ;
				if (errno != RPE_CANCELED) {
					timerpt_ErrorMsg(
						hDlgRpt, 
						"Error while Printing",
						errno
					) ;
				}
			}
			return(TRUE) ;
			break ;

		case	ID_FILE_PRINTSETUP:
			lclInvokeDialogPS(hDlgRpt, lpdlgrpt) ;
			PostMessage(hDlgRpt, UM_REDRAWRPT, 0, 0) ;
			return(TRUE) ;
			break ;

		case	ID_OPTIONS_FONTS:
			lclInvokeDialogSF(hDlgRpt, lpdlgrpt) ;
			PostMessage(hDlgRpt, UM_REDRAWRPT, 0, 0) ;
			return(TRUE) ;
			break ;

		case	ID_OPTIONS_TOTALS:
		case	ID_OPTIONS_SPARSE:
		case	ID_OPTIONS_COMMENTS:
		case	ID_OPTIONS_ALLOCATIONS:
		case	ID_OPTIONS_ADJUSTMENTS:
			lclDlgMenuToggleCheck(
				GetMenu(hDlgRpt),
				(UINT) wParam
			) ;
			PostMessage(hDlgRpt, UM_REDRAWRPT, 0, 0) ;
			return(TRUE) ;
			break ;

		case	ID_OPTIONS_TIMESPAN:
			lclInvokeDialogTS(hDlgRpt, lpdlgrpt) ;
			PostMessage(hDlgRpt, UM_REDRAWRPT, 0, 0) ;
			return(TRUE) ;
			break ;

		case	ID_OPTIONS_VARS:
			lclInvokeDialogVAR(hDlgRpt, lpdlgrpt) ;
			PostMessage(hDlgRpt, UM_REDRAWRPT, 0, 0) ;
			return(TRUE) ;
			break ;

		case	ID_OPTIONS_TIMEPERIOD_DAYS:
		case	ID_OPTIONS_TIMEPERIOD_MONTHS:
		case	ID_OPTIONS_TIMEPERIOD_YEARS:
			lclDlgMenuSetListSel(
				GetMenu(hDlgRpt),
				ID_OPTIONS_TIMEPERIOD_DAYS,
				ID_OPTIONS_TIMEPERIOD_YEARS,
				(UINT) (wParam - ID_OPTIONS_TIMEPERIOD_DAYS)
			) ;
			PostMessage(hDlgRpt, UM_REDRAWRPT, 0, 0) ;
			return(TRUE) ;
			break ;

		case	ID_OPTIONS_TIMEDISPLAY_FRACTION:
		case	ID_OPTIONS_TIMEDISPLAY_HHMM:
			lclDlgMenuSetListSel(
				GetMenu(hDlgRpt),
				ID_OPTIONS_TIMEDISPLAY_FRACTION,
				ID_OPTIONS_TIMEDISPLAY_HHMM,
				(UINT) (wParam-ID_OPTIONS_TIMEDISPLAY_FRACTION)
			) ;
			PostMessage(hDlgRpt, UM_REDRAWRPT, 0, 0) ;
			return(TRUE) ;
			break ;

		case	ID_OPTIONS_ENTRYTYPE_NAME:
		case	ID_OPTIONS_ENTRYTYPE_LABEL:
		case	ID_OPTIONS_ENTRYTYPE_NAMEANDLABEL:
			lclDlgMenuSetListSel(
				GetMenu(hDlgRpt),
				ID_OPTIONS_ENTRYTYPE_NAME,
				ID_OPTIONS_ENTRYTYPE_NAMEANDLABEL,
				(UINT) (wParam - ID_OPTIONS_ENTRYTYPE_NAME)
			) ;
			PostMessage(hDlgRpt, UM_REDRAWRPT, 0, 0) ;
			return(TRUE) ;
			break ;

		case	IDOK:
		case	IDCANCEL:
			lclReadDialogRpt(hDlgRpt, lpdlgrpt) ;
			SendMessage(hDlgRpt, UM_FINIDIALOG, IDOK, 0) ;
			return(TRUE) ;
			break ;
				
	}

	return(FALSE) ;
}
		
/*
	function group:	dialog menu control manipulation
*/

static BOOL lclDlgMenuGetCheckByPos(HMENU hMenu, UINT ipos)
{
	UINT fFlags ;

	fFlags= GetMenuState(hMenu, ipos, MF_BYPOSITION) ;
	if (fFlags & MF_CHECKED) return(TRUE) ;
	return(FALSE) ;
}

static BOOL lclDlgMenuSetCheckByPos(HMENU hMenu, UINT ipos, BOOL bCheck)
{
	CheckMenuItem(
		hMenu, ipos,
		MF_BYPOSITION | (bCheck? MF_CHECKED: MF_UNCHECKED)
	) ;
	return(TRUE) ;
}

/*
	BOOL lclDlgMenuFindItem

	This function was written to find a menu item by its COMMAND
	id value, no matter where it is located in the menu tree.

	In my opinion, it is stupid that the Windows API doesn't give
	me a function for this.  If I want to Get/Set a check on a
	menu item in a "sub-popup menu", I must do it according to
	its position, which I want to be free to change without
	recompiling!

	NOTE:	this function is recursive!

*/
static BOOL lclDlgMenuFindItem(
	HMENU hMenuTop,
	UINT id,
	HMENU *phMenu,
	UINT *pitempos
) {
	HMENU hMenuSub ;
	UINT id0 ;
	int i, nMenuItems ;
	BOOL bOk ;

	bOk= FALSE ;
	nMenuItems= GetMenuItemCount(hMenuTop) ;
	for (i= 0; i< nMenuItems; i++) {
		hMenuSub= GetSubMenu(hMenuTop, i) ;
		if (hMenuSub != (HMENU) NULL) {		/* its a submenu */
			if (lclDlgMenuFindItem(hMenuSub, id, phMenu, pitempos)) {
				bOk= TRUE ;
				break ;
			}
			continue ;
		}
		id0= GetMenuItemID(hMenuTop, i) ;
		if (id0 == id) {
			(*phMenu)= hMenuTop ;
			(*pitempos)= i ;
			bOk= TRUE ;
			break ;
		}
	}

	return(bOk) ;
}

static BOOL lclDlgMenuGetCheck(HMENU hMenu, UINT id)
{
	HMENU hMenuItem ;
	UINT ipos ;

	if (!lclDlgMenuFindItem(hMenu, id, &hMenuItem, &ipos)) return(FALSE) ;
	return(lclDlgMenuGetCheckByPos(hMenuItem, ipos)) ;
}

static BOOL lclDlgMenuSetCheck(HMENU hMenu, UINT id, BOOL bCheck)
{
	HMENU hMenuItem ;
	UINT ipos ;

	if (!lclDlgMenuFindItem(hMenu, id, &hMenuItem, &ipos)) return(FALSE) ;
	return(lclDlgMenuSetCheckByPos(hMenuItem, ipos, bCheck)) ;
}

static BOOL lclDlgMenuToggleCheck(HMENU hMenu, UINT id)
{
	HMENU hMenuItem ;
	UINT ipos ;
	BOOL bCheck ;

	if (!lclDlgMenuFindItem(hMenu, id, &hMenuItem, &ipos)) return(FALSE) ;
	bCheck= lclDlgMenuGetCheckByPos(hMenuItem, ipos) ;
	return(lclDlgMenuSetCheckByPos(hMenuItem, ipos, !bCheck)) ;
}

/*
	note:	a "menu selection list" has:

	(1)	ID numbers from "id0" to "id1", inclusive
		(and appears in this order on the menu)
	(2)	is located in the same pop-up menu, in
		contiguous "MF_POSITION" order.
*/
static BOOL lclDlgMenuSetListSel(HMENU hMenu, UINT id0, UINT id1, UINT isel)
{
	HMENU hMenuList ;
	UINT ipos, ipos0, ipos1, ipos2set ;

	if (!lclDlgMenuFindItem(hMenu, id0, &hMenuList, &ipos0)) return(FALSE) ;
	ipos1= ipos0 + (id1 - id0) ;

	ipos2set= ipos0 + isel ;
	for (ipos= ipos0; ipos<= ipos1; ipos++) {
		lclDlgMenuSetCheckByPos(
			hMenuList, ipos, (BOOL) (ipos == ipos2set)
		) ;
	}
	return(TRUE) ;
}

static int lclDlgMenuGetListSel(HMENU hMenu, UINT id0, UINT id1)
{
	HMENU hMenuList ;
	UINT ipos, ipos0, ipos1 ;

	if (!lclDlgMenuFindItem(hMenu, id0, &hMenuList, &ipos0)) return(FALSE) ;
	ipos1= ipos0 + (id1 - id0) ;
	
	for (ipos= ipos0; ipos<= ipos1; ipos++) {
		if (lclDlgMenuGetCheckByPos(hMenuList, ipos)) {
			return(ipos - ipos0) ;
		}
	}
#if	DEBUG
	fprintf(fpdbg, "menu selection list(%u-%u) is empty!\n", id0, id1) ;
#endif
	return(-1) ;
}

/******************************************************************/

/*
	function group:	open and close reports for report window
*/

static BOOL lclOpenDialogRpt(HWND hWndRpt, LPDLGRPT lpdlgrpt)
{
	time_t tReportPeriod_Start ;
	time_t tReportPeriod_Ref ;
	LOGRPTDEF lrd ;
 	int nPrevious ;
	LOGRPT *plr ;
	int lrtype ;
	int tut_id ;
	HRPT hRpt ;
	
	/*
		fill in the report-generation def structure
	*/

	tReportPeriod_Ref= lpdlgrpt->tRptRef ;
	nPrevious= lpdlgrpt->nPeriods - 1 ;

	switch(lpdlgrpt->iTimeScope) {
		case	TS_YEAR:			/* yearly: years */
			lrtype=		LRT_YEARS ;
			tut_id=		TUT_STARTOFYEAR ;
			break ;
		case	TS_MONTH:			/* monthly: months */
			lrtype=		LRT_YEARMONTHS ;
			tut_id=		TUT_STARTOFMONTH ;
			break ;
		default:				/* daily: days */
			lrtype=		LRT_MONTHDAYS ;
			tut_id=		TUT_STARTOFDAY ;
			break ;
	}

	/*
		Calculate Start of period as "the beginning of
		'nPrevious' periods" before the "reference time".
	*/
	tReportPeriod_Start= timeutil_tTime(
		tReportPeriod_Ref,
		tut_id,
		0 - nPrevious
	) ;

	if (lstrlen(lpdlgrpt->szUserName)> LRMAXSIZE_USERNAME) {
		memcpy(lrd.szUser, lpdlgrpt->szUserName, LRMAXSIZE_USERNAME) ;
		lrd.szUser[LRMAXSIZE_USERNAME]= '\0' ;
	}
	else {
		lstrcpy(lrd.szUser, lpdlgrpt->szUserName) ;
	}
	lrd.t0= 		tReportPeriod_Start ; 
	lrd.t1=        		tReportPeriod_Ref ;
	lrd.lrtype=    		lrtype ;
	lrd.maxentries=		64 ;
	lrd.maxcomments=	512 ;
	lrd.iEntryType=		lpdlgrpt->iEntryType ;
	lrd.lrOptions=		LROPT_DEFAULT ;
	if (lpdlgrpt->bTotals)	lrd.lrOptions|= LROPT_TOTAL ;
	if (lpdlgrpt->bAlloc)	lrd.lrOptions|= LROPT_ALLOC ;
	if (lpdlgrpt->bAdjust)	lrd.lrOptions|= LROPT_ADJUST ;
	if (lpdlgrpt->bComments) lrd.lrOptions|= LROPT_COMMENTS ;

	/*
		Generate a new report
	*/
	plr= logrpt_Create(&lrd) ;
	if (plr == (LOGRPT *) NULL) {
		timerpt_ErrorMsg(
			hWndRpt,
			"Can't create report",
			logrpt_errno()
		) ;
		return(FALSE) ;
	}

	/*
		initialize the report display subystem
	*/
	hRpt= tlrptdsp_CreateLink(
		hWndRpt,
		plr,
		&lpdlgrpt->tlrptdsp,
		&lpdlgrpt->pd
	) ;
	if (hRpt == (HRPT) NULL) {
		timerpt_ErrorMsg(
			hWndRpt,
			"Can't display report",
			tlrptdsp_errno()
		) ;
		logrpt_Delete(plr) ;
		return(FALSE) ;
	}

	lpdlgrpt->plr= plr ;
	lpdlgrpt->hRpt= hRpt ;

	return(TRUE) ;
}

static BOOL lclCloseDialogRpt(HWND hWndRpt, LPDLGRPT lpdlgrpt)
{
	BOOL bOk ;

	bOk= FALSE ;
	if (lpdlgrpt->hRpt != (HRPT) NULL) {
		bOk= tlrptdsp_DestroyLink(lpdlgrpt->hRpt, hWndRpt) ;
		lpdlgrpt->hRpt= (HRPT) NULL ;
	}
	if (lpdlgrpt->plr != (LOGRPT *) NULL) {
		logrpt_Delete(lpdlgrpt->plr) ;
		lpdlgrpt->plr= (LOGRPT *) NULL ;
	}
	return(bOk) ;
}

/******************************************************************/

/*
	function group:	invoke the various sub-dialogs
*/

static BOOL lclInvokeDialogPS(HWND hWndParent, LPDLGRPT lpdlgrpt)
{
	DWORD PDFlags ;
	BOOL bOk ;
	TRHELPREF hrOld ;

	PDFlags= lpdlgrpt->pd.Flags ;
	lpdlgrpt->pd.Flags|= PD_PRINTSETUP ;
	lpdlgrpt->pd.hwndOwner= hWndParent ;
	
	hrOld= tlrpthlp_SetHelpRef(TRHELPREF_NONE) ;
	bOk= PrintDlg(&lpdlgrpt->pd) ;
	tlrpthlp_SetHelpRef(hrOld) ;
	if (!bOk) {			/* Failure or CANCELed */
		if (CommDlgExtendedError() == 0) {
			/*
				Here, we might want to revert back to the
				state of "lpdlgrpt->pd" that we had before
				the PrintDlg() function was called.  I've
				noticed that if you enter "Setup" from within
				the "Print" dialog, change the printer, then
				exit the "Print" dialog with "Cancel", the
				newly selected printer remains selected.
				The question is: are there data structures
				that we need to free/manage before we can
				revert back to the previous state of "pd"?
			*/
			bOk= TRUE ;	/* Dialog box CANCELed */
		}
	}
	lpdlgrpt->pd.Flags= PDFlags ;
	return(bOk) ;
}

static BOOL lclInvokeDialogVAR(HWND hWndParent, LPDLGRPT lpdlgrpt)
{
	DLGVAR dlgvar ;

	memset((void *)&dlgvar, '\0', sizeof(DLGVAR)) ;
	lstrcpy(dlgvar.szUserName, lpdlgrpt->szUserName) ;
	if (!tlrptvar_InvokeDialog(hWndParent, &dlgvar)) return(FALSE) ;
	if (dlgvar.bUpdate) {
		lstrcpy(lpdlgrpt->szUserName, dlgvar.szUserName) ;
	}
	return(TRUE) ;
}

static BOOL lclInvokeDialogTS(HWND hWndParent, LPDLGRPT lpdlgrpt)
{
	DLGTS dlgts ;
	memset((void *)&dlgts, '\0', sizeof(DLGTS)) ;
	dlgts.tStop=		lpdlgrpt->tRptRef ;
	dlgts.nPeriods= 	lpdlgrpt->nPeriods ;
	if (!tlrptts_InvokeDialog(hWndParent, &dlgts)) return(FALSE) ;
	if (dlgts.bUpdate) {
		lpdlgrpt->tRptRef=	dlgts.tStop ;
		lpdlgrpt->nPeriods= 	dlgts.nPeriods ;
	}
	return(TRUE) ;
}

static BOOL lclInvokeDialogAbout(HWND hWndParent)
{
	if (!tlrptabt_InvokeDialog(hWndParent)) return(FALSE) ;
	return(TRUE) ;
}

/******************************************************************/

static BOOL lclGetCompatibleLogicalFont(
	HDC hICNew,
	HDC hICBase,
	LOGFONT *plfNew,
	LOGFONT *plfBase
) {
	long l ;

	memset(plfNew, '\0', sizeof(LOGFONT)) ;
	lstrcpy(plfNew->lfFaceName,	plfBase->lfFaceName) ;
	plfNew->lfWeight=		plfBase->lfWeight ;
	plfNew->lfCharSet=		plfBase->lfCharSet ;
	plfNew->lfPitchAndFamily=	plfBase->lfPitchAndFamily ;

	l= (long) plfBase->lfHeight ;
	l*= (long) GetDeviceCaps(hICNew, LOGPIXELSY) ;
	l/= (long) GetDeviceCaps(hICBase, LOGPIXELSY) ;
	plfNew->lfHeight= (int) l ;
#if 0
	plfNew->lfHeight*= 72 ;
	plfNew->lfHeight/= GetDeviceCaps(hICBase, LOGPIXELSY) ;
		/* now "plfNew->lfHeight" is in normalized font "Points" */
	plfNew->lfHeight*= GetDeviceCaps(hICNew, LOGPIXELSY) ;
	plfNew->lfHeight/= 72 ;
#endif
	return(TRUE) ;
}

static BOOL lclGetPrinterCompatibleLogFont(HWND hWnd, LPDLGRPT lpdlgrpt)
{
	LOGFONT lfp, lfd ;
	HDC hICPrinter, hICDisplay ;
	TEXTMETRIC tm ;

	/*
		get logical font compatible with printer into "lfp"
	*/
	memset(&lfp, '\0', sizeof(LOGFONT)) ;
	hICPrinter= tlrptdsp_CreateDC_FromPPD(&lpdlgrpt->pd, TRUE) ;
	if (hICPrinter == (HDC) NULL) return(FALSE) ;

	GetTextMetrics(hICPrinter, &tm) ;

	GetTextFace(hICPrinter, LF_FACESIZE, lfp.lfFaceName) ;

	lfp.lfHeight=		-(tm.tmHeight-tm.tmInternalLeading) ;
	lfp.lfWeight=		tm.tmWeight ;
	lfp.lfCharSet=		tm.tmCharSet ;
	lfp.lfPitchAndFamily=	tm.tmPitchAndFamily ;

	/*
		get logical font for display that is
		"identical" to printer logical font
	*/
	hICDisplay= GetDC(hWnd) ;
	lclGetCompatibleLogicalFont(hICDisplay, hICPrinter, &lfd, &lfp) ;

	ReleaseDC(hWnd, hICDisplay) ;
	DeleteDC(hICPrinter) ;

	memcpy(&lpdlgrpt->tlrptdsp.lfp, &lfp, sizeof(LOGFONT)) ;
	memcpy(&lpdlgrpt->tlrptdsp.lfd, &lfd, sizeof(LOGFONT)) ;

#if	DEBUG
	fprintf(
		fpdbg,
		"initial face='%s' height=%d weight=%d charset=%d pf=%d\n",
		lpdlgrpt->tlrptdsp.lfp.lfFaceName,
		lpdlgrpt->tlrptdsp.lfp.lfHeight,
		lpdlgrpt->tlrptdsp.lfp.lfWeight,
		lpdlgrpt->tlrptdsp.lfp.lfCharSet,
		lpdlgrpt->tlrptdsp.lfp.lfPitchAndFamily
	) ;
#endif

	return(TRUE) ;
}

static BOOL lclInvokeDialogSF(HWND hWnd, LPDLGRPT lpdlgrpt)
{
	CHOOSEFONT cf ;
	HDC hICPrinter, hICDisplay ;
	TRHELPREF hrOld ;

	/*
		choose new display logical font, from
		choice of fonts compatible with printer
	*/
	hICPrinter= tlrptdsp_CreateDC_FromPPD(&lpdlgrpt->pd, TRUE) ;
	if (hICPrinter == (HDC) NULL) return(FALSE) ;

	cf.lStructSize=	sizeof(CHOOSEFONT) ;
	cf.hwndOwner=	hWnd ;
	cf.lpLogFont=	&lpdlgrpt->tlrptdsp.lfd ;
	cf.hDC=		hICPrinter ;
	cf.nFontType=	PRINTER_FONTTYPE ;

	cf.Flags= CF_BOTH
		| CF_SCALABLEONLY
		| CF_WYSIWYG
		| CF_INITTOLOGFONTSTRUCT
	;

	hrOld= tlrpthlp_SetHelpRef(TRHELPREF_NONE) ;
	ChooseFont(&cf) ;
	tlrpthlp_SetHelpRef(hrOld) ;

	/*
		get logical font for printer that is
		"identical" to display logical font
	*/
	hICDisplay= GetDC(hWnd) ;
	lclGetCompatibleLogicalFont(
		hICPrinter,
		hICDisplay,
		&lpdlgrpt->tlrptdsp.lfp,
		&lpdlgrpt->tlrptdsp.lfd
	) ;
	ReleaseDC(hWnd, hICDisplay) ;
	DeleteDC(hICPrinter) ;

#if	DEBUG
	fprintf(
		fpdbg,
		"new face='%s' height=%d weight=%d charset=%d pf=%d\n",
		lpdlgrpt->tlrptdsp.lfp.lfFaceName,
		lpdlgrpt->tlrptdsp.lfp.lfHeight,
		lpdlgrpt->tlrptdsp.lfp.lfWeight,
		lpdlgrpt->tlrptdsp.lfp.lfCharSet,
		lpdlgrpt->tlrptdsp.lfp.lfPitchAndFamily
	) ;
#endif

	return(TRUE) ;
}

/* end of tlrpt.c */
