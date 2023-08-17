#define	Revision	12

#define		DEBUG		0

/*

	scanlog.c

*/

#include	"wininc.h"

#include	<memory.h>
#include	<stdio.h>
#include	<time.h>

#include	"logfile.h"

#if	DEBUG
static FILE *fplog ;
#endif

static time_t tEntryTime(char *) ;

static long lPeriod(LONG, LONG, LONG, LONG) ;
static long lPeriodAdjust(LONG ltTime, long lAdjust, LONG ltFrom, LONG ltTo) ;

LONG scanlog_lTotalTime(
	time_t tFrom,
	time_t tTo,
	int icategory,
	char *pszMatchEname
) {
	LONG lTotal, ltStart ;
	LOGREC lr ;

	lTotal= (LONG) 0 ;

	if (logfile_OpenForRead(NULL)) {
		return(lTotal) ;
	}
		
#if	DEBUG
	fplog= fopen("scanlog.log", "w") ;
#endif

	/*
		Scan entire log file
	*/
	while(logfile_GetRec(&lr) == 0) {

		/*
			Skip records that are of no interest
		*/
		if (lr.icategory != icategory) continue ;
		if (pszMatchEname != (char *) NULL) {
			if (lstrcmp(pszMatchEname, lr.szEname) != 0) continue ;
		}

		/*
			Assume perfect matching of "start/stop" action pairs
		*/
		switch(lr.iaction) {

			case	LRA_START:
				ltStart= lr.ltime_a ;
				break ;

			case	LRA_STOP:
				lTotal+= lPeriod(
					ltStart,
					lr.ltime_a,
					(LONG) tFrom,
					(LONG) tTo
				) ;
				break ;

			case	LRA_ADJUSTMENT:
				lTotal+= lPeriodAdjust(
					lr.ltime_a,
					lr.lval,
					(LONG) tFrom,
					(LONG) tTo
				) ;
				break ;

		}

	}
	logfile_Close() ;

#if	DEBUG
	fclose(fplog) ;
#endif

	return(lTotal) ;
}

/*

	start of internal subroutines
	
*/

static long lPeriod(LONG ltStart, LONG ltStop, LONG ltFrom, LONG ltTo)
{
	LONG lValue ;

#if	DEBUG
	fprintf(fplog, "start=%ld stop=%ld from=%ld to=%ld\n", 
		ltStart, ltStop, ltFrom, ltTo) ;
#endif

	lValue= ltStop - ltStart ;
	if (ltFrom> ltStart) lValue-= (ltFrom - ltStart) ;
	if (ltTo< ltStop) lValue-= (ltStop - ltTo) ;

#if	DEBUG
	fprintf(fplog, "  ==> lValue:%ld\n", lValue) ;
#endif

	if (lValue< 0) lValue= 0 ;

	return(lValue) ;
}

static long lPeriodAdjust(LONG ltTime, long lAdjust, LONG ltFrom, LONG ltTo)
{
	if ((ltTime< ltFrom) || (ltTime> ltTo)) return(0L) ;
	return(lAdjust) ;
}

/* end of scanlog.c */
