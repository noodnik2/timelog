/*	Revision:	11
*/

/*

	tldlg.h				TimeLog Dialog Definitions

*/

#ifndef	TLDLG_H_INC
#define	TLDLG_H_INC

#include	"timelog.h"
#include	"project.h"
#include	"tloghlp.h"

#define	MAXSIZE_EDITNUMBER	16	/* max size of editable numbers	*/
#define	MAXSIZE_COMMENT		128	/* max size of comment		*/

					/* timelog dialog user messages	*/
#define	UMD_RESTATE		(WM_USER+1)

typedef struct tagDLG {
	time_t tTime ;
	char szCmtText[MAXSIZE_COMMENT+1] ;
	char szOvhProjectName[MAXSIZE_PROJECTNAME+1] ;
	char szSelProjectName[MAXSIZE_PROJECTNAME+1] ;
	BOOL bAutoDialog ;
	int iTimeDisplayFormat ;
	int iTimeScope ;
} DLG, FAR *LPDLG ;	

typedef struct tagDLGNEW {
	char szNewProjectName[MAXSIZE_PROJECTNAME+1] ;
	char szNewProjectLabel[MAXSIZE_PROJECTLABEL+1] ;
} DLGNEW, FAR *LPDLGNEW ;

typedef struct tagDLGADJ {
	time_t tAdjRef ;
	int iaHours ;
	int iaMinutes ;
	char szAdjProjectName[MAXSIZE_PROJECTNAME+1] ;
	char szCmtText[MAXSIZE_COMMENT+1] ;
} DLGADJ, FAR *LPDLGADJ ;

/* #ifndef	TLDLG_H_INC */
#endif

/* end of tldlg.h */
