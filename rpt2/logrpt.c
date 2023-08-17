#define	Revision	62

#define		DEBUG		0

/*

	logrpt.c

*/

#include	"wininc.h"

#include	<malloc.h>
#include	<memory.h>
#include	<string.h>
#include	<stdio.h>
#include	<time.h>

#include	"logrpt.h"
#include	"logfile.h"
#include	"project.h"

static int lrAddComment(
	LOGRPT *plr,
	int ctype,
	char *pszText,
	time_t tTime,
	time_t tTime_a,
	long lval
) ;

static int lrMakeEntryName(char *szEntry, char *szEname, int iEntryType) ;
static int lrAccumEntry(LOGRPT *plr, char *pszEntry, long *plTime) ;
static int lrFindEntry(LOGRPT *plr, char *pszEntry) ;
static int lrAddEntry(LOGRPT *plr, char *pszEntry) ;
static int lrCalcTotals(LOGRPT *plr) ;
static long lPeriod(time_t ltStart, time_t ltStop, time_t ltFrom, time_t ltTo) ;
static BOOL bPeriodTest(time_t ltTime, time_t ltFrom, time_t ltTo) ;

static int lr_errno= LRE_SUCCESS ;

static char szTotalLiteral[]= "Total" ;

#if	DEBUG
static FILE *fplog= (FILE *) NULL ;
#endif

int logrpt_Init()
{
#if	DEBUG
	fplog= fopen("logrpt.log", "w") ;
#endif
	return(0) ;
}

void logrpt_Fini()
{
#if	DEBUG
	if (fplog != (FILE *) NULL) {
		fclose(fplog) ;
		fplog= (FILE *) NULL ;
	}
#endif	
}

