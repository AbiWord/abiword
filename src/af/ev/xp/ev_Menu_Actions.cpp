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

/*!
 * This class is only a vector of EV_Menu_Action, but instead
 * of start with an index of 0, we start with an index of
 * "first" (and we finish at "last"), but of course the underlying
 * vector will still start at 0 and finish at (last - first).
 *
 * In practice, "first" will be AP_MENU_ID__BOGUS1__, and "last"
 * will be AP_MENU_ID__BOGUS2__.
 */
EV_Menu_ActionSet::EV_Menu_ActionSet(XAP_Menu_Id first, XAP_Menu_Id last)
	: m_actionTable(last - first + 1),
	  m_first(first)
{
	size_t nb_items = last - first + 1;
	size_t i;
	
	for (i = 0; i < nb_items; ++i)
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

	if ((id < m_first) || (id >= m_first + (UT_sint32)m_actionTable.size()))
		return false;

	UT_uint32 index = (id - m_first);
	EV_Menu_Action *pAction = new EV_Menu_Action(id,bHoldsSubMenu,bRaisesDialog,bCheckable,
												 szMethodName,pfnGetState,pfnGetLabel);
	UT_uint32 error = m_actionTable.setNthItem(index, pAction, &tmp);

	EV_Menu_Action * pTmpAction = static_cast<EV_Menu_Action *> (tmp);
	DELETEP(pTmpAction);
	return (error == 0);
}

const EV_Menu_Action * EV_Menu_ActionSet::getAction(XAP_Menu_Id id) const
{
	xxx_UT_DEBUGMSG(("JCA: EV_Menu_ActionSet::getAction(%d) m_first = [%d], size_table = [%d]\n", id, m_first, m_actionTable.size()));
	if ((id < m_first) || (id > m_first + (UT_sint32)m_actionTable.size()))
		return NULL;

	UT_uint32 index = (id - m_first);
	const EV_Menu_Action * pAction = static_cast<const EV_Menu_Action *> (m_actionTable[index]);
	UT_ASSERT(pAction && (pAction->getMenuId() == id));
	return pAction;
}

bool EV_Menu_ActionSet::addAction(EV_Menu_Action *pAction)
{
	UT_DEBUGMSG(("JCA: EV_Menu_ActionSet::addAction\n"));
	UT_ASSERT(pAction);
	size_t size_table = m_actionTable.size();
	UT_DEBUGMSG(("pAction->getMenuId() = [%d], size_table = [%d], m_first = [%d]\n",
				 pAction->getMenuId(), size_table, m_first));

#ifdef DEBUG
	if (pAction->getMenuId() < (UT_sint32)size_table + m_first - 1 || pAction->getMenuId() > (UT_sint32)size_table + m_first)
		UT_DEBUGMSG(("WARNING: Weird menu id.\n"));
#endif

	m_actionTable.insertItemAt(pAction, pAction->getMenuId() - m_first);
	return (size_table + 1 == m_actionTable.size());
}
