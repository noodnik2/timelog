#define	Revision	79

#define	TLRPTDSP_DEBUG		0

/*

	tlrptdsp.c

*/

#include	"wininc.h"

#include	<malloc.h>
#include	<memory.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<print.h>

#include	"resource.h"
#include	"tlrptdsp.h"
#include	"tlrpthlp.h"

/*
	internal #definitions, structures
*/

#define	MAXSIZE_PTIME  		32

#define	MAXCOLS			62	/* max. # of columns for report page */
#define	MAXROWS			64

#define	AF_USER			1 	/* abort flags	*/
#define	AF_PRINT		2
#define	AF_ERROR       		4

#define	IA_LEFT			1	/* alignment flags */
#define	IA_RIGHT       		2     	
#define	IA_CENTER      		4
#define	IAO_WRAP       		8	/* and options */

typedef struct tagPAGESTATE {
	BOOL bIsPrinter ;		/* is this a printer device?	*/
	HDC hDC ;			/* device context		*/
	HFONT hFont ;			/* font to select for each page */
	RECT rWindow ;			/* "redraw window"		*/
	int pgHorgPos ;			/* horz origin			*/
	int pgVorgPos ;			/* vert origin			*/
	int pgVsize ;			/* physical total vert pagesize */
	int pgHsize ;			/* physical total horz pagesize */
	int pgHmgn ;			/* horz margin			*/
	int pgVmgn ;			/* vert margin			*/
	int pgHeight ;			/* height of printable page	*/
	int pgWidth ;			/* width of printable page	*/
	int rowVsize ;			/* height of row		*/
	int colHsize ;			/* approx width of col		*/
	int loginchVsize ;		/* "logical inch" vertical	*/
	int loginchHsize ;		/* "logical inch" horizontal	*/
	int pgRows ;			/* rows on page			*/
	int pgCols ;			/* approx cols on page		*/
	int pgNum ;			/* current page number		*/
	int pgVdrawPos ;		/* current vertical draw pos	*/
	int pgVpagePos ;		/* current top of page pos	*/
} PAGESTATE, *PPAGESTATE ;

typedef struct tagRPT {
	HWND hWndParent ;
	TLRPTDSP *prd ;
	LOGRPT *plr ;
	PAGESTATE ps ;
} RPT, *PRPT ;

/*
	internal data/variables
*/

static rp_errno= RPE_SUCCESS ;
static HWND hWndAbortDlg ;
static int iAbortFlags= 0 ;
static char szErrorTitle[]= "TimeLog Report Error" ;

#if	TLRPTDSP_DEBUG
static FILE *fpdbg ;
#endif

/*
	public function templates
*/

int CALLBACK __export AbortDialogProc(HWND, UINT msg, WPARAM, LPARAM) ;
BOOL CALLBACK __export AbortProc(HDC, int fCode) ;

/*
	internal function templates
*/

static BOOL lclErrorMsg(HWND, char *, DWORD) ;
static char *pszTime(time_t tTime) ;
static char *pszCommentText(LOGRPTCOMMENT *plrpc, TLRPTDSP *prd) ;
static char *pszltval(long ltval, TLRPTDSP *prd) ;

static void lclInitPageSize(PPAGESTATE, HDC hDCDsp, HDC hDCPrn, BOOL bIsPrn) ;
static void lclInitPageDraw(PPAGESTATE pps, HDC hDC, HFONT hFont) ;
static BOOL lclPrintRpt(PRPT prpt) ;
static BOOL lclPrintHeader(PRPT prpt) ;
static BOOL lclPrintTables(PRPT prpt) ;
static BOOL lclPrintComments(PRPT prpt) ;
static BOOL lclPrintStart(PRPT prpt) ;
static BOOL lclPrintEject(PRPT prpt) ;

/*
	start of entry points
*/

BOOL tlrptdsp_Init()
{
	rp_errno= RPE_SUCCESS ;
#if	TLRPTDSP_DEBUG
	fpdbg= fopen("tlrptdsp.log", "w") ;
	if (fpdbg == (FILE *) NULL) return(FALSE) ;
	fprintf(fpdbg, "Revision: %d\n", Revision) ;
#endif
	return(TRUE) ;
}

void tlrptdsp_Fini()
{
#if	TLRPTDSP_DEBUG
	if (fpdbg == (FILE *) NULL) return ;
	fprintf(fpdbg, "End Log, rp_errno=%d\n", rp_errno) ;
	fclose(fpdbg) ;
	fpdbg= (FILE *) NULL ;
#endif
}

int tlrptdsp_errno()
{
	return(rp_errno) ;
}

HDC tlrptdsp_CreateDC_FromPPD(PRINTDLG *ppd, BOOL bCreateIC)
{	
	LPDEVNAMES lpDevNames ;
	LPDEVMODE lpDevMode ;
	LPSTR lpszDriverName ;
	LPSTR lpszDeviceName ;
	LPSTR lpszPortName ;
	HDC hDC ;

	if (ppd->hDevNames == (HGLOBAL) NULL) return((HDC) NULL) ;

	lpDevMode=	(LPDEVMODE) NULL ;
	lpDevNames=	(LPDEVNAMES) GlobalLock(ppd->hDevNames) ;

	lpszDriverName=	(LPSTR) lpDevNames + lpDevNames->wDriverOffset ;
	lpszDeviceName=	(LPSTR) lpDevNames + lpDevNames->wDeviceOffset ;
	lpszPortName=	(LPSTR) lpDevNames + lpDevNames->wOutputOffset ;
	GlobalUnlock(ppd->hDevNames) ;
	if (ppd->hDevMode != (HGLOBAL) NULL) {
		lpDevMode= (LPDEVMODE) GlobalLock(ppd->hDevMode) ;
	}
	if (bCreateIC) hDC= CreateIC(
		lpszDriverName, lpszDeviceName, lpszPortName, lpDevMode
	) ;
	else hDC= CreateDC(
		lpszDriverName, lpszDeviceName, lpszPortName, lpDevMode
	) ;
	if (
		(ppd->hDevMode != (HGLOBAL) NULL)
	     && (lpDevMode != (LPDEVMODE) NULL)
	) {
		GlobalUnlock(ppd->hDevMode) ;
	}
	return(hDC) ;
}

