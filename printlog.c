/*	Revision:	13
*/

/*

	printlog.c

*/

#include	<stdio.h>
#include	<time.h>

#include	"printlog.h"

static void printhdr(FILE *fpout, char *logfn, LOGHDR *lhp) ;
static void printrec(FILE *fpout, LOGREC *lrp) ;

int printlog(char *logfn, char *tofn)
{
	LOGHDR lh ;
	LOGREC lr ;
	FILE *fpout ;

	fpout= fopen(tofn, "w") ;
	if (fpout == (FILE *) NULL) {
		return(1) ;
	}

	logfile_Init(logfn) ;
	if (logfile_OpenForRead(&lh)) {
		fclose(fpout) ;
		return(2) ;
	}
	
	printhdr(fpout, logfn, &lh) ;

	while(logfile_GetRec(&lr) == 0) {
		printrec(fpout, &lr) ;
	}

	logfile_Close() ;
	logfile_Fini() ;

	fclose(fpout) ;
}

/*
	start of internal routines
*/

static void printhdr(FILE *fpout, char *logfn, LOGHDR *lhp)
{
	fprintf(
		fpout,
		"%s: version=%d compatibility=%d\n",
		logfn,
		lhp->ver,
		lhp->compatibility
	) ;
}

static char *pszaAction[]= {	"START",   "STOP",   "COMMENT",  "ADJUST"  } ;
static char *pszaCategory[]= {	"SESSION", "PROJECT" } ;

static void printrec(FILE *fpout, LOGREC *lrp)
{
	time_t tTime ;
	time_t tTime_a ;

	tTime= (time_t) lrp->ltime ;
	tTime_a= (time_t) lrp->ltime_a ;
	fprintf(fpout, "%.24s ", asctime(localtime(&tTime))) ;
	fprintf(fpout, "%.24s ", asctime(localtime(&tTime_a))) ;
	fprintf(
		fpout,
		"%-10ld %-9s %-11s",
		lrp->lval,
		pszaAction[lrp->iaction],
		pszaCategory[lrp->icategory]
	) ;
	if (lrp->szEname[0] != '\0') {
		fprintf(fpout, " \"%s\"", lrp->szEname) ;
	}
	fprintf(fpout, "\n") ;
}

/* end of printlog.c */
