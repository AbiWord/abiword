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
#include "ev_Menu.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "xap_Menu_Layouts.h"
#include "xap_Menu_LabelSet.h"


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

EV_Menu::~EV_Menu(void)
{
	DELETEP(m_pMenuLayout);
	DELETEP(m_pMenuLabelSet);
}

const EV_Menu_Layout * EV_Menu::getMenuLayout(void) const
{
	return m_pMenuLayout;
}

const EV_Menu_LabelSet * EV_Menu::getMenuLabelSet(void) const
{
	return m_pMenuLabelSet;
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


