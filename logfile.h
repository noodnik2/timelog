/*	Revision:	9
*/

/*
	logfile.h
*/

#ifndef	LOGFILE_H_INC
#define	LOGFILE_H_INC

#define	MAXSIZE_LOGENAME	128
#define	MAXSIZE_LOGFILENAME	128

#define	LRA_START		0x0000		/* log record actions	*/
#define	LRA_STOP		0x0001
#define	LRA_COMMENT		0x0002
#define	LRA_ADJUSTMENT		0x0003

#define	LRC_SESSION		0x0000		/* log rec categories	*/
#define	LRC_PROJECT		0x0001

#define	LFE_SUCCESS    		0		/* error codes		*/
#define	LFE_CANTOPEN   		1
#define	LFE_CANTCREATE 		2
#define	LFE_BADPARAM   		3
#define	LFE_OVERFLOW   		4
#define	LFE_IOERROR    		5
#define	LFE_INCOMPATIBLE	6

typedef struct tagLOGREC {
	long ltime ;			/* timestamp from "time()"	*/
	long ltime_a ;			/* time action to be applied	*/
	long lval ;			/* long value associated 	*/
	int iaction ;			/* action: LRA_*		*/
	int icategory ;			/* category: LRC_*		*/
	char szEname[MAXSIZE_LOGENAME+1] ; /* entry name		*/
} LOGREC ;

#define	LOGFILE_VER		0x0100	/* logfile version number	*/
#define	LOGFILE_COMPATIBILITY	0x0002	/* logfile compatibility level	*/

typedef struct tagLOGHDR {
	int ver ;			/* LOGFILE_VER			*/
	int compatibility ;		/* LOGFILE_COMPATIBILITY	*/
} LOGHDR ;

int logfile_errno(void) ;
int logfile_Init(char *pszLogFileUser) ;
int logfile_Fini(void) ;
int logfile_CreateAndOpenForWrite(LOGHDR *lhp) ;
int logfile_CheckHeader(LOGHDR *lhp) ;
int logfile_OpenForWrite(LOGHDR *lhp) ;
int logfile_OpenForRead(LOGHDR *lhp) ;
int logfile_GetRec(LOGREC *plr) ;
int logfile_PutRec(LOGREC *plr) ;
void logfile_Close(void) ;
int logfile_ReadEntry(
	long *pltime,
	long *pltime_a,
	long *plval,
	int *piaction,
	int *picategory,
	char *pszData,
	int maxdatalen
) ;
int logfile_WriteEntry(
	long ltime,
	long ltime_a,
	long lval,
	int iaction,
	int icategory,
	char *pszData
) ;

/* #ifndef LOGFILE_H_INC */
#endif

/* end of logfile.h */
