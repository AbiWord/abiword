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
 



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ev_Menu.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "xap_Menu_Layouts.h"
#include "xap_Menu_LabelSet.h"
#include "xap_App.h"
#include "xap_Frame.h"


/*****************************************************************/
// Utility function.  Too bad we're not using std:: stuff... that should
// be something like find(labels.begin(), labels.end(), label);

static XAP_Menu_Id
searchMenuLabel(const EV_Menu_LabelSet &labels, const UT_String &label)
{
	const UT_Vector &labels_table = labels.getAllLabels();
	const EV_Menu_Label *l = 0;
	UT_uint32 size_labels = labels_table.size();
	XAP_Menu_Id id = 0;

	for (UT_uint32 i = 0; i < size_labels; ++i)
	{
		l = static_cast<EV_Menu_Label*> (labels_table[i]);
		if (l && label == l->getMenuLabel())
		{
			id = l->getMenuId();
			break;
		}
	}

	return id;
}

/*****************************************************************/

EV_Menu::EV_Menu(EV_EditMethodContainer * pEMC,
				 const char * szMenuLayoutName,
				 const char * szMenuLabelSetName)
{
	UT_ASSERT(pEMC);

	m_pEMC = pEMC;

	//UT_DEBUGMSG(("EV_Menu: Creating menu for [layout %s, language %s]\n",
	//			 szMenuLayoutName,szMenuLabelSetName));
	
	m_pMenuLayout = AP_CreateMenuLayout(szMenuLayoutName);
	UT_ASSERT(m_pMenuLayout);

	m_pMenuLabelSet = AP_CreateMenuLabelSet(szMenuLabelSetName);
	UT_ASSERT(m_pMenuLabelSet);

}

EV_Menu::~EV_Menu()
{
	DELETEP(m_pMenuLayout);
	DELETEP(m_pMenuLabelSet);
}

const EV_Menu_Layout * EV_Menu::getMenuLayout() const
{
	return m_pMenuLayout;
}

const EV_Menu_LabelSet * EV_Menu::getMenuLabelSet() const
{
	return m_pMenuLabelSet;
}

void EV_Menu::addPath(const UT_String &path)
{
	UT_DEBUGMSG(("Adding path %s.\n", path.c_str()));
	UT_Vector *names = simpleSplit(path, '/');
	EV_Menu_ActionSet *pMenuActionSet = getApp()->getMenuActionSet();
	const UT_String *label;
	UT_uint32 last_pos = 1;
	XAP_Menu_Id last_index = 0;
	XAP_Menu_Id index = 0;
	UT_ASSERT(names);
	UT_ASSERT(m_pMenuLabelSet);
	UT_ASSERT(pMenuActionSet);

	// if need, we create submenus
	UT_DEBUGMSG(("Gonna create submenus...\n"));
	size_t end = names->size() - 1;
	for (size_t i = 0; i < end; ++i)
	{
		label = static_cast<UT_String*> ((*names)[i]);
		UT_ASSERT(label);
		index = searchMenuLabel(*m_pMenuLabelSet, *label);

		// Here we should create end - i submenus
		if (index == 0)
		{
			UT_DEBUGMSG(("... yes.  i = [%d], end = [%d]\n", i, end));
			UT_uint32 lpos = m_pMenuLayout->getLayoutIndex(last_index);

			// and now we add the new submenus
			for (size_t j = i; j < end; ++j)
			{
				label = static_cast<UT_String*> ((*names)[j]);
				UT_ASSERT(label);
				index = m_pMenuLayout->addLayoutItem(++lpos, EV_MLF_BeginSubMenu);
				pMenuActionSet->addAction(new EV_Menu_Action(index, true, false, false, NULL, NULL, NULL));
				m_pMenuLabelSet->addLabel(new EV_Menu_Label(index, label->c_str(), label->c_str()));
				_doAddMenuItem(lpos);
			}

			last_pos = lpos + 1;

			// and we close the submenus
			for (size_t k = i; k < end; ++k)
			{
				m_pMenuLayout->addFakeLayoutItem(++lpos, EV_MLF_EndSubMenu);
				_doAddMenuItem(lpos);
			}

			break;
		}

		last_index = index;
	}

	if (index != 0)
		last_pos = m_pMenuLayout->getLayoutIndex(last_index) + 1;

	UT_DEBUGMSG(("Gonna add a menu item.\n"));
	// and now we create the menu item
	index = m_pMenuLayout->addLayoutItem(last_pos, EV_MLF_Normal);
	pMenuActionSet->addAction(new EV_Menu_Action(index, false, false, false, "scriptPlay", NULL, NULL));
	m_pMenuLabelSet->addLabel(new EV_Menu_Label(index, ((UT_String *) names->back())->c_str(),
												((UT_String *) names->back())->c_str()));

	if (!_doAddMenuItem(last_pos))
	{
		UT_ASSERT(UT_NOT_IMPLEMENTED);
#if 0
		pMenuActionSet->deleteAction(index);
		m_pMenuLabelSet->deleteLabel(index);
		m_pMenuLayout->deleteLayoutItem(index);
#endif
	}

	delete names;
}