BOOL tlrptdsp_Print(
	HWND hWndParent,
	LOGRPT *plr,
	TLRPTDSP *prd,
	PRINTDLG *ppd
) {
	DOCINFO di ;
	DLGPROC lpAbortDialogProc ;
	ABORTPROC lpAbortProc ;
	RPT rpt ;
	HDC hDC ;
	HFONT hFont, hFontOld ;
	DWORD FlagsSave ;
	BOOL bOk ;
	TRHELPREF hrOld ;
	
	rp_errno= RPE_SUCCESS ;

	rpt.hWndParent= hWndParent ;
	rpt.plr= plr ;
	rpt.prd= prd ;

	FlagsSave= ppd->Flags ;
	ppd->hwndOwner= hWndParent ;
	ppd->Flags|= PD_RETURNDC ;

	hrOld= tlrpthlp_SetHelpRef(TRHELPREF_NONE) ;
	bOk= PrintDlg(ppd) ;
	tlrpthlp_SetHelpRef(hrOld) ;

	ppd->Flags= FlagsSave ;
	if (!bOk) {
		rp_errno= RPE_CANCELED ;
		return(FALSE) ;
	}
	hDC= ppd->hDC ;

	if (hDC == (HDC) NULL) {
		lclErrorMsg(hWndParent, "No Printer DC available.", 0) ;
		rp_errno= RPE_CANTCREATE ;	/* can't create print job */
		return(FALSE) ;
	}
	hFont= CreateFontIndirect(&rpt.prd->lfp) ;
	if (hFont != (HFONT) NULL) {
		hFontOld= SelectObject(hDC, hFont) ;
	}
	lpAbortDialogProc= (DLGPROC) MakeProcInstance(
		(FARPROC) AbortDialogProc, hInst
	) ;
	hWndAbortDlg= CreateDialogParam(
		hInst,
		"ABORTDLG",
		hWndParent,
		lpAbortDialogProc,
		(LPARAM) ((LPSTR) plr->szName)
	) ;
	if (hWndAbortDlg == (HWND) NULL) {
		lclErrorMsg(hWndParent, "Can't create abort dialog", 0) ;
		rp_errno= RPE_FAILURE ;		/* failure for abort dialog */
	}
	else {
		di.cbSize= sizeof(DOCINFO) ;
		di.lpszDocName= plr->szName ;
		di.lpszOutput= NULL ;
		lpAbortProc= (ABORTPROC) MakeProcInstance(
			(FARPROC) AbortProc, hInst
		) ;
		iAbortFlags= 0 ;
		SetAbortProc(hDC, lpAbortProc) ;
		if (StartDoc(hDC, &di)< 0) {
			lclErrorMsg(hWndParent, "StartDoc error", 0) ;
			rp_errno= RPE_CANTCREATE ;
		}
		else {
			EnableWindow(hWndParent, FALSE) ;
			lclInitPageSize(&rpt.ps, hDC, hDC, TRUE) ;
			lclInitPageDraw(&rpt.ps, hDC, hFont) ;
			if (lclPrintRpt(&rpt)) EndDoc(hDC) ;
			EnableWindow(hWndParent, TRUE) ;
		}
		FreeProcInstance((FARPROC) lpAbortProc) ;
		DestroyWindow(hWndAbortDlg) ;
	}
	FreeProcInstance((FARPROC) lpAbortDialogProc) ;
	if (hFont != (HFONT) NULL) {
		SelectObject(hDC, hFontOld) ;
		DeleteObject(hFont) ;
	}
	DeleteDC(hDC) ;

	return((BOOL) (rp_errno == RPE_SUCCESS)) ;
}

HRPT tlrptdsp_CreateLink(
	HWND hWnd,
	LOGRPT *plr,
	TLRPTDSP *prd,
	PRINTDLG *ppd
) {
	PRPT prpt ;
	HDC hDCDisplay, hDCPrinter ;

	prpt= malloc(sizeof(RPT)) ;
	if (prpt == (PRPT) NULL) {
		return((PRPT) NULL) ;
	}
	prpt->plr= plr ;
	prpt->prd= prd ;
	prpt->hWndParent= hWnd ;
	iAbortFlags= 0 ;

	hDCDisplay= GetDC(hWnd) ;
	hDCPrinter= tlrptdsp_CreateDC_FromPPD(ppd, TRUE) ;

	lclInitPageSize(&prpt->ps,hDCDisplay,hDCPrinter, FALSE) ;

	DeleteDC(hDCPrinter) ;
	ReleaseDC(hWnd, hDCDisplay) ;

	return((HRPT) prpt) ;
}

BOOL tlrptdsp_DestroyLink(HRPT hRpt, HWND hWnd)
{
	PRPT prpt ;

	prpt= (PRPT) hRpt ;
	if (prpt == (PRPT) NULL) return(FALSE) ;
	free(prpt) ;
	return(TRUE) ;
}

#define	VTOP   		0
#define	VBOTTOM		(prpt->ps.pgVpagePos - prpt->ps.pgVmgn)
#define	VPAGE  		(prpt->ps.pgVsize / 4)
#define	VLINE  		(prpt->ps.rowVsize)
#define	SETVPOS(y)	prpt->ps.pgVorgPos= -(y)
#define	GETVPOS		(-prpt->ps.pgVorgPos)

#define	HTOP   		0
#define	HBOTTOM		(prpt->ps.pgHmgn + prpt->ps.pgWidth)
#define	HPAGE  		(prpt->ps.pgHsize / 4)
#define	HLINE  		(prpt->ps.colHsize)
#define	SETHPOS(x)	prpt->ps.pgHorgPos= -(x)
#define	GETHPOS		(-prpt->ps.pgHorgPos)

#if	DEBUG
#define	ASSERTRPT()		       			\
	if (prpt == (PRPT) NULL) {     			\
		fprintf(fplog, "tlrptdsp_Link: NULL prpt " __LINE__) ;	\
		MessageBeep(0) ;			\
		return(FALSE) ;	       	      		\
		break ;		       	       		\
	}
#else
#define	ASSERTRPT()		       			\
	if (prpt == (PRPT) NULL) {     			\
		return(FALSE) ;	       	      		\
		break ;		       	       		\
	}
#endif

