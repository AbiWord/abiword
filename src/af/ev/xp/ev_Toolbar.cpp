/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_Toolbar.h"
#include "ev_Toolbar_Labels.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "xap_Toolbar_Layouts.h"
#include "xap_Toolbar_LabelSet.h"
#include "xap_App.h"

/*****************************************************************/

EV_Toolbar::EV_Toolbar(EV_EditMethodContainer * pEMC,
					   const char * szToolbarLayoutName,
					   const char * szToolbarLabelSetName)
	: m_bHidden(false)
{
	UT_ASSERT(pEMC);

	m_pEMC = pEMC;

	m_pToolbarLayout = XAP_App::getApp()->getToolbarFactory()->CreateToolbarLayout(szToolbarLayoutName);
	UT_ASSERT(m_pToolbarLayout);

	m_pToolbarLabelSet = AP_CreateToolbarLabelSet(szToolbarLabelSetName);
	UT_ASSERT(m_pToolbarLabelSet);
}

EV_Toolbar::~EV_Toolbar(void)
{
	DELETEP(m_pToolbarLayout);
	DELETEP(m_pToolbarLabelSet);
}

const EV_Toolbar_Layout * EV_Toolbar::getToolbarLayout(void) const
{
	return m_pToolbarLayout;
}

const EV_Toolbar_LabelSet * EV_Toolbar::getToolbarLabelSet(void) const
{
	return m_pToolbarLabelSet;
}

bool EV_Toolbar::invokeToolbarMethod(AV_View * pView,
									 EV_EditMethod * pEM,
									 const UT_UCSChar * pData,
									 UT_uint32 dataLength)
{
	UT_ASSERT(pView);
	UT_return_val_if_fail(pEM, false);

	//UT_DEBUGMSG(("invokeToolbarMethod: %s\n",pEM->getName()));

	EV_EditMethodType t = pEM->getType();

	if (((t & EV_EMT_REQUIREDATA) != 0) && (!pData || !dataLength))
	{
		// This method requires character data and the caller did not provide any.
		UT_DEBUGMSG(("    invoke aborted due to lack of data\n"));
		return false;
	}

	EV_EditMethodCallData emcd(pData,dataLength);
	pEM->Fn(pView,&emcd);

	return true;
	
}



