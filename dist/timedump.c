#define	Revision	1

/*

	timedump.c		TimeLog log file ASCII Dump Program
				Copyright (C) 1994, Marty Ross


	Non-exclusive permission granted for use in
	custom External TimeLog Report programs...

*/

#include	<stdio.h>
#include	<time.h>

/*
	main procedure
*/

static int timedump(char *logfn, char *tofn) ;

static char *pname= "timedump" ;

void main(int argc, char *argv[])
{
	char *infn, *outfn ;

	if (argc< 2) {
		fprintf(stderr, "%s: specify input [output] files\n", pname) ;
		exit(-1) ;
	}

	infn=		argv[1] ;
	if (argc< 3)	outfn= (char *) NULL ;
	else		outfn= argv[2] ;

	exit(timedump(infn, outfn)) ;
}

/*
	start of logfile access:	"logfile.h"
*/

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

/*
	start of logfile processing
*/

static void dumphdr(FILE *fpout, char *logfn, LOGHDR *lhp) ;
static void dumprec(FILE *fpout, LOGREC *lrp) ;

static int timedump(char *logfn, char *tofn)
{
	LOGHDR lh ;
	LOGREC lr ;
	FILE *fpout ;

	if (tofn == (char *) NULL) {
		fpout= stdout ;
		tofn= "<stdout>" ;
	}
	else {
		fpout= fopen(tofn, "w") ;
		if (fpout == (FILE *) NULL) {
			fprintf(stderr, "%s: can't open output file\n", pname) ;
			return(1) ;
		}
	}

	logfile_Init(logfn) ;
	if (logfile_OpenForRead(&lh)) {
		fprintf(
			stderr,
			"%s: error#%d opening input log file\n",
			pname,
			logfile_errno()
		) ;
		fclose(fpout) ;
		return(2) ;
	}
	
	fprintf(stderr, "%s: TimeLog log file Dump Utility\n", pname) ;
	fprintf(stderr, "Revision: %d\n", Revision) ;
	fprintf(stderr, "(writing to %s)\n", tofn) ;
	fprintf(stderr, "\n") ;

	dumphdr(fpout, logfn, &lh) ;

	while(logfile_GetRec(&lr) == 0) {
		dumprec(fpout, &lr) ;
	}

	logfile_Close() ;
	logfile_Fini() ;

	fclose(fpout) ;
	return(0) ;
}

/*
	start of internal routines
*/

static void dumphdr(FILE *fpout, char *logfn, LOGHDR *lhp)
{
	fprintf(
		fpout,
		"%s: version=0x%04X compatibility=0x%04X\n",
		logfn,
		lhp->ver,
		lhp->compatibility
	) ;
}

static char *pszaAction[]= {	"START",   "STOP",   "COMMENT",  "ADJUST"  } ;
static char *pszaCategory[]= {	"SESSION", "PROJECT" } ;

static void dumprec(FILE *fpout, LOGREC *lrp)
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

/*
	start of logfile access:	"logfile.c"
*/

#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<io.h>
#include	<stdio.h>
#include	<memory.h>

#define	MAXRECSIZE	(sizeof(LOGREC) + sizeof(int))

static char szLogFile[MAXSIZE_LOGFILENAME+1] ;
static int fd= -1 ;
static lf_errno= LFE_SUCCESS ;

static int lf_strcpyn(char *s1, char *s2, int nmax) ;

static char *FileGetBuffer(int maxlen, int *pilen) ;
static void FileAdvanceBuffer(int iamount) ;

int logfile_errno()
{
	return(lf_errno) ;
}

int logfile_Init(char *pszLogFileUser)
{
	lf_errno= LFE_SUCCESS ;
	if (lf_strcpyn(szLogFile, pszLogFileUser, MAXSIZE_LOGFILENAME+1)) {
		lf_errno= LFE_OVERFLOW ;
		return(1) ;
	}
	return(0) ;
}

int logfile_Fini()
{
	lf_errno= LFE_SUCCESS ;
	return(0) ;
}