LOGRPT *logrpt_Create(LOGRPTDEF *plrd)
{
	LOGREC lr ;
	LOGRPT *plr ;
	time_t tStart ;
	time_t lTimeStart ;
	int tut_id ;
	int tul_loadid ;
	int tun_id ;
	int i ;
	int nperiods ;
	BOOL bSave, bError ;
	BOOL bTotal, bAlloc, bAdjust, bComments ;
	long *plTime ;			/* -> [nperiods] */
	long lAccum ;
	char szEntry[MAXSIZE_LOGENAME+1] ;
	char szEname[LRMAXSIZE_ENAME+1] ;
	char *pszlrtypeName ;

	lr_errno= LRE_SUCCESS ;

	/*
		load variables according to 'lrtype',
		check 'lrtype' while we're at it.
	*/
	switch(plrd->lrtype) {

		case	LRT_MONTHDAYS:
			tut_id=		TUT_STARTOFDAY ;
			tul_loadid=	TUL_MWDAY ;
			tun_id=		TUN_DAYS ;
			pszlrtypeName=	"Daily" ;
			break ;

		case	LRT_YEARMONTHS:
			tut_id=		TUT_STARTOFMONTH ;
			tul_loadid=	TUL_MONTH ;
			tun_id=		TUN_MONTHS ;
			pszlrtypeName=	"Monthly" ;
			break ;

		case	LRT_YEARS:
			tut_id=		TUT_STARTOFYEAR ;
			tul_loadid=	TUL_YEAR ;
			tun_id=		TUN_YEARS ;
			pszlrtypeName=	"Yearly" ;
			break ;

		default:
			lr_errno= LRE_BADPARAM ;
			return((LOGRPT *) NULL) ;
			break ;

	}
	nperiods= timeutil_nPeriods(plrd->t0, plrd->t1, tun_id) ;
#if	DEBUG
	fprintf(fplog, "nperiods= %d\n", nperiods) ;
#endif
	if (plrd->lrOptions & LROPT_TOTAL) nperiods++ ;

	/*
		allocate and initialize main "LOGRPT" structure.
	*/
	plr= (LOGRPT *) malloc(sizeof(LOGRPT)) ;
	if (plr == (LOGRPT *) NULL) {
		lr_errno= LRE_NOMEMORY ;
		return((LOGRPT *) NULL) ;
	}
	memset(plr, '\0', sizeof(LOGRPT)) ;
	lstrcpy(plr->szName, pszlrtypeName) ;
	lstrcpy(plr->szUser, plrd->szUser) ;

	bTotal= (BOOL) ((plrd->lrOptions & LROPT_TOTAL) != 0) ;
	bAlloc= (BOOL) ((plrd->lrOptions & LROPT_ALLOC) != 0) ;
	bAdjust= (BOOL) ((plrd->lrOptions & LROPT_ADJUST) != 0) ;
	bComments= (BOOL) ((plrd->lrOptions & LROPT_COMMENTS) != 0) ;
	if (bTotal) lstrcat(plr->szName, " totals") ;
	if (bAlloc) lstrcat(plr->szName, " allocations") ;
	if (bAdjust) lstrcat(plr->szName, " adjustments") ;
	if (bComments) lstrcat(plr->szName, " comments") ;

	switch(plrd->iEntryType) {
		case	LRDET_ALL:
			lstrcat(plr->szName, " (with labels)") ;
			break ;
		case	LRDET_LABEL:
			lstrcat(plr->szName, " (labelled entries only)") ;
			break ;
	}

	plr->tStart= plrd->t0 ;
	plr->tStop= plrd->t1 ;

	/*
		allocate "LOGRPTPERIOD" array,
		and link into "LOGRPT" structure.
	*/
	plr->plrperiod= malloc(sizeof(LOGRPTPERIOD) * nperiods) ;
	if (plr->plrperiod == (LOGRPTPERIOD *) NULL) {
		logrpt_Delete(plr) ;
		lr_errno= LRE_NOMEMORY ;
		return((LOGRPT *) NULL) ;
	}
	plr->nperiods= nperiods ;

	/*
		allocate "LOGRPTENTRY" array,
		and link into "LOGRPT" structure.
	*/
	plr->plrentry= malloc(sizeof(LOGRPTENTRY) * plrd->maxentries) ;
	if (plr->plrentry == (LOGRPTENTRY *) NULL) {
		logrpt_Delete(plr) ;
		lr_errno= LRE_NOMEMORY ;
		return((LOGRPT *) NULL) ;
	}
	plr->maxentries= plrd->maxentries ;
	plr->nentries= 0 ;

	/*
		allocate "LOGRPTCOMMENT" array,
		and link into "LOGRPT" structure.
	*/
	plr->plrcomment= malloc(sizeof(LOGRPTCOMMENT) * plrd->maxcomments) ;
	if (plr->plrcomment == (LOGRPTCOMMENT *) NULL) {
		logrpt_Delete(plr) ;
		lr_errno= LRE_NOMEMORY ;
		return((LOGRPT *) NULL) ;
	}
	plr->maxcomments= plrd->maxcomments ;
	plr->ncomments= 0 ;

	/*
		Initialize the "LOGRPTPERIOD" array.
	*/
	tStart= timeutil_tTime(plrd->t0, tut_id, 0) ;
	for (i= 0; i< (plr->nperiods); i++) {
		timeutil_szLoad(tStart, tul_loadid, plr->plrperiod[i].szName) ;
		plr->plrperiod[i].tStart= tStart ;
		tStart= timeutil_tTime(tStart, tut_id, 1) ;
		plr->plrperiod[i].tStop= tStart ;
	}
	if (plrd->lrOptions & LROPT_TOTAL) {
		for (i= 0; i< TUMAXSIZE_PERIODNAME; i++) {
			if (szTotalLiteral[i] == '\0') break ;
			plr->plrperiod[plr->nperiods-1].szName[i]= 
				szTotalLiteral[i] ;
		}
		plr->plrperiod[plr->nperiods-1].szName[i]= '\0' ;
	}

	/*
		Open the file
	*/
	if (logfile_OpenForRead(NULL)) {
		logrpt_Delete(plr) ;
		lr_errno= LRE_CANTOPEN ;
		return((LOGRPT *) NULL) ;
	}

	/*
		Allocate temporary period time accumulator array.
	*/
	plTime= (long *) malloc((plr->nperiods) * sizeof(long)) ;
	if (plTime == (long *) NULL) {
		logrpt_Delete(plr) ;
		lr_errno= LRE_NOMEMORY ;
		return((LOGRPT *) NULL) ;
	}

	bError= FALSE ;
	while(!bError && (logfile_GetRec(&lr) == 0)) {
		if (lr.icategory != LRC_PROJECT) continue ;
		switch(lr.iaction) {

			case	LRA_START:
				lTimeStart= (time_t) lr.ltime_a ;
				lstrcpy(szEntry, lr.szEname) ;
				break ;

			case	LRA_STOP:
				if (lstrcmp(szEntry, lr.szEname) != 0) {
					lr_errno= LRE_LOGFILECORRUPT ;
					bError= TRUE ;
					break ;
				}
				bSave= 0 ;
				lAccum= 0L ;
				for (i= 0; i< (plr->nperiods); i++) {
					plTime[i]= lPeriod(
						lTimeStart,
						lr.ltime_a,
						plr->plrperiod[i].tStart,
						plr->plrperiod[i].tStop
					) ;
					if (plTime[i] != 0) {
						bSave= TRUE ;
						lAccum+= plTime[i] ;
					}
				}
				if (bSave) {
					lrMakeEntryName(
						szEntry,
						szEname,
						plrd->iEntryType
					) ;
					if (
						lrAccumEntry(
							plr,
							szEname,
							plTime
						)
					) {
						bError= TRUE ;
					}
					if (bAlloc) {
						if (lrAddComment(
							plr,
							LRCT_ALLOCATION,
							szEname,
							(time_t) lr.ltime_a,
							(time_t) lTimeStart,
							lAccum
						)< 0) {
							bError= TRUE ;
						}
					}
				}
				break ;

			case	LRA_ADJUSTMENT:
				bSave= 0 ;
				lrMakeEntryName(
					lr.szEname,
					szEname,
					plrd->iEntryType
				) ;
				for (i= 0; i< (plr->nperiods); i++) {
					plTime[i]= 0 ;
					if (bPeriodTest(
						lr.ltime_a,
						plr->plrperiod[i].tStart,
						plr->plrperiod[i].tStop
					)) {
						plTime[i]= lr.lval ;
						bSave= TRUE ;
						if (bAdjust) {
							if (lrAddComment(
								plr,
								LRCT_ADJUSTMENT,
								szEname,
								(time_t) lr.ltime,
								(time_t) lr.ltime_a,
								lr.lval
							)< 0) {
								bError= TRUE ;
							}
						}
					}
				}
				if (bSave) {
					if (
						lrAccumEntry(
							plr,
							szEname,
							plTime
						)
					) {
						bError= TRUE ;
					}
				}
				break ;

			case	LRA_COMMENT:
				if (!bComments) break ;
				bSave= 0 ;
				for (i= 0; i< (plr->nperiods); i++) {
					if (bPeriodTest(
						lr.ltime_a,
						plr->plrperiod[i].tStart,
						plr->plrperiod[i].tStop
					)) {
						bSave= TRUE ;
					}
				}
				if (bSave) {
					if (lrAddComment(
						plr,
						LRCT_COMMENT,
						lr.szEname,
						(time_t) lr.ltime,
						(time_t) lr.ltime_a,
						0
					)< 0) {
						bError= TRUE ;
					}
				}
				break ;

		}
	}

	/*
		Free temporary time accumulator.
	*/
	free(plTime) ;

	/*
		Close the file
	*/
	logfile_Close() ;

	if (plrd->lrOptions & LROPT_TOTAL) {
		lrCalcTotals(plr) ;
	}

	if (lr_errno) {
		logrpt_Delete(plr) ;
		return((LOGRPT *) NULL) ;
	}

	return(plr) ;
}