BOOL tlrptdsp_Link(
	HRPT hRpt,
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	PAINTSTRUCT pst ;
	BOOL bUpdate ;
	PRPT prpt ;
	HDC hDC ;
	HFONT hFont, hFontOld ;
	int nPos ;

	prpt= (PRPT) hRpt ;

	switch(message) {

		case	UM_UPDATESCROLL:
			if (GETHPOS> HBOTTOM)	SETHPOS(HBOTTOM) ;
			if (GETHPOS< HTOP)	SETHPOS(HTOP) ;
			if (GETVPOS> VBOTTOM)	SETVPOS(VBOTTOM) ;
			if (GETVPOS< VTOP)	SETVPOS(VTOP) ;
			InvalidateRect(hWnd, NULL, TRUE) ;
			return(TRUE) ;
			break ;

		case	WM_VSCROLL:
			ASSERTRPT() ;
			bUpdate= TRUE ;
			nPos= LOWORD(lParam) ;
			switch(wParam) {
				case	SB_THUMBPOSITION:
					SETVPOS(nPos) ;
					break ;
				case	SB_TOP:
					SETVPOS(VTOP) ;
					break ;
				case	SB_BOTTOM:
					SETVPOS(VBOTTOM) ;
					break ;
				case	SB_LINEDOWN:
					SETVPOS(GETVPOS + VLINE) ;
					break ;
				case	SB_LINEUP:
					SETVPOS(GETVPOS - VLINE) ;
					break ;
				case	SB_PAGEDOWN:
					SETVPOS(GETVPOS + VPAGE) ;
					break ;
				case	SB_PAGEUP:
					SETVPOS(GETVPOS - VPAGE) ;
					break ;
				default:
					bUpdate= FALSE ;
					break ;
			}
			if (bUpdate) SendMessage(hWnd, UM_UPDATESCROLL, 0, 0) ;
			return(TRUE) ;
			break ;

		case	WM_HSCROLL:
			ASSERTRPT() ;
			bUpdate= TRUE ;
			nPos= LOWORD(lParam) ;
			switch(wParam) {
				case	SB_THUMBPOSITION:
					SETHPOS(nPos) ;
					break ;
				case	SB_TOP:
					SETHPOS(HTOP) ;
					break ;
				case	SB_BOTTOM:
					SETHPOS(HBOTTOM) ;
					break ;
				case	SB_LINEDOWN:
					SETHPOS(GETHPOS + HLINE) ;
					break ;
				case	SB_LINEUP:
					SETHPOS(GETHPOS - HLINE) ;
					break ;
				case	SB_PAGEDOWN:
					SETHPOS(GETHPOS + HPAGE) ;
					break ;
				case	SB_PAGEUP:
					SETHPOS(GETHPOS - HPAGE) ;
					break ;
				default:
					bUpdate= FALSE ;
					break ;
			}
			if (bUpdate) SendMessage(hWnd, UM_UPDATESCROLL, 0, 0) ;
			return(TRUE) ;
			break ;

		case	WM_PAINT:
			ASSERTRPT() ;

			hDC= BeginPaint(hWnd, &pst) ;
			if (hDC == (HDC) NULL) {
#if	DEBUG
				fprintf(fpdbg, "tlrptdsp_Link: PAINT: no hDC!\n") ;
				MessageBeep(1) ;
#endif
				break ;
			}

			CopyRect(&prpt->ps.rWindow, &pst.rcPaint) ;

			hFont= CreateFontIndirect(&prpt->prd->lfd) ;
			if (hFont != (HFONT) NULL) {
				hFontOld= SelectObject(hDC, hFont) ;
			}

			lclInitPageDraw(&prpt->ps, hDC, hFont) ;
			lclPrintRpt(prpt) ;

			if (hFont != (HFONT) NULL) {
				SelectObject(hDC, hFontOld) ;
				DeleteObject(hFont) ;
			}

			EndPaint(hWnd, &pst) ;
 
			SetScrollRange(hWnd, SB_VERT, VTOP, VBOTTOM, FALSE) ;
			SetScrollPos(hWnd, SB_VERT, GETVPOS, TRUE) ;
			SetScrollRange(hWnd, SB_HORZ, HTOP, HBOTTOM, FALSE) ;
			SetScrollPos(hWnd, SB_HORZ, GETHPOS, TRUE) ;

			return(TRUE) ;
			break ;

	}

	return(FALSE) ;
}

/*
	Windows callback procedures: dialog proc and abort proc
*/

BOOL CALLBACK __export AbortProc(HDC hPrtDC, int fCode)
{
	MSG msg ;
	
	while(
		(iAbortFlags == 0)
	     && PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)
	) {
		if (!IsDialogMessage(hWndAbortDlg, &msg)) {
			TranslateMessage(&msg) ;
			DispatchMessage(&msg) ;
		}
	}
	return((BOOL) (iAbortFlags == 0)) ;
}

int CALLBACK __export AbortDialogProc(
	HWND hDlg,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
) {
	switch(msg) {
		case	WM_INITDIALOG:
			SetDlgItemText(hDlg, IDC_EDITNAME, (LPSTR) lParam) ;
			return(TRUE) ;
			break ;
		case	WM_COMMAND:
#if	TLRPTDSP_DEBUG
			fprintf(fpdbg, "AF_USER from WM_COMMAND\n") ;
#endif
			iAbortFlags|= AF_USER ;
			return(TRUE) ;
			break ;
	}
	return(FALSE) ;
}

/*
	start of internal subroutines
*/

static void lclShowPreviewPage(PPAGESTATE pps)
{
	RECT rPage ;
	HPEN hPen, hPenOld ;
	HDC hDC ;

	hDC= pps->hDC ;
	hPen= CreatePen(PS_DOT, 1, RGB(127, 127, 127)) ;
	hPenOld= SelectObject(hDC, hPen) ;

	rPage.left= pps->pgHorgPos + pps->pgHmgn ;
	rPage.top= pps->pgVorgPos + pps->pgVpagePos + pps->pgVmgn ;
	rPage.right= rPage.left + pps->pgWidth - 1 ;
	rPage.bottom= rPage.top + pps->pgHeight - 1 ;

	MoveTo(hDC, rPage.left, rPage.top) ;
	LineTo(hDC, rPage.right, rPage.top) ;
	LineTo(hDC, rPage.right, rPage.bottom) ;
	LineTo(hDC, rPage.left, rPage.bottom) ;
	LineTo(hDC, rPage.left, rPage.top) ;

	SelectObject(hDC, hPenOld) ;
	DeleteObject(hPen) ;

	hPen= CreatePen(PS_DASH, 1, RGB(127, 127, 127)) ;
	hPenOld= SelectObject(hDC, hPen) ;
		
	rPage.left= pps->pgHorgPos ;
	rPage.top= pps->pgVorgPos + pps->pgVpagePos + pps->pgVsize ;
	rPage.right= rPage.left + pps->pgHsize - 1 ;

	MoveTo(hDC, rPage.left, rPage.top-1) ;
	LineTo(hDC, rPage.right, rPage.top-1) ;
	MoveTo(hDC, rPage.left, rPage.top) ;
	LineTo(hDC, rPage.right, rPage.top) ;
	MoveTo(hDC, rPage.left, rPage.top+1) ;
	LineTo(hDC, rPage.right, rPage.top+1) ;

	SelectObject(hDC, hPenOld) ;
	DeleteObject(hPen) ;
}