bool EV_Menu::invokeMenuMethod(AV_View * pView,
							   EV_EditMethod * pEM,
							   UT_UCSChar * pData,
							   UT_uint32 dataLength)
{
	UT_ASSERT(pView);
	UT_ASSERT(pEM);

	//UT_DEBUGMSG(("invokeMenuMethod: %s\n",pEM->getName()));

	EV_EditMethodType t = pEM->getType();

	if (((t & EV_EMT_REQUIREDATA) != 0) && (!pData || !dataLength))
	{
		// This method requires character data and the caller did not provide any.
		UT_DEBUGMSG(("    invoke aborted due to lack of data\n"));
		return false;
	}

	EV_EditMethodCallData emcd(pData,dataLength);
	(*pEM->getFn())(pView,&emcd);

	return true;
}


/* replace _ev_GetLabelName () */
/* this version taken from ev_UnixMenu.cpp */
const char ** EV_Menu::getLabelName(XAP_App * pApp,  XAP_Frame * pFrame,
									EV_Menu_Action * pAction, EV_Menu_Label * pLabel)
{
	static const char * data[2] = {NULL, NULL};

	// hit the static pointers back to null each time around
	data[0] = NULL;
	data[1] = NULL;
	
	const char * szLabelName;
	
	if (pAction->hasDynamicLabel())
		szLabelName = pAction->getDynamicLabel(pFrame,pLabel);
	else
		szLabelName = pLabel->getMenuLabel();

	if (!szLabelName || !*szLabelName)
		return data;	// which will be two nulls now

	static char accelbuf[32];
	{
		// see if this has an associated keybinding
		const char * szMethodName = pAction->getMethodName();

		if (szMethodName)
		{
			const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
			UT_ASSERT(pEMC);

			EV_EditMethod * pEM = pEMC->findEditMethodByName(szMethodName);
			UT_ASSERT(pEM);						// make sure it's bound to something

			const EV_EditEventMapper * pEEM = pFrame->getEditEventMapper();
			UT_ASSERT(pEEM);

			const char * string = pEEM->getShortcutFor(pEM);
			if (string && *string)
				strcpy(accelbuf, string);
			else
				// zero it out for this round
				*accelbuf = 0;
		}
	}

	// set shortcut mnemonic, if any
	if (*accelbuf)
		data[1] = accelbuf;
	
	if (!pAction->raisesDialog())
	{
		data[0] = szLabelName;
		return data;
	}

	// append "..." to menu item if it raises a dialog
	static char buf[128];
	memset(buf,0,NrElements(buf));
	strncpy(buf,szLabelName,NrElements(buf)-4);
	strcat(buf,"...");

	data[0] = buf;
	
	return data;
}