void logrpt_Delete(LOGRPT *plr)
{
	int i ;

	if (plr == (LOGRPT *) NULL) return ;
	if (plr->plrperiod != (LOGRPTPERIOD *) NULL) {
		free(plr->plrperiod) ;
		plr->plrperiod= (LOGRPTPERIOD *) NULL ;
	}
	if (plr->plrentry != (LOGRPTENTRY *) NULL) {
		for (i= 0; i< (plr->nentries); i++) {
			free(plr->plrentry[i].pszName) ;
			free(plr->plrentry[i].plTime) ;
		}
		free(plr->plrentry) ;
		plr->plrentry= (LOGRPTENTRY *) NULL ;
	}
	if (plr->plrcomment != (LOGRPTCOMMENT *) NULL) {
		for (i= 0; i< (plr->ncomments); i++) {
			free(plr->plrcomment[i].pszText) ;
		}
		free(plr->plrcomment) ;
		plr->plrcomment= (LOGRPTCOMMENT *) NULL ;
	}
	free(plr) ;
}

int logrpt_errno()
{
	return(lr_errno) ;
}

/*
	start of internal subroutines
*/

/*
	lrCalcTotals

	NOTE:	this routine is only called if the "LROPT_TOTAL"
		report option was specified at report creation.
*/
static int lrCalcTotals(LOGRPT *plr)
{
	long laccum ;
	int nperiods, nentries ;
	int i, j, pi ;
	LOGRPTENTRY *plrentry ;
	
	/*
		add a final row to the table, the "total" row
	*/
	pi= lrAddEntry(plr, szTotalLiteral) ;
	if (pi< 0) return(-1) ;

	/*
		scan the table, by column by row, inserting
		column totals in the total (final) row.
	*/
	plrentry= plr->plrentry ;
	nperiods= plr->nperiods - 1 ;
	nentries= plr->nentries - 1 ;
#if 1
	if (pi != nentries) {
		lr_errno= LRE_INTERNAL ;	/* internal error */
		return(-2) ;			/* this is an assert */
	}
#endif
	for (i= 0; i< nperiods; i++) {			/* by column */
		laccum= 0L ;
		for (j= 0; j< nentries; j++) {		/* by row */
			laccum+= plrentry[j].plTime[i] ;
		}
		plrentry[nentries].plTime[i]= laccum ;
	}

	/*
		now scan the table by row by column,
		inserting the total row values in the
		total (final) column.  include the
		total (final) row in the scan.
	*/
	nentries++ ;				/* include total row */
	for (j= 0; j< nentries; j++) {			/* by row */
		laccum= 0L ;	
		for (i= 0; i< nperiods; i++) {		/* by column */
			laccum+= plrentry[j].plTime[i] ;
		}
		plrentry[j].plTime[nperiods]= laccum ;
	}

	return(pi) ;
}

