/* AbiSource Application Framework
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

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <stdio.h>	// for sprintf

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Actions.h"
#include "ev_Toolbar_Actions.h"
#include "xap_App.h"
#include "xap_Args.h"
#include "xap_Clipboard.h"
#include "xap_Frame.h"
#include "xap_EditMethods.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"


#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

AP_Clipboard* AP_App::_pClipboard = NULL;

AP_App::AP_App(AP_Args * pArgs, const char * szAppName) : m_hashClones(5)
{
	UT_ASSERT(szAppName && *szAppName);

	m_pArgs = pArgs;
	m_szAppName = szAppName;
	m_pEMC = NULL;
	m_pMenuActionSet = NULL;
	m_pToolbarActionSet = NULL;
}

AP_App::~AP_App(void)
{
	// run thru and destroy all frames on our window list.
	UT_VECTOR_PURGEALL(AP_Frame *, m_vecFrames);

	DELETEP(m_pEMC);
	DELETEP(m_pMenuActionSet);
	DELETEP(m_pToolbarActionSet);
	
#ifdef ABI_OPT_JS
	if (m_pJSInterp)
	{
		js_destroy_interp (m_pJSInterp);
	}
#endif /* ABI_OPT_JS */
}

UT_Bool AP_App::initialize(void)
{
	// create application-wide resources that
	// are shared by everything.

	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);

	// TODO use m_pArgs->{argc,argv} to process any command-line
	// TODO options that we need.

#ifdef ABI_OPT_JS
	// Create our app-global JavaScript interpreter
	js_init_default_options (&m_JSOptions);

	m_JSOptions.verbose = 2; // TODO change this later.
	m_JSOptions.s_context = this;

	m_pJSInterp = js_create_interp (&m_JSOptions);

	// TODO initialize the interp with our object model.

#endif /* ABI_OPT_JS */

	return UT_TRUE;
}

#ifdef ABI_OPT_JS
JSInterpPtr AP_App::getInterp(void) const
{
	return m_pJSInterp;
}
#endif /* ABI_OPT_JS */

const char * AP_App::getApplicationTitleForTitleBar(void) const
{
	static char _title[512];

	// return a string that the platform-specific code
	// can copy to the title bar of a window.

	/*
		TODO the format of the string below is not necessarily
		what we will want to use for end-user shipping builds.
		For example, we might not want to include the build ID.
		We should probably make this conditional based on whether
		the build is debug or not.
	*/

	sprintf(_title, "%s Personal %s (%s)", m_szAppName, s_szBuild_Version, s_szBuild_ID);

	return _title;
}

const char * AP_App::getApplicationName(void) const
{
	// return a string that the platform-specific code
	// can use as a class name for various window-manager-like
	// operations.
	return m_szAppName;
}

EV_EditMethodContainer * AP_App::getEditMethodContainer(void) const
{
	return m_pEMC;
}

const EV_Menu_ActionSet * AP_App::getMenuActionSet(void) const
{
	return m_pMenuActionSet;
}

const EV_Toolbar_ActionSet * AP_App::getToolbarActionSet(void) const
{
	return m_pToolbarActionSet;
}

UT_Bool AP_App::rememberFrame(AP_Frame * pFrame, AP_Frame * pCloneOf)
{
	UT_ASSERT(pFrame);

	// add this frame to our window list
	m_vecFrames.addItem(pFrame);

	// TODO: error-check the following for mem failures
	if (pCloneOf)
	{
		// locate vector of this frame's clones
		UT_HashTable::UT_HashEntry* pEntry = m_hashClones.findEntry(pCloneOf->getViewKey());

		UT_Vector * pvClones = NULL;

		if (pEntry)
		{
			// hash table entry already exists
			pvClones = (UT_Vector *) pEntry->pData;

			if (!pvClones)
			{
				// nothing there, so create a new one
				pvClones = new UT_Vector();
				UT_ASSERT(pvClones);

				pvClones->addItem(pCloneOf);

				// reuse this slot
				m_hashClones.setEntry(pEntry, NULL, pvClones);
			}
		}
		else
		{
			// create a new one
			pvClones = new UT_Vector();
			UT_ASSERT(pvClones);

			pvClones->addItem(pCloneOf);

			// add it to the hash table
			m_hashClones.addEntry(pCloneOf->getViewKey(), NULL, pvClones);
		}

		pvClones->addItem(pFrame);

		// notify all clones of their new view numbers
		for (UT_uint32 j=0; j<pvClones->getItemCount(); j++)
		{
			AP_Frame * f = (AP_Frame *) pvClones->getNthItem(j);
			UT_ASSERT(f);

			f->setViewNumber(j+1);

			if (f != pFrame)
				f->updateTitle();
		}
	}
	
	// TODO do something here...
	return UT_TRUE;
}

UT_Bool AP_App::forgetFrame(AP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	if (pFrame->getViewNumber() > 0)
	{
		// locate vector of this frame's clones
		UT_HashTable::UT_HashEntry* pEntry = m_hashClones.findEntry(pFrame->getViewKey());
		UT_ASSERT(pEntry);

		if (pEntry)
		{
			UT_Vector * pvClones = (UT_Vector *) pEntry->pData;
			UT_ASSERT(pvClones);

			// remove this frame from the vector
			UT_sint32 i = pvClones->findItem(pFrame);
			UT_ASSERT(i >= 0);

			if (i >= 0)
			{
				pvClones->deleteNthItem(i);
			}

			// see how many clones are left
			UT_uint32 count = pvClones->getItemCount();
			UT_ASSERT(count > 0);
			AP_Frame * f = NULL;

			if (count == 1)
			{
				// remaining clone is now a singleton
				f = (AP_Frame *) pvClones->getNthItem(count-1);
				UT_ASSERT(f);

				f->setViewNumber(0);
				f->updateTitle();

				// remove this entry from hashtable
				m_hashClones.setEntry(pEntry, NULL, NULL);
				delete pvClones;
			}
			else
			{
				// notify remaining clones of their new view numbers
				for (UT_uint32 j=0; j<count; j++)
				{
					f = (AP_Frame *) pvClones->getNthItem(j);
					UT_ASSERT(f);

					f->setViewNumber(j+1);
					f->updateTitle();
				}
			}
		}
	}

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

UT_Bool AP_App::updateClones(AP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(pFrame->getViewNumber() > 0);

	// locate vector of this frame's clones
	UT_HashTable::UT_HashEntry* pEntry = m_hashClones.findEntry(pFrame->getViewKey());
	UT_ASSERT(pEntry);

	if (pEntry)
	{
		UT_Vector * pvClones = (UT_Vector *) pEntry->pData;
		UT_ASSERT(pvClones);

		UT_uint32 count = pvClones->getItemCount();
		UT_ASSERT(count > 0);
		AP_Frame * f = NULL;

		for (UT_uint32 j=0; j<count; j++)
		{
			f = (AP_Frame *) pvClones->getNthItem(j);
			UT_ASSERT(f);

			f->updateTitle();
		}
	}

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
	
UT_sint32 AP_App::findFrame(const char * szFilename)
{
	if (!szFilename || !*szFilename)
		return -1;

	for (UT_uint32 i=0; i<getFrameCount(); i++)
	{
		AP_Frame * f = getFrame(i);
		UT_ASSERT(f);
		const char * s = f->getFilename();

		if (s && *s && (0 == UT_stricmp(szFilename, s)))
		{
			return (UT_sint32) i;
		}
	}

	return -1;
}

AP_Clipboard* AP_App::getClipboard(void)
{
	return _pClipboard;
}