static void lclInitPageSize(
	PPAGESTATE pps,
	HDC hDCDisplay,
	HDC hDCPrinter,
	BOOL bIsPrinter
) {
	double d ;

	memset((void *) pps, '\0', sizeof(PAGESTATE)) ;

	pps->hDC= hDCPrinter ;

	pps->bIsPrinter= bIsPrinter ;
	pps->pgHorgPos= 0 ;
	pps->pgVorgPos= 0 ;

        pps->pgVsize= GetDeviceCaps(hDCPrinter, VERTRES) ;
        pps->pgHsize= GetDeviceCaps(hDCPrinter, HORZRES) ;

	pps->loginchVsize= GetDeviceCaps(hDCDisplay, LOGPIXELSY) ;
	pps->loginchHsize= GetDeviceCaps(hDCDisplay, LOGPIXELSX) ;

	if (!pps->bIsPrinter) {
		d= (double) pps->pgVsize ;
		d/= (double) GetDeviceCaps(hDCPrinter, LOGPIXELSY) ;
		d*= (double) pps->loginchVsize ;
		pps->pgVsize= (int) d ;
		d= (double) pps->pgHsize ;
		d/= (double) GetDeviceCaps(hDCPrinter, LOGPIXELSX) ;
		d*= (double) pps->loginchHsize ;
		pps->pgHsize= (int) d ;
	}

#if	TLRPTDSP_DEBUG
	fprintf(fpdbg, "Vsize= %d\n", pps->pgVsize) ;
	fprintf(fpdbg, "Hsize= %d\n", pps->pgHsize) ;
#endif

	SetRect(&pps->rWindow, 0, 0, pps->pgHsize, pps->pgVsize) ;
}

static void lclInitPageDraw(PPAGESTATE pps, HDC hDC, HFONT hFont)
{
	TEXTMETRIC tm ;

	if (hFont != (HFONT) NULL) SelectObject(hDC, hFont) ;

	GetTextMetrics(hDC, &tm) ;

	pps->hDC= hDC ;
	pps->hFont= hFont ;
	pps->pgNum= 0 ;
	pps->pgVdrawPos= 0 ;
	pps->pgVpagePos= 0 ;

	pps->rowVsize= tm.tmHeight + tm.tmExternalLeading ;
	pps->colHsize= tm.tmMaxCharWidth + tm.tmOverhang ;

	pps->pgVmgn= pps->loginchVsize / 2 ;
	pps->pgHmgn= pps->loginchHsize / 2 ;
	pps->pgWidth= pps->pgHsize - (2 * pps->pgHmgn) ;
	pps->pgHeight= pps->pgVsize - (2 * pps->pgVmgn) ;
	pps->pgCols= (pps->pgWidth / pps->colHsize) ;
	pps->pgRows= (pps->pgHeight / pps->rowVsize) ;
}

static void lclStartPhysicalPage(PPAGESTATE pps)
{
	if (pps->bIsPrinter) {
		if (StartPage(pps->hDC)< 0) {
			iAbortFlags|= AF_PRINT ;
#if	TLRPTDSP_DEBUG
			fprintf(fpdbg, "AF_PRINT from StartPage\n") ;
#endif
		}
		pps->pgVpagePos= 0 ;
	}
	pps->pgVdrawPos= 0 ;
	if (pps->hFont != (HFONT) NULL) {
		SelectObject(pps->hDC, pps->hFont) ;
	}
}

static void lclEndPhysicalPage(PPAGESTATE pps)
{
	if (pps->bIsPrinter) {
		if (EndPage(pps->hDC)< 0) {
			iAbortFlags|= AF_PRINT ;
#if	TLRPTDSP_DEBUG
			fprintf(fpdbg, "AF_PRINT from EndPage\n") ;
#endif
		}
	}
}

static void lclFirstPage(PPAGESTATE pps)
{
	pps->pgNum= 0 ;
	pps->pgVpagePos= 0 ;
	lclStartPhysicalPage(pps) ;
}

static void lclNextPage(PPAGESTATE pps, BOOL bMore)
{
	pps->pgNum++ ;
	lclEndPhysicalPage(pps) ;
	if (!pps->bIsPrinter) {
		lclShowPreviewPage(pps) ;
		pps->pgVpagePos+= pps->pgVsize ;
	}
	if (bMore) lclStartPhysicalPage(pps) ;
}

static void lclAdvancePage(PPAGESTATE pps, int pgVertLen, BOOL bMove)
{
	if ((pps->pgVdrawPos + pgVertLen)>= (pps->pgHeight)) {
		lclNextPage(pps, TRUE) ;
		return ;
	}
	if (bMove) {
		(pps->pgVdrawPos)+= pgVertLen ;
	}
}

static void lclDrawText(
	PPAGESTATE pps,
	char *pszText,
	int iAlign,
	BOOL bDraw,
	RECT *pr
) {
	UINT fuFormat ;
	RECT rDraw, rIntersection ;

	fuFormat= DT_EXTERNALLEADING
		| DT_NOPREFIX
		| DT_EXPANDTABS
		| DT_NOCLIP
	;
	if (iAlign & IA_LEFT)		fuFormat|= DT_LEFT ;
	else if (iAlign & IA_RIGHT)	fuFormat|= DT_RIGHT ;
	else if (iAlign & IA_CENTER)	fuFormat|= DT_CENTER ;
	if (iAlign & IAO_WRAP)		fuFormat|= DT_WORDBREAK ; 
	if (!bDraw)			fuFormat|= DT_CALCRECT ;
	else {
		CopyRect(&rDraw, pr) ;
		OffsetRect(
			&rDraw,
			pps->pgHorgPos + pps->pgHmgn,
			pps->pgVpagePos + pps->pgVorgPos + pps->pgVmgn
		) ;
		if (!IntersectRect(&rIntersection, &pps->rWindow, &rDraw)) {
			return ;
		}
		pr= &rDraw ;
	}
	DrawText(pps->hDC, pszText, -1, pr, fuFormat) ;
}

static BOOL lclPrintRpt(PRPT prpt)
{
	lclPrintStart(prpt) ;
	lclPrintHeader(prpt) ;
	lclPrintTables(prpt) ;
	lclPrintComments(prpt) ;
	lclPrintEject(prpt) ;
	if (iAbortFlags != 0) {
		if (iAbortFlags & AF_USER) rp_errno= RPE_CANCELED ;
		else if (iAbortFlags & AF_PRINT) rp_errno= RPE_ABORTED ;
		else rp_errno= RPE_FAILURE ;
		return(FALSE) ;
	}
	return(TRUE) ;
}

