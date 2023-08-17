/*	Revision:	13
*/

/******************************************************************/
#include	<windows.h>
#include	<commdlg.h>
#include	<memory.h>
/******************************************************************/

static char *pszAppName= "PTst" ;
static char *pszAppClass= "ClsPTst" ;

static HINSTANCE hInst ;
static HINSTANCE hWndTop ;
static BOOL bAbort ;

/******************************************************************/

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM) ;
int CALLBACK _export AbortProc(HDC hPrtDC, int fCode) ;
int CALLBACK _export AbortDlg(
	HWND hDlg,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
) ;
static BOOL lclInitApplication(HINSTANCE, HINSTANCE) ;
static BOOL lclInitInstance(HINSTANCE, LPSTR, int) ;
static BOOL lclInitError(char *) ;
static BOOL lclGetMessageProc(WPARAM *pexitcode) ;

static void ptst(HWND hWnd) ;
static BOOL printit(HWND hWnd, HDC hDC) ;
/******************************************************************/

int PASCAL WinMain(
	HINSTANCE hInstance,
	HINSTANCE hInstancePrev,
	LPSTR lpszCmdLine,
	int nCmdShow
) {
	WPARAM exitcode ;
	
	if (!lclInitApplication(hInstance, hInstancePrev)) {
		return(FALSE) ;
	}
	if (!lclInitInstance(hInstance, lpszCmdLine, nCmdShow)) {
		return(FALSE) ;
	}
	while(lclGetMessageProc(&exitcode)) ;
	return(exitcode) ;
}


LRESULT CALLBACK WindowProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
) {
	switch(message) {
		case	WM_CREATE:
			ptst(hWnd) ;
			return(-1) ;
			break ;
			
		case	WM_DESTROY:
			PostQuitMessage(0) ;
			return(0) ;
			break ;
	}
	return(DefWindowProc(hWnd, message, wParam, lParam)) ;
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

static BOOL lclInitApplication(
	HINSTANCE hInstance,
	HINSTANCE hInstancePrev
) {
	WNDCLASS wc ;
	
	if (hInstancePrev != (HINSTANCE) NULL) {
		lclInitError("Application already active") ;
		return(FALSE) ;
	}
	
	wc.style		= 0 ;
	wc.lpfnWndProc		= WindowProc ;
	wc.cbClsExtra		= 0 ;
	wc.cbWndExtra		= 0 ;
	wc.hInstance		= hInstance ;
	wc.hIcon		= NULL ;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW) ;
	wc.hbrBackground	= GetStockObject(WHITE_BRUSH) ;
	wc.lpszMenuName		= NULL ;
	wc.lpszClassName	= pszAppClass ;
	
	if (!RegisterClass(&wc)) {
		lclInitError("Can't register class") ;
		return(FALSE) ;
	}
	
	return(TRUE) ;
}

static BOOL lclInitInstance(
	HINSTANCE hInstance,
	LPSTR lpszCmdLine,
	int nCmdShow
) {
	HWND hWnd ;

	hInst= hInstance ;
		
	hWnd= CreateWindow(
		pszAppClass,
		pszAppName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL
	) ;
	if (hWnd == NULL) {
		lclInitError("Can't create window") ;
		return(FALSE) ;
	}
	
	ShowWindow(hWnd, nCmdShow) ;
	UpdateWindow(hWnd) ;

	hWndTop= hWnd ;
		
	return(TRUE) ;
}

static BOOL lclInitError(char *pszErrorMsg)
{
	MessageBox(
		NULL,
		pszErrorMsg,
		pszAppName,
		MB_OK | MB_ICONEXCLAMATION
	) ;
	return(FALSE) ;
}

static HWND hWndAbortDlg ;

static void ptst(HWND hWnd)
{
	PRINTDLG pd ;
	DOCINFO di ;
	FARPROC lpAbortDlg ;
	FARPROC lpAbortProc ;

				/* Set all structure members to zero. */
	memset(&pd, 0, sizeof(PRINTDLG)) ;

		/* Initialize the necessary PRINTDLG structure members. */
	pd.lStructSize= sizeof(PRINTDLG) ;
	pd.hwndOwner= hWnd ;
	pd.Flags= PD_RETURNDC ;

				/* Print a test page if successful */
	if (!PrintDlg(&pd)) {
    		lclInitError("PrintDlg error") ;
    		return ;
	}
	
	if (!pd.hDC) {
		lclInitError("no DC") ;
	}
	else {
		lpAbortDlg= MakeProcInstance(AbortDlg, hInst) ;
		hWndAbortDlg= CreateDialog(
			hInst, "ABORTDLG", hWnd, lpAbortDlg
		) ;
		if (hWndAbortDlg == (HWND) NULL) {
			lclInitError("Can't Create abort dialog") ;
		}
		else {
			bAbort= FALSE ;
			lpAbortProc= MakeProcInstance(AbortProc, hInst) ;
			SetAbortProc(pd.hDC, lpAbortProc) ;
			di.cbSize= sizeof(DOCINFO) ;
			di.lpszDocName= "Test-Doc" ;
			di.lpszOutput= NULL ;
			if (StartDoc(pd.hDC, &di)< 0) {
				lclInitError("StartDoc error") ;
			}
			else {
			    	if (printit(hWnd, pd.hDC)) {
					EndDoc(pd.hDC) ;
				}
			}
			FreeProcInstance(lpAbortProc) ;
			DestroyWindow(hWndAbortDlg) ;
		}
		FreeProcInstance(lpAbortDlg) ;
		DeleteDC(pd.hDC) ;
	}
	if (pd.hDevMode != NULL) {
		GlobalFree(pd.hDevMode) ;
	}
	if (pd.hDevNames != NULL) {
		GlobalFree(pd.hDevNames) ;
	}
}

