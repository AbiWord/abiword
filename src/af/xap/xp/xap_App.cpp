/* AbiWord
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
 

#include "ut_types.h"
#include "ut_assert.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Actions.h"
#include "ap_Ap.h"
#include "ap_Frame.h"
#include "ap_EditMethods.h"
#include "ap_Menu_ActionSet.h"


#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

AP_Ap::AP_Ap(void)
{
	m_pEMC = NULL;
	m_pMenuActionSet = NULL;
}

AP_Ap::~AP_Ap(void)
{
	// run thru and destroy all frames on our window list.
	UT_VECTOR_PURGEALL(AP_Frame, m_vecFrames);

	// TODO: add another method to give them fair warning first

	DELETEP(m_pEMC);
	DELETEP(m_pMenuActionSet);
}

UT_Bool AP_Ap::initialize(int * pArgc, char *** pArgv)
{
	// create application-wide resources that
	// are shared by everything.

	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	// TODO use argc,argv to process any command-line
	// TODO options that we need.

	return UT_TRUE;
}

const char * AP_Ap::getApplicationTitleForTitleBar(void) const
{
	// return a string that the platform-specific code
	// can copy to the title bar of a window.

	// TODO fix the following...
	return "AbiWord Personal 0.1.4";
}

const char * AP_Ap::getApplicationName(void) const
{
	// return a string that the platform-specific code
	// can use as a class name for various window-manager-like
	// operations.

	// TODO fix the following...
	return "AbiWord";
}

EV_EditMethodContainer * AP_Ap::getEditMethodContainer(void) const
{
	return m_pEMC;
}

const EV_Menu_ActionSet * AP_Ap::getMenuActionSet(void) const
{
	return m_pMenuActionSet;
}

UT_Bool AP_Ap::rememberFrame(AP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	// add this frame to our window list
	m_vecFrames.addItem(pFrame);
	
	// TODO do something here...
	return UT_TRUE;
}

UT_Bool AP_Ap::forgetFrame(AP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	// remove this frame from our window list
	UT_sint32 ndx = m_vecFrames.findItem(pFrame);
	UT_ASSERT(ndx >= 0);

	if (ndx > 0)
	{
		m_vecFrames.deleteNthItem(ndx);
	}

	// TODO do something here...
	return UT_TRUE;
}
