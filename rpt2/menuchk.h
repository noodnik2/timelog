/*	Revision:	1
*/
/*

	menuchk.h		MS-Windows Menu Check-Item Manipulation
		 		Copyright (C) 1994-1995, Contahal Ltd.

*/

BOOL menuchk_GetCheckByPos(HMENU hMenu, UINT ipos) ;
BOOL menuchk_SetCheckByPos(HMENU hMenu, UINT ipos, BOOL bCheck) ;
BOOL menuchk_FindItem(
	HMENU hMenuTop,
	UINT id,
	HMENU *phMenu,
	UINT *pitempos
) ;
BOOL menuchk_GetCheck(HMENU hMenu, UINT id) ;
BOOL menuchk_SetCheck(HMENU hMenu, UINT id, BOOL bCheck) ;
BOOL menuchk_ToggleCheck(HMENU hMenu, UINT id) ;
BOOL menuchk_SetListSel(HMENU hMenu, UINT id0, UINT id1, UINT isel) ;
int menuchk_GetListSel(HMENU hMenu, UINT id0, UINT id1) ;

/* end of menuchk.h */
