/*	Revision:	17
*/

/*

	timeutil.c

*/

#include	<stdio.h>
#include	<string.h>
#include	<time.h>

#include	"timeutil.h"

int timeutil_nPeriods(time_t t0, time_t t1, int tun_id)
{
	time_t tNext ;
	int nPeriods ;

	tNext= t0 ;
	switch(tun_id)
	{
		case	TUN_DAYS:
			for (nPeriods= 0; tNext< t1; nPeriods++) {
				tNext= timeutil_tTime(
					tNext,
					TUT_STARTOFDAY,
					1
				) ;
			}
			break ;

		case	TUN_MONTHS:
			for (nPeriods= 0; tNext< t1; nPeriods++) {
				tNext= timeutil_tTime(
					tNext,
					TUT_STARTOFMONTH,
					1
				) ;
			}
			break ;

		case	TUN_YEARS:
			for (nPeriods= 0; tNext< t1; nPeriods++) {
				tNext= timeutil_tTime(
					tNext,
					TUT_STARTOFYEAR,
					1
				) ;
			}
			break ;

		default:
			return(-1) ;
			break ;
	}

	return(nPeriods) ;
}

time_t timeutil_tTime(time_t t0, int tut_id, int nOffset)
{
	time_t t1 ;
	struct tm tm ;

	switch(tut_id) {

		/*
			"start of"
		*/
		case	TUT_STARTOFMINUTE:
			tm= *localtime(&t0) ;
			tm.tm_sec= 0 ;			/* start of min  */
			tm.tm_min+= nOffset ;		/* offset minute */
			t1= mktime(&tm) ;
			break ;

		case	TUT_STARTOFHOUR:
			tm= *localtime(&t0) ;
			tm.tm_sec= tm.tm_min= 0 ;	/* start of hour */
			tm.tm_hour+= nOffset ;		/* offset hour   */
			t1= mktime(&tm) ;
			break ;

		case	TUT_STARTOFDAY:
			tm= *localtime(&t0) ;
			tm.tm_sec= tm.tm_min= 
				tm.tm_hour= 0 ;		/* start of day */
			tm.tm_mday+= nOffset ;		/* offset day */
			t1= mktime(&tm) ;
			break ;

		case	TUT_STARTOFMONTH:
			tm= *localtime(&t0) ;
			tm.tm_sec= tm.tm_min= 
				tm.tm_hour= 0 ;		/* start of day */
			tm.tm_mday= 1 ;			/* first of month */
			tm.tm_mon+= nOffset ;		/* offset month */
			t1= mktime(&tm) ;
			break ;

		case	TUT_STARTOFYEAR:
			tm= *localtime(&t0) ;
			tm.tm_sec= tm.tm_min= 
				tm.tm_hour= 0 ;		/* start of day */
			tm.tm_mday= 1 ;			/* first of month */
			tm.tm_mon= 0 ;			/* Jan 1 */
			tm.tm_year+= nOffset ;		/* offset year */
			t1= mktime(&tm) ;
			break ;

		/*
			"period only"
		*/
		case	TUT_MINUTE:
			tm= *localtime(&t0) ;
			tm.tm_min+= nOffset ;		/* offset minute */
			t1= mktime(&tm) ;
			break ;

		case	TUT_HOUR:
			tm= *localtime(&t0) ;
			tm.tm_hour+= nOffset ;		/* offset hour   */
			t1= mktime(&tm) ;
			break ;

		case	TUT_DAY:
			tm= *localtime(&t0) ;
			tm.tm_mday+= nOffset ;		/* offset day */
			t1= mktime(&tm) ;
			break ;

		case	TUT_MONTH:
			tm= *localtime(&t0) ;
			tm.tm_mon+= nOffset ;		/* offset month */
			t1= mktime(&tm) ;
			break ;

		case	TUT_YEAR:
			tm= *localtime(&t0) ;
			tm.tm_year+= nOffset ;		/* offset year */
			t1= mktime(&tm) ;
			break ;

		/*
			"forever" is defined here as:
				"Midnight, Feb 5, 2036"
			(see help for "mktime")
		*/
		case	TUT_FOREVER:
			tm.tm_mon= 1 ;			/* Feb */
			tm.tm_mday= 5	 ;		/* 5 */
			tm.tm_hour= 23 ;		/* 11:hh:ss */
			tm.tm_sec= tm.tm_min= 59 ;	/* 11:59:59 */
			tm.tm_year= (2036 - 1900) ;	/* 2036 */
			t1= mktime(&tm) ;
			break ;

		default:
			t1= (time_t) -1 ;
			break ;
	}

	return(t1) ;
}

static char *aszWeekDays[]= {
	"Sun",			/*	0	*/
	"Mon",			/*	1	*/
	"Tue",			/*	2	*/
	"Wed",			/*	3	*/
	"Thu",			/*	4	*/
	"Fri",			/*	5	*/
	"Sat"			/*	6	*/
} ;

static char *aszYearMonths[]= {
	"Jan",			/*	0	*/
	"Feb",			/*	1	*/
	"Mar",			/*	2	*/
	"Apr",			/*	3	*/
	"May",			/*	4	*/
	"Jun",		       	/*	5	*/
	"Jul",		       	/*	6	*/
	"Aug",	       		/*	7	*/
	"Sep",			/*	8	*/
	"Oct",			/*	9	*/
	"Nov",			/*	10	*/
	"Dec"			/*	11	*/
} ;

int timeutil_szLoad(time_t t0, int tul_id, char *pszPeriodName)
{
	struct tm tm ;

	switch(tul_id)
	{
		case	TUL_MINUTE:
			tm= *localtime(&t0) ;
			sprintf(pszPeriodName, "%u", tm.tm_min) ;
			break ;

		case	TUL_HOUR:
			tm= *localtime(&t0) ;
			sprintf(pszPeriodName, "%u", tm.tm_hour) ;
			break ;

		case	TUL_MDAY:
			tm= *localtime(&t0) ;
			sprintf(pszPeriodName, "%u", tm.tm_mday) ;
			break ;

		case	TUL_MWDAY:
			tm= *localtime(&t0) ;
			sprintf(
				pszPeriodName,
				"%s%u",
				aszWeekDays[tm.tm_wday],
				tm.tm_mday
			) ;
			break ;

		case	TUL_WDAY:
			tm= *localtime(&t0) ;
			strcpy(pszPeriodName, aszWeekDays[tm.tm_wday]) ;
			break ;

		case	TUL_MONTH:
			tm= *localtime(&t0) ;
			strcpy(pszPeriodName, aszYearMonths[tm.tm_mon]) ;
			break ;

		case	TUL_YEAR:
			tm= *localtime(&t0) ;
			sprintf(pszPeriodName, "%u", 1900 + tm.tm_year) ;
			break ;

		default:
			return(-1) ;
			break ;
	}

	return(0) ;
}

/* end of timeutil.c */
