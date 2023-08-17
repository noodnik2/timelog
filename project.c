#define	Revision	4

/*

	project.c

		
*/

#include	"wininc.h"
#include	"project.h"

#define		PTOK_DELIMCHAR		0x1B

int project_assemble(
	char *pszProject,
	char *pszProjectName,
	char *pszProjectLabel
) {
	int ln, ll, lp ;

	ln= lstrlen(pszProjectName) ;
	ll= lstrlen(pszProjectLabel) ;
	if ((ln+ll+1)>= MAXSIZE_PROJECT) return(1) ;	/* together too long */
	lstrcpy(pszProject, pszProjectName) ;
	if (pszProjectLabel[0] != '\0' ) {
		lp= lstrlen(pszProject) ;
		pszProject[lp]= PTOK_DELIMCHAR ;
		lstrcpy(pszProject+lp+1, pszProjectLabel) ;
	}
	return(0) ;
}

int project_disassemble(
	char *pszProject,
	char *pszProjectName,
	char *pszProjectLabel
) {
	int i, j ;
	char ch ;

	pszProjectName[0]= '\0' ;
	pszProjectLabel[0]= '\0' ;
	if (pszProject == (char *) NULL) return(1) ;	/* no project */

	for (i= 0; i< MAXSIZE_PROJECTNAME; i++) {
		ch= pszProject[i] ;
		if (ch == '\0') break ;
		if (ch == PTOK_DELIMCHAR) break ;
		pszProjectName[i]= ch ;
	}
	pszProjectName[i]= '\0' ;

	if (i == MAXSIZE_PROJECTNAME) {
		for (;;i++) {
			ch= pszProject[i] ;
			if (ch == '\0') break ;
			if (ch == PTOK_DELIMCHAR) break ;
		}
	}

	if (ch == PTOK_DELIMCHAR) i++ ;
	for (j= 0; j< MAXSIZE_PROJECTLABEL; j++) {
		ch= pszProject[i+j] ;
		if (ch == '\0') break ;
		if (ch == PTOK_DELIMCHAR) break ;
		pszProjectLabel[j]= ch ;
	}
	pszProjectLabel[j]= '\0' ;

	return(0) ;
}

/* end of project.c */
