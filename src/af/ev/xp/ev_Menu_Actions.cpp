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

#include "ev_Menu_Actions.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"


/*****************************************************************/
/*****************************************************************/

EV_Menu_Action::EV_Menu_Action(XAP_Menu_Id id,
							   bool bHoldsSubMenu,
							   bool bRaisesDialog,
							   bool bCheckable,
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

EV_Menu_Action::~EV_Menu_Action()
{
	FREEP(m_szMethodName);
}

XAP_Menu_Id EV_Menu_Action::getMenuId() const
{
	return m_id;
}

bool EV_Menu_Action::hasDynamicLabel() const
{
	return (m_pfnGetLabel != NULL);
}

const char * EV_Menu_Action::getDynamicLabel(XAP_Frame * pFrame, const EV_Menu_Label * pLabel) const
{
	if (m_pfnGetLabel)
		return m_pfnGetLabel(pFrame,pLabel,m_id);
	else
		return NULL;
}

bool EV_Menu_Action::hasGetStateFunction() const
{
	return (m_pfnGetState != NULL);
}

EV_Menu_ItemState EV_Menu_Action::getMenuItemState(AV_View * pView) const
{
	if (m_pfnGetState)
		return m_pfnGetState(pView,m_id);
	else
		return EV_MIS_ZERO;
}

const char * EV_Menu_Action::getMethodName() const
{
	return m_szMethodName;
}

bool EV_Menu_Action::raisesDialog() const
{
	return m_bRaisesDialog;
}

bool EV_Menu_Action::isCheckable() const
{
	return m_bCheckable;
}

/*****************************************************************/
/*****************************************************************/

EV_Menu_ActionSet::EV_Menu_ActionSet(XAP_Menu_Id first, XAP_Menu_Id last)
	: m_actionTable(last - first + 1),
	  m_first(first),
	  m_last(last)
{
	for (size_t i = 0; i < m_actionTable.getItemCount(); ++i)
		m_actionTable.addItem(0);
}

EV_Menu_ActionSet::~EV_Menu_ActionSet()
{
	UT_VECTOR_SPARSEPURGEALL(EV_Menu_Action *, m_actionTable);
}

bool EV_Menu_ActionSet::setAction(XAP_Menu_Id id,
								  bool bHoldsSubMenu,
								  bool bRaisesDialog,
								  bool bCheckable,
								  const char * szMethodName,
								  EV_GetMenuItemState_pFn pfnGetState,
								  EV_GetMenuItemComputedLabel_pFn pfnGetLabel)
{
	void *tmp;

	if ((id < m_first) || (id > m_last))
		return false;

	UT_uint32 index = (id - m_first);
	EV_Menu_Action *pAction = new EV_Menu_Action(id,bHoldsSubMenu,bRaisesDialog,bCheckable,
												 szMethodName,pfnGetState,pfnGetLabel);
	UT_uint32 error = m_actionTable.setNthItem(index, pAction, &tmp);

	EV_Menu_Action * pTmpAction = static_cast<EV_Menu_Action *> (tmp);
	DELETEP(pTmpAction);
	return (error == 0);
}

EV_Menu_Action * EV_Menu_ActionSet::getAction(XAP_Menu_Id id) const
{
	if ((id < m_first) || (id > m_last))
		return NULL;

	UT_uint32 index = (id - m_first);
	EV_Menu_Action * pAction = static_cast<EV_Menu_Action *> (m_actionTable[index]);
	UT_ASSERT(pAction && (pAction->getMenuId()==id));
	return pAction;
}

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) < (b) ? (b) : (a))

bool EV_Menu_ActionSet::addAction(EV_Menu_Action *pAction)
{
	int id = pAction->getMenuId();
	int i;
	void *tmp;

	UT_DEBUGMSG(("JCA: EV_Menu_ActionSet::addAction\n"));
	if ((id >= m_first) && (id <= m_last))
	{
		// here we do exactly the same thing that setAction
		UT_uint32 index = (id - m_first);
		UT_uint32 error = m_actionTable.setNthItem(index, pAction, &tmp);
		EV_Menu_Action * pTmpAction = static_cast<EV_Menu_Action *> (tmp);
		DELETEP(pTmpAction);

		return (error == 0);
	}

	for (i = m_first; i > id; --i)
		UT_ASSERT(UT_NOT_IMPLEMENTED);
	for (i = m_last; i < id; ++i)
		m_actionTable.addItem(0);

	m_actionTable.setNthItem(id - m_first, pAction, &tmp);
	UT_ASSERT(tmp == 0);

	m_first = min(id, m_first);
	m_last = max(id, m_last);

	return true;
}