typedef struct tagPAGESTATE {
	HDC hDC ;
	int pgHeight ;
	int pgWidth ;
	int pgVertPos ;
	int pgLmgnPos ;
	int pgNum ;
} PAGESTATE ;

static void lclInitPage(PAGESTATE *pps, HDC hDC)
{
	pps->hDC= hDC ;
        pps->pgHeight= GetDeviceCaps(hDC, VERTRES) ;
        pps->pgWidth= GetDeviceCaps(hDC, HORZRES) ;
	pps->pgVertPos= 0 ;
	pps->pgLmgnPos= 0 ;
	pps->pgNum= 0 ;
}

static void lclNextPage(PAGESTATE *pps)
{
	Escape(pps->hDC, NEWFRAME, 0, NULL, NULL) ;
	pps->pgNum++ ;
	pps->pgVertPos= 0 ;
}

static void lclAdvancePage(PAGESTATE *pps, int pgVertLen, BOOL bMove)
{
	if ((pps->pgVertPos + pgVertLen)>= (pps->pgHeight)) {
		lclNextPage(pps) ;
		return ;
	}
	if (bMove) {
		(pps->pgVertPos)+= pgVertLen ;
	}
}

#define	IA_LEFT		0
#define	IA_RIGHT	1
#define	IA_CENTER	2

static void lclDrawText(
	PAGESTATE *pps,
	char *pszText,
	int iAlign,
	BOOL bDraw,
	RECT *pr
) {
	UINT fuFormat ;

	fuFormat= DT_NOPREFIX | DT_EXPANDTABS | DT_WORDBREAK ;
	switch(iAlign) {
		case	IA_LEFT:   fuFormat|= DT_LEFT ;		break ;
		case	IA_RIGHT:  fuFormat|= DT_RIGHT ;	break ;
		case	IA_CENTER: fuFormat|= DT_CENTER ;	break ;
	}
	if (!bDraw) fuFormat|= DT_CALCRECT ;
	DrawText(pps->hDC, pszText, -1, pr, fuFormat) ;
}

static void lclNewLineRect(
	const PAGESTATE *pps,
	const RECT* prleft,
	RECT* prright,
	int rColLen
) {
	if (prleft == (const RECT*) NULL) {
		prright->top= pps->pgVertPos ;
		prright->left= pps->pgLmgnPos ;
	}
	else {
		prright->top= prleft->top ;
		prright->left= prleft->right + 1 ;
	}
	prright->bottom= prright->top + 1 ;
	prright->right= prright->left + rColLen - 1 ;
}

#if 0
1.	go across page, formatting into intersection of rectangles
2.	get page with enough space to put it
3.	go across page, formatting onto page
4.	advance vertical current page position
#endif

static BOOL printit(HWND hWnd, HDC hDC)
{
	RECT rleft, rcenter, rright, rtotal ;
	char *s1, *s2 ;
	PAGESTATE ps ;
			       
	s1= "thisisaverystrangestringthatwontfitintothecol" ;
	s2= "This is only a test of the non-emergency printer text formatter" ;

	lclInitPage(&ps, hDC) ;

	lclNewLineRect(&ps, NULL, &rleft, 300) ;
#if 0
	rleft.top= ps.pgVertPos ;
	rleft.bottom= ps.pgVertPos + 1 ;
	rleft.left= 0 ;
	rleft.left + 300 ;
#endif
	lclDrawText(&ps, s1, IA_LEFT, FALSE, &rleft) ;

	lclNewLineRect(&ps, &rleft, &rcenter, 50) ;
	lclNewLineRect(&ps, &rcenter, &rright, 300) ;
#if 0
	rright.top= ps.pgVertPos ;
	rright.bottom= ps.pgVertPos + 1 ;
	rright.left= rleft.right + 50 ;
	rright.right= rright.left + 300 ;
#endif
	lclDrawText(&ps, s2, IA_LEFT, FALSE, &rright) ;

	UnionRect(&rtotal, &rleft, &rright) ;
	lclAdvancePage(&ps, rtotal.bottom - rtotal.top + 1, FALSE) ;

	lclDrawText(&ps, s1, IA_LEFT, TRUE, &rleft) ;
	lclDrawText(&ps, s2, IA_LEFT, TRUE, &rright) ;
	lclAdvancePage(&ps, rtotal.bottom - rtotal.top + 1, TRUE) ;

	lclNextPage(&ps) ;

	return(TRUE) ;
}

int CALLBACK _export AbortProc(HDC hPrtDC, int fCode)
{
	MSG msg ;
	
	while(!bAbort && PeekMessage(&msg, NULL, NULL, NULL, TRUE)) {
		if (!IsDialogMessage(hWndAbortDlg, &msg)) {
			TranslateMessage(&msg) ;
			DispatchMessage(&msg) ;
		}
	}
	return(!bAbort) ;
}

int CALLBACK _export AbortDlg(
	HWND hDlg,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
) {
	switch(msg) {
		case	WM_COMMAND:
			return(bAbort= TRUE) ;
			break ;
		case	WM_INITDIALOG:
			return(TRUE) ;
			break ;
	}
	return(FALSE) ;
}

/******************************************************************/
