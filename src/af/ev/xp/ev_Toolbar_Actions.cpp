/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 


#include <stdlib.h>

#include "ev_Toolbar_Actions.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"


#define FREEP(p)	do { if (p) free(p); } while (0)
#define DELETEP(p)	do { if (p) delete(p); } while (0)

/*****************************************************************/
/*****************************************************************/

EV_Toolbar_Action::EV_Toolbar_Action(AP_Toolbar_Id id,
									 EV_Toolbar_ItemType type,
									 const char * szMethodName,
									 AV_ChangeMask maskOfInterest,
									 EV_GetToolbarItemState_pFn pfnGetState)
{
	m_id = id;
	m_type = type;
	UT_cloneString(m_szMethodName,szMethodName);
	m_maskOfInterest = maskOfInterest;
	m_pfnGetState = pfnGetState;
}

EV_Toolbar_Action::~EV_Toolbar_Action(void)
{
	FREEP(m_szMethodName);
}

AP_Toolbar_Id EV_Toolbar_Action::getToolbarId(void) const
{
	return m_id;
}

EV_Toolbar_ItemType EV_Toolbar_Action::getItemType(void) const
{
	return m_type;
}

const char * EV_Toolbar_Action::getMethodName(void) const
{
	return m_szMethodName;
}

AV_ChangeMask EV_Toolbar_Action::getChangeMaskOfInterest(void) const
{
	return m_maskOfInterest;
}

EV_Toolbar_ItemState EV_Toolbar_Action::getToolbarItemState(AV_View * pView, const char ** pszState) const
{
	if (m_pfnGetState)
		return m_pfnGetState(pView,m_id,pszState);
	else
		return EV_TIS_ZERO;
}

/*****************************************************************/
/*****************************************************************/

EV_Toolbar_ActionSet::EV_Toolbar_ActionSet(AP_Toolbar_Id first, AP_Toolbar_Id last)
{
	// TODO tis bad to call malloc/calloc from a constructor, since we cannot report failure.
	// TODO move this allocation to somewhere else.
	m_actionTable = (EV_Toolbar_Action **)calloc((last-first+1),sizeof(EV_Toolbar_Action *));
	m_first = first;
	m_last = last;
}

EV_Toolbar_ActionSet::~EV_Toolbar_ActionSet(void)
{
	if (!m_actionTable)
		return;

	UT_uint32 k, kLimit;
	for (k=0, kLimit=(m_last-m_first+1); (k<kLimit); k++)
		DELETEP(m_actionTable[k]);
	free(m_actionTable);
}

UT_Bool EV_Toolbar_ActionSet::setAction(AP_Toolbar_Id id,
										EV_Toolbar_ItemType type,
										const char * szMethodName,
										AV_ChangeMask maskOfInterest,
										EV_GetToolbarItemState_pFn pfnGetState)
{
	if ((id < m_first) || (id > m_last))
		return UT_FALSE;

	UT_uint32 index = (id - m_first);
	DELETEP(m_actionTable[index]);
	m_actionTable[index] = new EV_Toolbar_Action(id,type,szMethodName,maskOfInterest,pfnGetState);
	return (m_actionTable[index] != NULL);
}

EV_Toolbar_Action * EV_Toolbar_ActionSet::getAction(AP_Toolbar_Id id) const
{
	if ((id < m_first) || (id > m_last))
		return NULL;

	UT_uint32 index = (id - m_first);
	EV_Toolbar_Action * pAction = m_actionTable[index];
	UT_ASSERT(pAction && (pAction->getToolbarId()==id));
	return pAction;
}
