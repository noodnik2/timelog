/*	Revision	24
*/
/*

	logrpt.h

*/

#ifndef	LOGRPT_H_INC
#define	LOGRPT_H_INC

#include	"timeutil.h"

#define	LRT_MONTHDAYS		0
#define	LRT_YEARMONTHS		1
#define	LRT_YEARS		2

#define	LRE_SUCCESS		0	/* no problem			*/
#define	LRE_BADPARAM		1	/* bad parameters on input	*/
#define	LRE_NOMEMORY		2	/* ran out of memory		*/
#define	LRE_CANTOPEN		3	/* cant open log file		*/
#define	LRE_OVERFLOW		4	/* overflow in calculations	*/
#define	LRE_LOGFILECORRUPT	5	/* error reading log file	*/
#define	LRE_INTERNAL   		6	/* internal error: programmer	*/

#define	LRMAXSIZE_REPORTNAME	256	/* max size of report name	*/
#define	LRMAXSIZE_USERNAME	64	/* max size of user name  	*/
#define	LRMAXSIZE_ENAME		72	/* max size of report entry	*/

typedef struct tagLOGRPTPERIOD {
	time_t tStart ;		/* Start of period (inclusive)	*/
	time_t tStop ;		/* End of period (exclusive)	*/
	char szName[TUMAXSIZE_PERIODNAME+1] ;	/* name of per	*/
} LOGRPTPERIOD ;

typedef struct tagLOGRPTENTRY {
	char *pszName ;		/* Name of entry  		*/
	long *plTime ;		/* -> long[nperiods]		*/
} LOGRPTENTRY ;

					/* LOGRPTCOMMENT.ctype:	*/
#define	LRCT_COMMENT		0		/* standard	*/
#define	LRCT_ADJUSTMENT		1		/* adjustment	*/
#define	LRCT_ALLOCATION		2		/* allocation	*/
typedef struct tagLOGRPTCOMMENT {
	int ctype ;		/* comment type			*/
	char *pszText ;		/* Text of comment		*/
	time_t tTime ;		/* timestamp on comment		*/
	time_t tTime_a ;	/* actual time for comment	*/
	long lval ;		/* lvalue for comment		*/
} LOGRPTCOMMENT ;

typedef struct tagLOGRPT {

	char szName[LRMAXSIZE_REPORTNAME+1] ;	/* report name		*/
	char szUser[LRMAXSIZE_USERNAME+1] ;	/* user name		*/

	time_t tStart ;		       		/* start of period	*/
	time_t tStop ;		       		/* end of period	*/

	int nperiods ;				/* number of periods	*/
	LOGRPTPERIOD *plrperiod ;      		/* -> [nperiods]	*/

	int maxentries ;			/* max # of entries	*/
	int nentries ;				/* number of entries	*/
	LOGRPTENTRY *plrentry ;    		/* -> [nentries]	*/

	int maxcomments ;			/* max # of comments	*/
	int ncomments ;				/* number of comments	*/
	LOGRPTCOMMENT *plrcomment ;		/* -> [ncomments]	*/

} LOGRPT ;

#define	LRDET_NAME	0			/* (LOGRPTDEF).iEntryType */
#define	LRDET_LABEL	1
#define	LRDET_ALL	2

#define	LROPT_DEFAULT	0			/* default options	*/
#define	LROPT_TOTAL	1			/* include totals	*/
#define	LROPT_COMMENTS	2			/* include comments	*/
#define	LROPT_ALLOC	4			/* include allocations	*/
#define	LROPT_ADJUST	8			/* include adjustments	*/

typedef struct tagLOGRPTDEF {
	char szUser[LRMAXSIZE_USERNAME+1] ;	/* user name		*/
	time_t t0 ;
	time_t t1 ;
	int lrtype ;
	int maxentries ;
	int maxcomments ;
	int iEntryType ;
	int lrOptions ;
} LOGRPTDEF ;

int logrpt_Init(void) ;
void logrpt_Fini(void) ;
LOGRPT *logrpt_Create(LOGRPTDEF *plrd) ;
void logrpt_Delete(LOGRPT *plr) ;
int logrpt_errno(void) ;

/* #ifndef LOGRPT_H_INC */
#endif

/* end of logrpt.h */
