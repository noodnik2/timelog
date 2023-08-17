/*	Revision:	12
*/

/*

	timeutil.h

*/

#ifndef	TIMEUTIL_H_INC
#define	TIMEUTIL_H_INC

#include	<time.h>

#define	TUT_MINUTE		0x0001
#define	TUT_HOUR       		0x0002
#define	TUT_DAY	       		0x0003
#define	TUT_MONTH      		0x0004
#define	TUT_YEAR		0x0005
#define	TUT_STARTOFMINUTE	0x0011
#define	TUT_STARTOFHOUR		0x0012
#define	TUT_STARTOFDAY		0x0013
#define	TUT_STARTOFMONTH	0x0014
#define	TUT_STARTOFYEAR		0x0015
#define	TUT_FOREVER		0x7FFF

#define	TUL_YEAR		0
#define	TUL_MONTH		1
#define	TUL_MDAY		2
#define	TUL_MWDAY		3
#define	TUL_WDAY		4
#define	TUL_HOUR		5
#define	TUL_MINUTE		6

#define	TUN_YEARS		0
#define	TUN_MONTHS		1
#define	TUN_DAYS		2

#define	TUMAXSIZE_PERIODNAME	16	/* max size of "period name" */

int timeutil_nPeriods(time_t t0, time_t t1, int tun_id) ;
int timeutil_szLoad(time_t t0, int tul_id, char *pszPeriodName) ;
time_t timeutil_tTime(time_t t0, int tut_id, int nOffset) ;

/* #ifndef TIMEUTIL_H_INC */
#endif

/* end of timeutil.h */
