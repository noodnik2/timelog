/*	Revision:	1
*/
/*

	menuchk.c		MS-Windows Menu Check-Item Manipulation
		 		Copyright (C) 1994-1995, Contahal Ltd.

*/

#include	<windows.h>
#include	"menuchk.h"

/*
	function group:	dialog menu control manipulation
*/

BOOL menuchk_GetCheckByPos(HMENU hMenu, UINT ipos)
{
	UINT fFlags ;

	fFlags= GetMenuState(hMenu, ipos, MF_BYPOSITION) ;
	if (fFlags & MF_CHECKED) return(TRUE) ;
	return(FALSE) ;
}

BOOL menuchk_SetCheckByPos(HMENU hMenu, UINT ipos, BOOL bCheck)
{
	CheckMenuItem(
		hMenu, ipos,
		MF_BYPOSITION | (bCheck? MF_CHECKED: MF_UNCHECKED)
	) ;
	return(TRUE) ;
}

/*
	BOOL menuchk_FindItem

	This function was written to find a menu item by its COMMAND
	id value, no matter where it is located in the menu tree.

	In my opinion, it is stupid that the Windows API doesn't give
	me a function for this.  If I want to Get/Set a check on a
	menu item in a "sub-popup menu", I must do it according to
	its position, which I want to be free to change without
	recompiling!

	NOTE:	this function is recursive!

*/
BOOL menuchk_FindItem(
	HMENU hMenuTop,
	UINT id,
	HMENU *phMenu,
	UINT *pitempos
) {
	HMENU hMenuSub ;
	UINT id0 ;
	int i, nMenuItems ;
	BOOL bOk ;

	bOk= FALSE ;
	nMenuItems= GetMenuItemCount(hMenuTop) ;
	for (i= 0; i< nMenuItems; i++) {
		hMenuSub= GetSubMenu(hMenuTop, i) ;
		if (hMenuSub != (HMENU) NULL) {		/* its a submenu */
			if (menuchk_FindItem(hMenuSub, id, phMenu, pitempos)) {
				bOk= TRUE ;
				break ;
			}
			continue ;
		}
		id0= GetMenuItemID(hMenuTop, i) ;
		if (id0 == id) {
			(*phMenu)= hMenuTop ;
			(*pitempos)= i ;
			bOk= TRUE ;
			break ;
		}
	}

	return(bOk) ;
}

BOOL menuchk_GetCheck(HMENU hMenu, UINT id)
{
	HMENU hMenuItem ;
	UINT ipos ;

	if (!menuchk_FindItem(hMenu, id, &hMenuItem, &ipos)) return(FALSE) ;
	return(menuchk_GetCheckByPos(hMenuItem, ipos)) ;
}

BOOL menuchk_SetCheck(HMENU hMenu, UINT id, BOOL bCheck)
{
	HMENU hMenuItem ;
	UINT ipos ;

	if (!menuchk_FindItem(hMenu, id, &hMenuItem, &ipos)) return(FALSE) ;
	return(menuchk_SetCheckByPos(hMenuItem, ipos, bCheck)) ;
}

BOOL menuchk_ToggleCheck(HMENU hMenu, UINT id)
{
	HMENU hMenuItem ;
	UINT ipos ;
	BOOL bCheck ;

	if (!menuchk_FindItem(hMenu, id, &hMenuItem, &ipos)) return(FALSE) ;
	bCheck= menuchk_GetCheckByPos(hMenuItem, ipos) ;
	return(menuchk_SetCheckByPos(hMenuItem, ipos, !bCheck)) ;
}

/*
	note:	a "menu selection list" has:

	(1)	ID numbers from "id0" to "id1", inclusive
		(and appears in this order on the menu)
	(2)	is located in the same pop-up menu, in
		contiguous "MF_POSITION" order.
*/
BOOL menuchk_SetListSel(HMENU hMenu, UINT id0, UINT id1, UINT isel)
{
	HMENU hMenuList ;
	UINT ipos, ipos0, ipos1, ipos2set ;

	if (!menuchk_FindItem(hMenu, id0, &hMenuList, &ipos0)) return(FALSE) ;
	ipos1= ipos0 + (id1 - id0) ;

	ipos2set= ipos0 + isel ;
	for (ipos= ipos0; ipos<= ipos1; ipos++) {
		menuchk_SetCheckByPos(
			hMenuList, ipos, (BOOL) (ipos == ipos2set)
		) ;
	}
	return(TRUE) ;
}

int menuchk_GetListSel(HMENU hMenu, UINT id0, UINT id1)
{
	HMENU hMenuList ;
	UINT ipos, ipos0, ipos1 ;

	if (!menuchk_FindItem(hMenu, id0, &hMenuList, &ipos0)) return(FALSE) ;
	ipos1= ipos0 + (id1 - id0) ;
	
	for (ipos= ipos0; ipos<= ipos1; ipos++) {
		if (menuchk_GetCheckByPos(hMenuList, ipos)) {
			return(ipos - ipos0) ;
		}
	}
	return(-1) ;
}

/* end of menuchk.c */