static char *pszltval(long ltval, TLRPTDSP *prd)
{
	static char szTbuf[MAXSIZE_TIMESCOPE+1] ;
	double dTime ;
	int hh, mm ;

	if (prd->bSparse) {
		if (ltval == 0) {
			szTbuf[0]= '\0' ;
			return(szTbuf) ;
		}
	}

	if (prd->iTimeDisplayFormat == TDF_HFRACT) {
		dTime= (double) ltval ;
		dTime/= (double) (60.0e0 * 60.0e0) ;
		sprintf(szTbuf, "%.2lf", dTime) ;
		return(szTbuf) ;
	}
	/* else prd->iTimeDisplayFormat == TDF_HHMM */
	hh= (int) (ltval / (60L * 60L)) ;
	mm= (int) ((ltval % (60L * 60L)) / 60L) ;
	if (mm< 0) {	hh++ ;	mm+= 60 ;	}
	sprintf(szTbuf, "%d:%02d", hh, mm) ;
	return(szTbuf) ;
}

static char *pszTime(time_t tTime)
{
	static char szTime[MAXSIZE_PTIME+1] ;
	char szTimeTemp[MAXSIZE_PTIME+1] ;

	lstrcpy(szTimeTemp, asctime(localtime(&tTime))) ;

	/*
		szTimeTemp is: 	"Fri Jul 15 16:52:00 1994"
	*/
	memcpy(szTime, szTimeTemp+4, 12) ;
	szTime[12]= ' ' ;
	memcpy(szTime+13, szTimeTemp+20, 4) ;
	szTime[17]= '\0' ;

	/*
		szTime is: 	"Jul 15 16:52 1994"
	*/
	return(szTime) ;
}

static char *pszCommentText(LOGRPTCOMMENT *plrpc, TLRPTDSP *prp)
{
	static char szText[
		32+LRMAXSIZE_ENAME+MAXSIZE_PTIME+MAXSIZE_TIMESCOPE+1
	] ;
	
	switch(plrpc->ctype) {
		case	LRCT_COMMENT:
			return(plrpc->pszText) ;
			break ;
		case	LRCT_ADJUSTMENT:
			sprintf(
				szText,
				"Adjustment to '%s' on %s: %s",
				plrpc->pszText,
				pszTime(plrpc->tTime),
				pszltval(plrpc->lval, prp)
			) ;
			return(szText) ;	
			break ;
		case	LRCT_ALLOCATION:
			sprintf(
				szText,
				"Allocation to '%s' until %s: %s",
				plrpc->pszText,
				pszTime(plrpc->tTime),
				pszltval(plrpc->lval, prp)
			) ;
			return(szText) ;	
			break ;
	}
	return(NULL) ;
}

static BOOL lclErrorMsg(HWND hWndParent, char *pszErrorMsg, DWORD dwrc)
{
	char emsg[256] ;

	lstrcpy(emsg, pszErrorMsg) ;
	if (dwrc != 0L) {
		wsprintf(
			(LPSTR)(emsg + lstrlen(emsg)),
			"\n(error code == 0x%lX)",
			dwrc
		) ;
	}
	EnableWindow(hWndParent, FALSE) ;	/* is there a better way? */
	MessageBox(				/* APPMODAL does not do it */
		hWndParent,			/* if hWndParent is a dialog */
		emsg,
		szErrorTitle,
		MB_OK | MB_ICONEXCLAMATION
	) ;
	EnableWindow(hWndParent, TRUE) ;
	return(FALSE) ;
}

#define	TDF_INIT		0
#define	TDF_FINI		1
#define	TDF_FIRSTROW		2
#define	TDF_NEXTROW		3
#define	TDF_FIRSTCOL		4
#define	TDF_NEXTCOL		5
#define	TDF_LEFTCOL		6
#define	TDF_SETLEFTCOL		7
#define	TDF_GETSIZE		8
#define	TDF_LAYOUT		9
#define	TDF_DRAW		10
#define	TDF_ROWHASDATA		11
#define	TDF_COLHASDATA		12
#define	TDF_ENOUGHCOLS		13

#define	VSIZER(r)	((r).bottom - (r).top)
#define	HSIZER(r)	((r).right - (r).left)
#define	VSIZEC(C,r,c)	(C[r][c].height)
#define	HSIZEC(C,r,c)	(C[r][c].width)

typedef struct tagCOL {
	int pos ;			/* column position		*/
	int width ;			/* column width			*/
} COL ;

typedef struct tagCELL {
	int width ;
	int height ;
} CELL ;

typedef struct tagTABLE {
	time_t tNow ;			/* time table generated		  */
 	int irow ;			/* current row			  */
	int icol ;			/* current column		  */
	int lcol ;			/* leftmost column being printed  */
	int rcol ;			/* rightmost column being printed */
	int vmgn ;			/* vertical clearance: margin	  */
	COL col[MAXCOLS] ;		/* column definition array	  */
	CELL cell[MAXROWS][MAXCOLS] ;	/* cell draw sizes		  */
	RECT rect ;			/* item formatting rectangle	  */
	LOGRPT *plr ;			/* logrpt structure		  */
	TLRPTDSP *prd ;			/* report display struct 	  */
	PPAGESTATE pps ;		/* pagestate output structure	  */
	BOOL bOverflow ;		/* "something overflowed" flag    */
} TABLE ;

static void initrect(RECT *rp)
{
	SetRectEmpty(rp) ;
	rp->right++ ;
	rp->bottom++ ;
}

static BOOL sempty(char *s)
{
	if (s == (char *) NULL) return(TRUE) ;
	while((*s) != '\0') {
		if (!isspace(*s)) return(FALSE) ;
		s++ ;
	}
	return(TRUE) ;
}

/*
	the function whose type is defined here will be called once for
	each table cell.  The function will calculate the rectangle
	required by, or draw (according to the parameter 'bDraw' the
	table entry specified by the coordinate: (tp->irow, tp->icol).

	The function returns TRUE if it succeeds in fitting the cell
	onto the page (defined by "*pps").

	It modifies the values of the "rect", and the "tblrect"
	members.  All others it does not modify.
	
	notes:
	
		1.	get maxlen[col] for all cols		GETSIZE
		2.	get rightmost col that fits on page	LAYOUT
		3.	draw table				DRAW
	
*/
typedef BOOL (*fndrawcell)(TABLE *tp, int tdf) ;

typedef struct tagPROJ {
	TABLE *tp ;
	int irow ;
	int icol ;
	int lcol ;
} PROJ ;

/*
	standard support for TDF_LAYOUT

	assumes that 'tp->col' and 'tp->cell' arrays are valid.
	returns in 'tp->rect' the rectangle to use for output.
*/
static BOOL std_dclayout(TABLE *tp)
{
	tp->rect.top= (tp->pps)->pgVdrawPos ;
	tp->rect.left= tp->col[tp->icol].pos ;
	tp->rect.bottom= tp->rect.top + tp->cell[tp->irow][tp->icol].height ;
	tp->rect.right= tp->rect.left + tp->col[tp->icol].width ;
	return(TRUE) ;
}