static int lrAddComment(
	LOGRPT *plr,
	int ctype,
	char *pszText,
	time_t tTime,
	time_t tTime_a,
	long lval
) {
	int ci ;

	/*
		Ignore comment entries whose text is empty.
	*/
	if (pszText[0] == '\0') {
		return(0) ;
	}

	ci= (plr->ncomments) ;
	if (ci>= (plr->maxcomments)) {
		lr_errno= LRE_OVERFLOW ;
		return(-1) ;
	}

	plr->plrcomment[ci].pszText= (char *) malloc(strlen(pszText) + 1) ;
	if (plr->plrcomment[ci].pszText == (char *) NULL) {
		lr_errno= LRE_NOMEMORY ;
		return(-2) ;
	}

	lstrcpy(plr->plrcomment[ci].pszText, pszText) ;
	plr->plrcomment[ci].ctype= ctype ;
	plr->plrcomment[ci].tTime= tTime ;
	plr->plrcomment[ci].tTime_a= tTime_a ;
	plr->plrcomment[ci].lval= lval ;

	plr->ncomments++ ;
	return(ci) ;
}

static int lrAccumEntry(LOGRPT *plr, char *pszEntry, long *plTime)
{
	int i, pi ;

	/*
		Ignore entries with no name.
	*/
	if (pszEntry[0] == '\0') {
		return(0) ;
	}

	/*
		Get project index 'pi'
	*/
	if ((pi= lrFindEntry(plr, pszEntry))< 0) {
		pi= lrAddEntry(plr, pszEntry) ;
		if (pi< 0) return(1) ;
	}

	/*
		Add project times into project time array.
	*/
	for (i= 0; i< (plr->nperiods); i++) {
		plr->plrentry[pi].plTime[i]+= plTime[i] ;
	}

	return(0) ;
}

