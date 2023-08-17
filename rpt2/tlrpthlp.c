#define	Revision	4

/*

	tlrpthlp.c


*/

#include	"wininc.h"
#include	"tlrpthlp.h"

extern HINSTANCE hInst ;

static TRHELPREF hrCurrent ;
static HWND hWndHelp ;
static UINT uiHelpMsg ;
static HOOKPROC lpfnHelpKeyHook ;
static HHOOK hhHelpKeyHook ;

LRESULT _export CALLBACK tlrpthlp_HelpKeyHook(int code, WPARAM wParam, LPARAM lParam)
{
	if ((code == MSGF_DIALOGBOX) || (code == MSGF_MENU)) {
		LPMSG lpmsg= (LPMSG) lParam ;
		if (lpmsg->message == WM_KEYDOWN) {
			if (lpmsg->wParam == VK_F1) {
				if (
					(hWndHelp != (HWND) NULL)
				     &&	(hrCurrent != TRHELPREF_NONE)
				) {
					PostMessage(
						hWndHelp,
						uiHelpMsg,
						(WPARAM) code,
						(LPARAM) tlrpthlp_GetHelpRef()
					) ;
					return(1) ;
				}
			}
		}
	}
	CallNextHookEx(hhHelpKeyHook, code, wParam, lParam) ;
	return(0) ;
}
	
BOOL tlrpthlp_Init()
{
	hWndHelp= (HWND) NULL ;
	hrCurrent= TRHELPREF_NONE ;
	uiHelpMsg= 0 ;
	
	lpfnHelpKeyHook= (HOOKPROC) MakeProcInstance(
		(FARPROC) tlrpthlp_HelpKeyHook,
		hInst
	) ;
	if (lpfnHelpKeyHook == (HOOKPROC) NULL) {
		return(FALSE) ;
	}
	hhHelpKeyHook= SetWindowsHookEx(
		WH_MSGFILTER,
		lpfnHelpKeyHook,
		hInst,
		GetCurrentTask()
	) ;
}

void tlrpthlp_Fini()
{
	if (lpfnHelpKeyHook == (HOOKPROC) NULL) return ;
	UnhookWindowsHookEx(hhHelpKeyHook) ;
	FreeProcInstance((FARPROC) lpfnHelpKeyHook) ;
	lpfnHelpKeyHook= (HOOKPROC) NULL ;
}

void tlrpthlp_SetHelpMsg(HWND hWnd, UINT helpmsg)
{
	hWndHelp= hWnd ;
	uiHelpMsg= helpmsg ;
}

TRHELPREF tlrpthlp_SetHelpRef(TRHELPREF hr)
{
	TRHELPREF hrOld ;

	hrOld= hrCurrent ;
	hrCurrent= hr ;
	return(hrOld) ;
}

TRHELPREF tlrpthlp_GetHelpRef()
{
	return(hrCurrent) ;
}

/* end of tlrpthlp.c */