static char *getprojstring(PROJ *pp)
{
	static char szPName[MAXSIZE_TIMESCOPE+1] ;
	TABLE *tp ;
	LOGRPT *plr ;
	int j ;

	tp= pp->tp ;
	plr= tp->plr ;

	if (tp->icol<= tp->lcol) {			/* entry title */
		if (tp->irow<= 0) {			/* "Entry" */
			return("Entry") ;
		}
		return(plr->plrentry[pp->irow].pszName) ;
	}

	if (tp->irow<= 0) {				/* period title */
		for (j= 0; j< 6; j++) {
			if (plr->plrperiod[pp->icol].szName[j] == '\0') break ;
			szPName[j]= plr->plrperiod[pp->icol].szName[j] ;
		}
		szPName[j]= '\0' ;
		return(szPName) ;
	}
	return(pszltval(plr->plrentry[pp->irow].plTime[pp->icol], tp->prd)) ;
}

static BOOL dc_project(TABLE *tp, int tdf)
{
	static PROJ p ;
	char *s ;
	CELL *cp ;
	int i, charwidth ;

	charwidth= (tp->pps)->colHsize ;
	p.tp= tp ;

	if (tdf == TDF_INIT) {
		tp->vmgn= tp->pps->loginchVsize / 2 ; /* give 1/2 inch margin */
		return(TRUE) ;
	}

	if (tdf == TDF_FIRSTROW) {
		p.irow= 0 ;
		return(TRUE) ;
	}
	
	if (tdf == TDF_FIRSTCOL) {
		p.lcol= 0 ;
		return(TRUE) ;
	}

	if (tdf == TDF_LEFTCOL) {
		p.icol= p.lcol ;
		if (p.icol>= ((tp->plr)->nperiods)) {
			return(FALSE) ;
		}
		return(TRUE) ;
	}

	if (tdf == TDF_SETLEFTCOL) {
		p.lcol= p.icol ;
		if (p.lcol>= ((tp->plr)->nperiods)) {
			return(FALSE) ;
		}
		return(TRUE) ;
	}

	if (tdf == TDF_NEXTROW) {
		if (tp->irow> 0) p.irow++ ;
		if (p.irow>= ((tp->plr)->nentries)) {
			return(FALSE) ;
		}
		return(TRUE) ;
	}

	if (tdf == TDF_NEXTCOL) {
		if (tp->icol> tp->lcol) p.icol++ ;
		if (p.icol>= ((tp->plr)->nperiods)) {
			return(FALSE) ;
		}
		return(TRUE) ;
	}

	/*
		returns the size of the formatted item in tp->cell[r][c]'
		also returns as zero-origin rectangle in 'tp->rect'
	*/
	if (tdf == TDF_GETSIZE) {
		s= getprojstring(&p) ;
		cp= &tp->cell[tp->irow][tp->icol] ;
		SetRectEmpty(&tp->rect) ;
		if (!sempty(s)) {
			lclDrawText(tp->pps, s, IA_RIGHT, FALSE, &tp->rect) ;
			tp->rect.right+= (2 * charwidth) ;
		}
		cp->width= HSIZER(tp->rect) ;
		cp->height= VSIZER(tp->rect) ;
		if (cp->height<= 0) cp->width= 0 ;	/* test */
		if (cp->width<= 0) cp->height= 0 ;	/* test */
		return(TRUE) ;
	}
        
	/*
		assumes that 'tp->rcol' contains rightmost col
		for table segment being printed.  (also assumes
		that 'tp->lcol' equals 0).
	*/
        if (tdf == TDF_ROWHASDATA) {
	      	for (i= 1; i< tp->rcol; i++) {
			if (tp->cell[tp->irow][i].height> 0) {
				return(TRUE) ;
			}
        	}
		return(FALSE) ;
        }

	/*
		assumes only that cell width is set.
	*/
        if (tdf == TDF_COLHASDATA) {
	      	for (i= 1; i< (tp->plr)->nentries; i++) {
			if (tp->cell[i][tp->icol].width> 0) {
				return(TRUE) ;
			}
        	}
		return(FALSE) ;
        }

        if (tdf == TDF_LAYOUT) return(std_dclayout(tp)) ;

	/*
		assumes that 'tp->rect' contains rectangle,
		to be used for drawing.
	*/
	if (tdf == TDF_DRAW) {
		if (!IsRectEmpty(&tp->rect)) {
			s= getprojstring(&p) ;
			lclDrawText(tp->pps, s, IA_RIGHT, TRUE, &tp->rect) ;
		}
		return(TRUE) ;
	}

	if (tdf == TDF_ENOUGHCOLS) {
		return((BOOL) ((tp->rcol - tp->lcol)> 1)) ;
//		return((BOOL) (p.lcol< p.icol)) ;
	}

	return(FALSE) ;			/* unknown table draw function */
}

typedef struct tagHDR {
	TABLE *tp ;
	int charwidth ;
	int iaflags ;
	int rpad ;
} HDR ;

static char *gethdrstring(HDR *hp)
{
	static char s[256] ;
	TABLE *tp ;

	tp= hp->tp ;

	if (tp->icol == 0) {
		hp->iaflags= IA_LEFT ;
		hp->rpad= 2 * hp->charwidth ;
	}
	else {
		hp->iaflags= IA_LEFT | IAO_WRAP ;
		hp->rpad= 0 ;
	}

	switch(	((tp->irow) * 10) + (tp->icol) ) {
		case    0:	return("User:") ;		break ;
		case    1:	return(tp->plr->szUser) ;	break ;
		case   10:	return("Report:") ;		break ;
		case   11:	return(tp->plr->szName) ;	break ;
		case   20:	return("Printed:") ;		break ;
		case   21:	return(pszTime(tp->tNow)) ;	break ;
		case   30:	return("Period:") ;		break ;
		case   31:
			lstrcpy(s, pszTime(tp->plr->tStart)) ;
			lstrcat(s, " through ") ;
			lstrcat(s, pszTime(tp->plr->tStop)) ;
			return(s) ;
			break ;
	}
	return((char *) "") ;
}

