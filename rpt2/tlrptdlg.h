/*	Revision:	19
*/

/*

	tlrptdlg.h			TimeRpt Dialog Definitions

*/

#ifndef	TLRPTDLG_H_INC
#define	TLRPTDLG_H_INC

#include	<commdlg.h>

#include	"timerpt.h"
#include	"tlrptdsp.h"
#include	"tlrpthlp.h"

#define	MAXSIZE_EDITNUMBER	16	/* max size of editable numbers	*/
#define	MAXSIZE_USERNAME	64	/* max size of user name	*/

typedef struct tagDLGRPT {
	BOOL bUpdate ;			/* dialog info was updated	*/
	PRINTDLG pd ;			/* printer dialog setup info    */
	TLRPTDSP tlrptdsp ;		/* report display options	*/
	LOGRPT *plr ;			/* handle to current report     */
	HRPT hRpt ;			/* handle to rept display struc */
	time_t tRptRef ;		/* report cut-off time refer	*/
	int nPeriods ;			/* number of periods backwards	*/
	int iTimeDisplayFormat ;	/* report time-display format	*/
	int iTimeScope ;		/* report time-scope		*/
	int iEntryType ;		/* report entry type		*/
	BOOL bTotals ;			/* include totals in report	*/
	BOOL bAlloc ;			/* include allocations in rpt	*/
	BOOL bAdjust ;			/* include adjustments in rpt	*/
	BOOL bComments ;		/* include comments in report	*/
	char szLogFile[MAXSIZE_LOGFILENAME+1] ;	/* log file		*/
	char szHelpFile[MAXSIZE_HELPFILENAME+1] ; /* help file		*/
	char szUserName[MAXSIZE_USERNAME+1] ;	/* user name		*/
} DLGRPT, FAR *LPDLGRPT ;

BOOL tlrptdlg_InvokeDialog(HWND hWndParent, LPDLGRPT lpdlgrpt) ;

/* #ifndef	TLRPTDLG_H_INC */
#endif

/* end of tlrptdlg.h */
