/*	Revision:	34
*/

#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<string.h>
#include	<io.h>
#include	<stdio.h>
#include	<memory.h>

#include	"logfile.h"

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

/* end of logfile.c */