static BOOL dc_header(TABLE *tp, int tdf)
{
	static HDR h ;
	CELL *cp ;
	char *s ;
	int i, height ;

	if (tdf == TDF_INIT) {
		h.tp= tp ;
		h.charwidth= (tp->pps)->colHsize ;
		h.iaflags= IA_LEFT ;
		tp->vmgn= tp->pps->loginchVsize / 2 ; /* give 1/2 inch margin */
		return(TRUE) ;
	}

	if (tdf == TDF_NEXTROW)		return(tp->irow< 3) ;
	if (tdf == TDF_NEXTCOL)		return(tp->icol< 1) ;
	if (tdf == TDF_SETLEFTCOL)	return(tp->icol<= 1) ;

	/*
		returns the size of the formatted item in tp->cell[r][c]'
		also returns as zero-origin rectangle in 'tp->rect'
	*/
	if (tdf == TDF_GETSIZE) {
		SetRectEmpty(&tp->rect) ;
		if (tp->icol> 0) {
			tp->rect.right= (tp->pps)->pgWidth-(2*h.charwidth) ;
			height= 0 ;
			for (i= 0; i< tp->icol; i++) {
				tp->rect.right-= tp->cell[tp->irow][i].width ;
				if (height< tp->cell[tp->irow][i].height) {
					height= tp->cell[tp->irow][i].height ;
				}
			}
			tp->rect.bottom= height ;
		}
		s= gethdrstring(&h) ;
		if (!sempty(s)) {
			lclDrawText(tp->pps, s, h.iaflags, FALSE, &tp->rect) ;
			tp->rect.right+= h.rpad ;
		}
		cp= &tp->cell[tp->irow][tp->icol] ;
		cp->width= HSIZER(tp->rect) ;
		cp->height= VSIZER(tp->rect) ;
		return(TRUE) ;
	}
        
        if (tdf == TDF_LAYOUT) return(std_dclayout(tp)) ;

	/*
		assumes that 'tp->rect' contains rectangle,
		to be used for drawing.
	*/
	if (tdf == TDF_DRAW) {
		if (!IsRectEmpty(&tp->rect)) {
			s= gethdrstring(&h) ;
			lclDrawText(tp->pps, s, h.iaflags, TRUE, &tp->rect) ;
		}
		return(TRUE) ;
	}

	if (tdf == TDF_ENOUGHCOLS) {
		return((BOOL) ((tp->rcol - tp->lcol)> 1)) ;
	}

	return(TRUE) ;
}

typedef struct tagCMTS {
	TABLE *tp ;
	int charwidth ;
	int rpad ;
	int iaflags ;
} CMTS ;

static char *getcmtstring(CMTS *cmtp)
{
	TABLE *tp ;
	int icmt ;
	char *s ;

	tp= cmtp->tp ;
	if (tp->icol == 0) {
		cmtp->iaflags= IA_LEFT ;
		cmtp->rpad= 2 * cmtp->charwidth ;
	}
	else {
		cmtp->iaflags= IA_LEFT | IAO_WRAP ;
		cmtp->rpad= 0 ;
	}

	if (tp->irow == 0) {
		if (tp->icol == 0) 	s= "Date" ;
		else 			s= "Comment" ;
	}
	else {
		icmt= tp->irow - 1 ;
		if (tp->icol == 0) {
			s= pszTime((tp->plr)->plrcomment[icmt].tTime_a) ;
		}
		else {
			s= pszCommentText(&(tp->plr)->plrcomment[icmt], tp->prd) ;
		}
	}
	return((s != (char *) NULL)? s: (char *) "") ;
}

static BOOL dc_comments(TABLE *tp, int tdf)
{
	static CMTS cmts ;
	CELL *cp ;
	char *s ;
	int i, height ;

	if (tdf == TDF_INIT) {		/* give 1/2 inch margin */
		cmts.tp= tp ;
		cmts.charwidth= (tp->pps)->colHsize ;
		tp->vmgn= tp->pps->loginchVsize / 2 ; /* give 1/2 inch margin */
		return(TRUE) ;
	}

	if (tdf == TDF_FIRSTROW)   return((tp->plr)->ncomments> 0) ;
	if (tdf == TDF_NEXTROW)	   return(tp->irow< (tp->plr)->ncomments) ;
	if (tdf == TDF_NEXTCOL)	   return(tp->icol< 1) ;
	if (tdf == TDF_SETLEFTCOL) return(FALSE) ;

	/*
		returns the size of the formatted item in tp->cell[r][c]'
		also returns as zero-origin rectangle in 'tp->rect'
	*/
	if (tdf == TDF_GETSIZE) {
		SetRectEmpty(&tp->rect) ;
		if (tp->icol> 0) {
			tp->rect.right= (tp->pps)->pgWidth-(2*cmts.charwidth) ;
			height= 0 ;
			for (i= 0; i< tp->icol; i++) {
				tp->rect.right-= tp->cell[tp->irow][i].width ;
				if (height< tp->cell[tp->irow][i].height) {
					height= tp->cell[tp->irow][i].height ;
				}
			}
			tp->rect.bottom= height ;
		}
		s= getcmtstring(&cmts) ;
		if (!sempty(s)) {
			lclDrawText(tp->pps, s, cmts.iaflags, FALSE, &tp->rect) ;
			tp->rect.right+= cmts.rpad ;
		}
		cp= &tp->cell[tp->irow][tp->icol] ;
		cp->width= HSIZER(tp->rect) ;
		cp->height= VSIZER(tp->rect) ;
		return(TRUE) ;
	}
        
        if (tdf == TDF_LAYOUT) return(std_dclayout(tp)) ;

	/*
		assumes that 'tp->rect' contains rectangle,
		to be used for drawing.
	*/
	if (tdf == TDF_DRAW) {
		if (!IsRectEmpty(&tp->rect)) {
			s= getcmtstring(&cmts) ;
			lclDrawText(tp->pps, s, cmts.iaflags, TRUE, &tp->rect) ;
		}
		return(TRUE) ;
	}

	if (tdf == TDF_ENOUGHCOLS) {
		return((BOOL) ((tp->rcol - tp->lcol)> 1)) ;
	}

	return(TRUE) ;
}

