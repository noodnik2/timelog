/*	Revision:	5
*/

#define		DEBUG  		0

/*

	timerpt.h

*/

#ifndef	TIMERPT_H_INC
#define	TIMERPT_H_INC

#if	DEBUG
#include	<stdio.h>
#endif

#include	"logfile.h"
#include	"logrpt.h"

#define	MAXSIZE_TITLETEXT	128	/* max size of title text	*/
#define	MAXSIZE_FILENAME	128	/* max size of system file name	*/
#define	MAXSIZE_TIMESCOPE	32	/* max size of printed time scope */
#define	MAXSIZE_SYSCMD		128	/* max size of system command	*/

#define	TDF_HFRACT		0	/* time display formats		*/
#define	TDF_HHMM		1

#define	TS_DAY	       		0	/* time scope categories	*/
#define	TS_MONTH		1
#define	TS_YEAR			2

typedef struct tagTIMERPT {
	LOGRPT *plr ;
//	BOOL (*fnrptprint)(HWND, LOGRPT *, RPTPRINT *) ;
} TIMERPT ;

extern HINSTANCE hInst ;

BOOL timerpt_ErrorMsg(HWND, LPSTR, int) ;

#if	DEBUG
extern FILE *fpdbg ;
#endif

/* #ifndef TIMERPT_H_INC */
#endif

/* end of timerpt.h */
