 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include <stdlib.h>

#include "ev_Menu_Actions.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"


#define FREEP(p)	do { if (p) free(p); } while (0)

/*****************************************************************/
/*****************************************************************/

EV_Menu_Action::EV_Menu_Action(AP_Menu_Id id,
							   UT_Bool bHoldsSubMenu,
							   UT_Bool bRaisesDialog,
							   UT_Bool bCheckable,
							   const char * szMethodName,
							   EV_GetMenuItemState_pFn pfnGetState,
							   EV_GetMenuItemComputedLabel_pFn pfnGetLabel)
{
	UT_ASSERT((bHoldsSubMenu + bRaisesDialog + bCheckable) < 2); // a 3-way exclusive OR
	UT_ASSERT((!bCheckable) || pfnGetState);
	
	m_id			= id;
	m_bHoldsSubMenu	= bHoldsSubMenu;
	m_bRaisesDialog = bRaisesDialog;
	m_bCheckable	= bCheckable;
	UT_cloneString(m_szMethodName,szMethodName);
	m_pfnGetState	= pfnGetState;
	m_pfnGetLabel	= pfnGetLabel;
}

EV_Menu_Action::~EV_Menu_Action(void)
{
	FREEP(m_szMethodName);
}

AP_Menu_Id EV_Menu_Action::getMenuId(void) const
{
	return m_id;
}

/*****************************************************************/
/*****************************************************************/

EV_Menu_ActionSet::EV_Menu_ActionSet(AP_Menu_Id first, AP_Menu_Id last)
{
	// TODO tis bad to call malloc/calloc from a constructor, since we cannot report failure.
	// TODO move this allocation to somewhere else.
	m_actionTable = (EV_Menu_Action **)calloc((last-first+1),sizeof(EV_Menu_Action *));
	m_first = first;
	m_last = last;
}

EV_Menu_ActionSet::~EV_Menu_ActionSet(void)
{
	if (!m_actionTable)
		return;

	UT_uint32 k, kLimit;
	for (k=0, kLimit=(m_last-m_first+1); (k<kLimit); k++)
		FREEP(m_actionTable[k]);
	free(m_actionTable);
}

UT_Bool EV_Menu_ActionSet::setAction(AP_Menu_Id id,
									 UT_Bool bHoldsSubMenu,
									 UT_Bool bRaisesDialog,
									 UT_Bool bCheckable,
									 const char * szMethodName,
									 EV_GetMenuItemState_pFn pfnGetState,
									 EV_GetMenuItemComputedLabel_pFn pfnGetLabel)
{
	if ((id < m_first) || (id > m_last))
		return UT_FALSE;

	UT_uint32 index = (id - m_first);
	FREEP(m_actionTable[index]);
	m_actionTable[index] = new EV_Menu_Action(id,bHoldsSubMenu,bRaisesDialog,bCheckable,
											  szMethodName,pfnGetState,pfnGetLabel);
	return (m_actionTable[index] != NULL);
}

EV_Menu_Action * EV_Menu_ActionSet::getAction(AP_Menu_Id id) const
{
	if ((id < m_first) || (id > m_last))
		return NULL;

	UT_uint32 index = (id - m_first);
	EV_Menu_Action * pAction = m_actionTable[index];
	UT_ASSERT(pAction && (pAction->getMenuId()==id));
	return pAction;
}