static BOOL drawtable(fndrawcell fndc, PRPT prpt)
{
	static TABLE tbl ;
	int rowheight ;
	int pos ;
	
	tbl.plr= prpt->plr ;
	tbl.prd= prpt->prd ;
	tbl.pps= &prpt->ps ;
	tbl.lcol= 0 ;
	tbl.bOverflow= FALSE ;
	tbl.vmgn= 0 ;
	tbl.tNow= time(NULL) ;

	if (!fndc(&tbl, TDF_INIT)) {
		return(FALSE) ;
	}
	if (!fndc(&tbl, TDF_FIRSTCOL)) {
		return(FALSE) ;
	}

	while(iAbortFlags == 0) {

		/*
			get sizes of all cells,
			initialize "row[]" and "col[]"
		*/
		if (!fndc(&tbl, TDF_FIRSTROW)) break ;
		for (tbl.irow= 0; tbl.irow< MAXROWS; tbl.irow++) {
			if (!fndc(&tbl, TDF_LEFTCOL)) break ;		
			for (tbl.icol= tbl.lcol; tbl.icol< MAXCOLS; tbl.icol++) {
				if (!fndc(&tbl, TDF_GETSIZE)) break ;
				if (!fndc(&tbl, TDF_NEXTCOL)) break ;
			}
			if (tbl.icol>= MAXCOLS) tbl.bOverflow= TRUE ;
			if (!fndc(&tbl, TDF_NEXTROW)) break ;
		}
		if (tbl.irow>= MAXROWS) tbl.bOverflow= TRUE ;
		
		/*
			set all column widths to 0
		*/
		if (!fndc(&tbl, TDF_LEFTCOL)) break ;		
		for (tbl.icol= tbl.lcol; tbl.icol< MAXCOLS; tbl.icol++) {
			tbl.col[tbl.icol].width= 0 ;
			if (!fndc(&tbl, TDF_NEXTCOL)) break ;
		}
		
		/*
			calculate the widths of all of the columns.
		*/
		if (!fndc(&tbl, TDF_FIRSTROW)) break ;
		for (tbl.irow= 0; tbl.irow< MAXROWS; tbl.irow++) {
			if (!fndc(&tbl, TDF_LEFTCOL)) break ;
			for (tbl.icol= tbl.lcol; tbl.icol< MAXCOLS; tbl.icol++) {
				if (!fndc(&tbl, TDF_COLHASDATA)) {
					if (!fndc(&tbl, TDF_NEXTCOL)) break ;
					continue ;
				}
				if (
					tbl.col[tbl.icol].width
				      < tbl.cell[tbl.irow][tbl.icol].width
				) {
					tbl.col[tbl.icol].width=
						tbl.cell[tbl.irow][tbl.icol].width ;
				}
				if (!fndc(&tbl, TDF_NEXTCOL)) break ;
			}
			if (!fndc(&tbl, TDF_NEXTROW)) break ;
		}
		/*
			set column positions
		*/
		pos= 0 ;
		if (!fndc(&tbl, TDF_LEFTCOL)) break ;
		for (tbl.icol= tbl.lcol; tbl.icol< MAXCOLS; tbl.icol++) {
			tbl.col[tbl.icol].pos= pos ;
			pos+= tbl.col[tbl.icol].width ;
			if (!fndc(&tbl, TDF_NEXTCOL)) break ;
		}

		/*
			calculate rightmost column that fits
		*/
		tbl.rcol= -1 ;
		if (!fndc(&tbl, TDF_FIRSTROW)) break ;
		for (tbl.irow= 0; tbl.irow< MAXROWS; tbl.irow++) {
			if (!fndc(&tbl, TDF_LEFTCOL)) break ;
			for (tbl.icol= tbl.lcol; tbl.icol< MAXCOLS; tbl.icol++) {
				if (
					(
						tbl.col[tbl.icol].pos
					      + tbl.col[tbl.icol].width
					 )>= tbl.pps->pgWidth
				) {
					break ;
				}
				if (!fndc(&tbl, TDF_NEXTCOL)) {
					tbl.icol++ ;  /* col DID fit! */
					break ;
				}
			}
			if (tbl.rcol< 0) tbl.rcol= tbl.icol ;
			else if (tbl.icol< tbl.rcol) tbl.rcol= tbl.icol ;
			if (!fndc(&tbl, TDF_NEXTROW)) break ;
		}
			/* tbl.rcol is first column that won't fit */
		
		if (
			(tbl.lcol>= tbl.rcol)
		     ||	!fndc(&tbl, TDF_ENOUGHCOLS)
		) {
//			lclErrorMsg(prpt->hWndParent, "page too narrow!", 0) ;
#if	DEBUG
			fprinf(fpdbg, "drawtable: page too narrow!\n") ;
			MessageBeep(0) ;
#endif
			return(FALSE) ;			/* no column fits */
		}

		/*
			draw table
		*/
		/*
			advance the paper to give vertical
			for the table about to be printed.
		*/
		lclAdvancePage(tbl.pps, tbl.vmgn, TRUE) ;

		if (!fndc(&tbl, TDF_FIRSTROW)) break ;
		for (tbl.irow= 0; tbl.irow< MAXROWS; tbl.irow++) {
			/*
				give the interface a chance to say:
				"don't print this row -- it has no data"
			*/
			if (!fndc(&tbl, TDF_ROWHASDATA)) {
				if (!fndc(&tbl, TDF_NEXTROW)) break ;
				continue ;
			}

			/*
				get vertical height of row
			*/
			rowheight= 0 ;
			for (tbl.icol=tbl.lcol; tbl.icol<tbl.rcol; tbl.icol++) {
				if (
					rowheight	
				      <	tbl.cell[tbl.irow][tbl.icol].height
				) {
					rowheight= tbl.cell[tbl.irow]
							   [tbl.icol]
							   .height
					;
				}
			}
			
			/*
				if row is empty, skip it
			*/
			if (rowheight<= 0) {
				if (!fndc(&tbl, TDF_NEXTROW)) break ;
				continue ;
			}

			/*
				prepare page with enough
				paper on it for the row.
			*/
			lclAdvancePage(tbl.pps, rowheight, FALSE) ;

			/*
				draw the row
			*/
			if (!fndc(&tbl, TDF_LEFTCOL)) break ;
			for (tbl.icol=tbl.lcol; tbl.icol<tbl.rcol; tbl.icol++) {
				if (fndc(&tbl, TDF_COLHASDATA)) {
					fndc(&tbl, TDF_LAYOUT) ;
					fndc(&tbl, TDF_DRAW) ;
				}
				fndc(&tbl, TDF_NEXTCOL) ;
			}
			if (iAbortFlags != 0) break ;
			
			/*
				advance the paper past the end
				of the drawn row.
			*/
			lclAdvancePage(tbl.pps, rowheight, TRUE) ;
			if (iAbortFlags != 0) break ;

			if (!fndc(&tbl, TDF_NEXTROW)) break ;
		}
		if (!fndc(&tbl, TDF_SETLEFTCOL)) break ;
	}

	if (iAbortFlags != 0) return(FALSE) ;

	return(TRUE) ;
}

static BOOL lclPrintTables(PRPT prpt)
{
	return(drawtable(dc_project, prpt)) ;
}

static BOOL lclPrintHeader(PRPT prpt)
{
	return(drawtable(dc_header, prpt)) ;
}

static BOOL lclPrintComments(PRPT prpt)
{
	return(drawtable(dc_comments, prpt)) ;
}

static BOOL lclPrintStart(PRPT prpt)
{
	lclFirstPage(&prpt->ps) ;
	return(TRUE) ;
}

static BOOL lclPrintEject(PRPT prpt)
{
	lclNextPage(&prpt->ps, FALSE) ;
	return(TRUE) ;
}

/* end of tlrptdsp.c */