int logfile_CreateAndOpenForWrite(LOGHDR *lhp)
{
	LOGHDR lh ;

	lf_errno= LFE_SUCCESS ;
	fd= open(
		szLogFile,
		O_RDWR | O_BINARY | O_CREAT | O_TRUNC,
		S_IREAD | S_IWRITE
	) ;
	if (fd< 0) {
		lf_errno= LFE_CANTCREATE ;
		return(-1) ;
	}

	memset(&lh, '\0', sizeof(LOGHDR)) ;
	lh.ver= LOGFILE_VER ;
	lh.compatibility= LOGFILE_COMPATIBILITY ;

	if (write(fd, &lh, sizeof(LOGHDR)) != sizeof(LOGHDR)) {
		lf_errno= LFE_IOERROR ;
		close(fd) ;
		return(-2) ;
	}

	if (lhp != (LOGHDR *) NULL) memcpy(lhp, &lh, sizeof(LOGHDR)) ;
	return(0) ;
}

int logfile_CheckHeader(LOGHDR *lhp)
{
	LOGHDR lh ;

	lf_errno= LFE_SUCCESS ;
	lseek(fd, 0L, SEEK_SET) ;
	if (read(fd, &lh, sizeof(LOGHDR)) != sizeof(LOGHDR)) {
		lf_errno= LFE_IOERROR ;
		return(-1) ;
	}
	if (lhp != (LOGHDR *) NULL) memcpy(lhp, &lh, sizeof(LOGHDR)) ;
	if (lh.compatibility != LOGFILE_COMPATIBILITY) {
		lf_errno= LFE_INCOMPATIBLE ;
		return(1) ;
	}
	return(0) ;
}

int logfile_OpenForWrite(LOGHDR *lhp)
{
	lf_errno= LFE_SUCCESS ;
	fd= open(szLogFile, O_RDWR | O_BINARY) ;
	if (fd>= 0) {
		if (logfile_CheckHeader(lhp)) {
			close(fd) ;
			fd= -1 ;
			return(1) ;
		}
		return(0) ;
	}
	return(logfile_CreateAndOpenForWrite(lhp)) ;
}

int logfile_OpenForRead(LOGHDR *lhp)
{
	lf_errno= LFE_SUCCESS ;
	fd= open(szLogFile, O_RDONLY | O_BINARY) ;
	if (fd< 0) {
		lf_errno= LFE_CANTOPEN ;
		return(1) ;
	}
	return(logfile_CheckHeader(lhp)) ;
}

int logfile_GetRec(LOGREC *plr)
{
	char *pcafilebuf, *pcadatabuf ;
	int ifilelen, idatalen ;

	lf_errno= LFE_SUCCESS ;
	pcafilebuf= FileGetBuffer(MAXRECSIZE, &ifilelen) ;
	if (ifilelen< sizeof(int)) {
		lf_errno= LFE_IOERROR ;
		return(-1) ;
	}

	pcadatabuf= pcafilebuf + sizeof(int) ;
	idatalen= *((int *) pcafilebuf) ;
	if (idatalen< 0) {
		lf_errno= LFE_IOERROR ;
		return(-2) ;
	}

	memset(plr, '\0', sizeof(LOGREC)) ;
	memcpy(plr, pcadatabuf, idatalen) ;

	ifilelen= idatalen + sizeof(int) ;
	FileAdvanceBuffer(ifilelen) ;
	return(0) ;
}

int logfile_PutRec(LOGREC *plr)
{
	int ifilelen, idatalen ;
	char cafilerec[MAXRECSIZE] ;
	char *pcadatarec ;

	lf_errno= LFE_SUCCESS ;

	/*
		right-trim the record of trailing zeroes
	*/
	pcadatarec= (char *) plr ;
	for (idatalen= sizeof(LOGREC); idatalen> 0; idatalen--) {
		if (pcadatarec[idatalen-1] != '\0') break ;
	}

	/*
		construct the file record
	*/
	ifilelen= idatalen + sizeof(int) ;
	*((int *)&cafilerec[0])= idatalen ;
	if (idatalen> 0) {
		memcpy(&cafilerec[sizeof(int)], pcadatarec, idatalen) ;
	}

	/*
		write record
	*/
	lseek(fd, 0L, SEEK_END) ;
	if (write(fd, cafilerec, ifilelen) != ifilelen) {
		lf_errno= LFE_IOERROR ;
		return(-1) ;
	}

	return(0) ;
}

