/*	Revision:	1
*/
/*

	project.h

		
*/

#ifndef PROJECT_H_INC
#define	PROJECT_H_INC

#define		MAXSIZE_PROJECT		64
#define		MAXSIZE_PROJECTNAME	31
#define		MAXSIZE_PROJECTLABEL	31

int project_assemble(
	char *pszProject,
	char *pszProjectName,
	char *pszProjectLabel
) ;
int project_disassemble(
	char *pszProject,
	char *pszProjectName,
	char *pszProjectLabel
) ;

/* #ifndef PROJECT_H_INC */
#endif

/* end of project.h */
