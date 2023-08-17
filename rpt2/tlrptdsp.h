/*	Revision:	12
*/

/*

	tlrptdsp.h

*/

#ifndef	TLRPTDSP_H_INC
#define	TLRPTDSP_H_INC

#include	<commdlg.h>

#include	"timerpt.h"
#include	"logrpt.h"
#include	"project.h"

#define	UM_UPDATESCROLL		(WM_USER+1)
#define	UM_TLRPTDSP_USER	(WM_USER+10)	/* this & above we don't use */

#define	RPE_SUCCESS		0
#define	RPE_CANCELED		1
#define	RPE_CANTCREATE		2
#define	RPE_ABORTED		3
#define	RPE_FAILURE		4

typedef void *HRPT ;				/* handle to rpt disp  */

typedef	struct tagTLRPTDSP {
	int iTimeDisplayFormat ;	/* time display format 		*/
	BOOL bSparse ;			/* sparse report table 		*/
//	BOOL bComments ;		/* include comments    		*/
	LOGFONT lfd ;			/* display logical font       	*/
	LOGFONT lfp ;			/* printer logical font       	*/
} TLRPTDSP ;

BOOL tlrptdsp_Init(void) ;
void tlrptdsp_Fini(void) ;
int tlrptdsp_errno(void) ;
HDC tlrptdsp_CreateDC_FromPPD(PRINTDLG *ppd, BOOL bCreateIC) ;
BOOL tlrptdsp_Print(HWND, LOGRPT *, TLRPTDSP *, PRINTDLG *) ;
HRPT tlrptdsp_CreateLink(HWND, LOGRPT *, TLRPTDSP *, PRINTDLG *) ;
BOOL tlrptdsp_DestroyLink(HRPT hRpt, HWND hWnd) ;
BOOL tlrptdsp_Link(HRPT, HWND, UINT, WPARAM, LPARAM) ;

/* #ifndef TLRPTDSP_H_INC */
#endif

/* end of rptprint.h */
