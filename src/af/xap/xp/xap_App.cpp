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
 
#include <js.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Actions.h"
#include "ap_App.h"
#include "ap_Frame.h"
#include "ap_EditMethods.h"
#include "ap_Menu_ActionSet.h"


#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

AP_App::AP_App(void)
{
	m_pEMC = NULL;
	m_pMenuActionSet = NULL;
}

AP_App::~AP_App(void)
{
	// run thru and destroy all frames on our window list.
	UT_VECTOR_PURGEALL(AP_Frame, m_vecFrames);

	// TODO: add another method to give them fair warning first

	DELETEP(m_pEMC);
	DELETEP(m_pMenuActionSet);

	if (m_pJSInterp)
		js_destroy_interp (m_pJSInterp);
}

UT_Bool AP_App::initialize(int * /*pArgc*/, char *** /*pArgv*/)
{
	// create application-wide resources that
	// are shared by everything.

	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	// TODO use argc,argv to process any command-line
	// TODO options that we need.

	// Create our app-global JavaScript interpreter
	js_init_default_options (&m_JSOptions);

	m_JSOptions.verbose = 2; // TODO change this later.
	m_JSOptions.s_context = this;

	m_pJSInterp = js_create_interp (&m_JSOptions);

	// TODO initialize the interp with our object model.

	return UT_TRUE;
}

JSInterpPtr AP_App::getInterp(void) const
{
	return m_pJSInterp;
}

const char * AP_App::getApplicationTitleForTitleBar(void) const
{
	// return a string that the platform-specific code
	// can copy to the title bar of a window.

	// TODO fix the following...
	return "AbiWord Personal 0.1.4";
}

const char * AP_App::getApplicationName(void) const
{
	// return a string that the platform-specific code
	// can use as a class name for various window-manager-like
	// operations.

	// TODO fix the following...
	return "AbiWord";
}

EV_EditMethodContainer * AP_App::getEditMethodContainer(void) const
{
	return m_pEMC;
}

const EV_Menu_ActionSet * AP_App::getMenuActionSet(void) const
{
	return m_pMenuActionSet;
}

UT_Bool AP_App::rememberFrame(AP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	// add this frame to our window list
	m_vecFrames.addItem(pFrame);
	
	// TODO do something here...
	return UT_TRUE;
}

UT_Bool AP_App::forgetFrame(AP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	// remove this frame from our window list
	UT_sint32 ndx = m_vecFrames.findItem(pFrame);
	UT_ASSERT(ndx >= 0);

	if (ndx >= 0)
	{
		m_vecFrames.deleteNthItem(ndx);
	}

	// TODO do something here...
	return UT_TRUE;
}

UT_uint32 AP_App::getFrameCount(void) const
{
	return m_vecFrames.getItemCount();
}

AP_Frame * AP_App::getFrame(UT_uint32 ndx) const
{
	AP_Frame * pFrame = NULL;
	
	if (ndx < m_vecFrames.getItemCount())
	{
		pFrame = (AP_Frame *) m_vecFrames.getNthItem(ndx);
	}

	return pFrame;
}
	
UT_sint32 AP_App::findFrame(AP_Frame * pFrame)
{
	return m_vecFrames.findItem(pFrame);
}
