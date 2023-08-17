/*	Revision:	12
*/

/*

	timelog.h

*/

#ifndef	TIMELOG_H_INC
#define	TIMELOG_H_INC

#define	MAXSIZE_TITLETEXT	128	/* max size of title text	*/
#define	MAXSIZE_FILENAME	128	/* max size of system file name	*/
#define	MAXSIZE_TIMESCOPE	32	/* max size of printed time scope */
#define	MAXSIZE_SYSCMD		128	/* max size of system command	*/

#define	MAX_INIPROJECTS		32	/* max. # of projects ini .INI	*/

					/* timelog window user messages	*/
#define	UM_DODIALOG		(WM_USER+1)
#define	UM_RECALC		(WM_USER+2)
#define UM_HELP			(WM_USER+3)
#define	 UMRCWPF_REPAINT	1	/* "wParam" flags for UM_RECALC */
#define	 UMRCWPF_REPERIOD	2

#define	TDF_HFRACT		0	/* time display formats		*/
#define	TDF_HHMM		1

#define	TS_DAY	       		0	/* time scope categories	*/
#define	TS_MONTH		1
#define	TS_YEAR			2

struct wincrparms {
	int nCmdShow ;
	LPSTR lpszCmdLine ;
} ;

/* #ifndef	TIMELOG_H_INC */
#endif

/* end of timelog.h */