void logfile_Close()
{
	if (fd>= 0) {
		close(fd) ;
		fd= -1 ;
	}
}

int logfile_ReadEntry(
	long *pltime,
	long *pltime_a,
	long *plval,
	int *piaction,
	int *picategory,
	char *pszEname,
	int maxdatalen
) {
	LOGREC lr ;
	int rc ;

	lf_errno= LFE_SUCCESS ;

	rc= logfile_GetRec(&lr) ;
	if (rc != 0) return(rc) ;
	
	(*pltime)= lr.ltime ;
	(*pltime_a)= lr.ltime_a ;
	(*plval)= lr.lval ;
	(*piaction)= lr.iaction ;
	(*picategory)= lr.icategory ;
	lf_strcpyn(pszEname, lr.szEname, maxdatalen) ;
	return(0) ;
}

int logfile_WriteEntry(
	long ltime,
	long ltime_a,
	long lval,
	int iaction,
	int icategory,
	char *pszEname
) {
	LOGREC lr ;

	lf_errno= LFE_SUCCESS ;

	/*
		create log record
	*/
	memset(&lr, '\0', sizeof(LOGREC)) ;
	lr.ltime= ltime ;
	lr.ltime_a= ltime_a ;
	lr.lval= lval ;
	lr.iaction= iaction ;
	lr.icategory= icategory ;
	lf_strcpyn(lr.szEname, pszEname, MAXSIZE_LOGENAME+1) ;

	/*
		write log record at end of file
	*/
	return(logfile_PutRec(&lr)) ;
}

/*
	internal functions
*/

static int lf_strcpyn(char *s1, char *s2, int nmax)
{
	int ovfl ;

	ovfl= 0 ;
	if (s2 != (char *) NULL) {
		if (((int) strlen(s2))>= nmax) {
			memcpy(s1, s2, nmax-1) ;
			s1[nmax-1]= '\0' ;
			lf_errno= LFE_OVERFLOW ;
			ovfl= 1 ;
		}
		else {
			strcpy(s1, s2) ;
		}
	}
	else {
		s1[0]= '\0' ;
	}
	return(ovfl) ;
}

/*
	Routines to support buffered reading of variable
	length records.

	void *FileGetBuffer(int maxlen, int *pilen) ;
	void FileAdvance(int bytes) ;

	FileGetBuffer(int maxlen, int *pilen):

		maintains status:
			buffer
			length of current buffer
			current offset into buffer

		if ((maxlen + curoffset)>= curlength) {
			move fragment to start of buffer
			curlength= curlength - curoffset ;
			curoffset= 0 ;
			curlength+= read(
				fh,
				buffer+curlength,
				sizeof(buffer)-curlength
			) ;
		}
		
		(*pilen)= min(maxlen, curlength - curoffset) ;
		return((void *) (buffer + curoffset)) ;

	FileAdvance(int iamount):
		
		curoffset+= iamount ;
		
*/

#define		FILEBUFFERSIZE	(MAXRECSIZE * 32)

static char buffer[FILEBUFFERSIZE] ;
static int curlength= 0 ;
static int curoffset= 0 ;

static char *FileGetBuffer(int maxlen, int *pilen)
{
	int l ;

	/*
		if there is possibly not a complete record in the buffer,
		then we move the possibly incomplete fragment to the start
		of the buffer, and we re-fill the buffer from the file.
	*/
	if ((maxlen+curoffset)>= curlength) {
					/* move fragment to start of buffer */
		curlength-= curoffset ;
#if 1
					/* this code assumes no overlap */
		memcpy(buffer, buffer+curoffset, curlength) ;
#else
		{
			int i ;
					/* this code handles overlap case */
			for (i= 0; i< curlength; i++) {
				buffer[i]= buffer[curoffset+i] ;
			}
		}
#endif
		curoffset= 0 ;
		
		l= read(fd, buffer+curlength, FILEBUFFERSIZE-curlength) ;
		if (l> 0) curlength+= l ;
	}
	(*pilen)= curlength - curoffset ;
	if ((*pilen)> maxlen) (*pilen)= maxlen ;

	return(buffer + curoffset) ;
}

static void FileAdvanceBuffer(int iamount)
{
	curoffset+= iamount ;
}

/*
	end of logfile access
*/

/* end of timedump.c */