static int lrFindEntry(LOGRPT *plr, char *pszEntry)
{
	int pi ;

	for (pi= 0; pi< (plr->nentries); pi++) {
		if (strcmp(plr->plrentry[pi].pszName, pszEntry) == 0) {
			return(pi) ;
		}
	}
	return(-1) ;
}

static int lrAddEntry(LOGRPT *plr, char *pszEntry)
{
	int i, pi ;

	pi= (plr->nentries) ;
	if (pi>= (plr->maxentries)) {
		lr_errno= LRE_OVERFLOW ;
		return(-1) ;
	}

	plr->plrentry[pi].pszName= (char *) malloc(
		strlen(pszEntry) + 1
	) ;
	if (plr->plrentry[pi].pszName == (char *) NULL) {
		lr_errno= LRE_NOMEMORY ;
		return(-2) ;
	}

	plr->plrentry[pi].plTime= (long *) malloc(
		sizeof(long) * (plr->nperiods)
	) ;
	if (plr->plrentry[pi].plTime == (long *) NULL) {
		free(plr->plrentry[pi].pszName) ;
		lr_errno= LRE_NOMEMORY ;
		return(-3) ;
	}

	lstrcpy(plr->plrentry[pi].pszName, pszEntry) ;
	for (i= 0; i< (plr->nperiods); i++) {
		plr->plrentry[pi].plTime[i]= 0 ;
	}

	plr->nentries++ ;
	return(pi) ;
}

static long lPeriod(time_t ltStart, time_t ltStop, time_t ltFrom, time_t ltTo)
{
	long lValue ;

#if	0
	fprintf(fplog, "start=%ld stop=%ld from=%ld to=%ld\n", 
		ltStart, ltStop, ltFrom, ltTo) ;
#endif

	lValue= ltStop - ltStart ;
	if (ltFrom> ltStart) lValue-= (ltFrom - ltStart) ;
	if (ltTo< ltStop) lValue-= (ltStop - ltTo) ;

#if	0
	fprintf(fplog, "  ==> lValue:%ld\n", lValue) ;
#endif

	if (lValue< 0) lValue= 0 ;

#if	DEBUG
	if (lValue> 0) fprintf(fplog, "  ==> lValue:%ld\n", lValue) ;
#endif

	return(lValue) ;
}

static BOOL bPeriodTest(time_t ltTime, time_t ltFrom, time_t ltTo)
{
	if ((ltTime< ltFrom) || (ltTime> ltTo)) return(FALSE) ;
	return(TRUE) ;
}

static int lrMakeEntryName(char *pszEntry, char *pszEname, int iEntryType)
{
	char szProjectName[MAXSIZE_PROJECTNAME+1] ;
	char szProjectLabel[MAXSIZE_PROJECTLABEL+1] ;

	project_disassemble(pszEntry, szProjectName, szProjectLabel) ;

	switch(iEntryType) {

		case	LRDET_ALL:
			lstrcpy(pszEname, szProjectName) ;
			if (szProjectLabel[0] != '\0') {
				lstrcat(pszEname, " (") ;
				lstrcat(pszEname, szProjectLabel) ;
				lstrcat(pszEname, ")") ;
			}	
			break ;

		case	LRDET_NAME:
			strcpy(pszEname, szProjectName) ;
			break ;

		case	LRDET_LABEL:
			strcpy(pszEname, szProjectLabel) ;
			break ;

		default:
			pszEname[0]= '\0' ;
			return(1) ;
			break ;
	}

	return(0) ;
}

/* end of logrpt.c */
