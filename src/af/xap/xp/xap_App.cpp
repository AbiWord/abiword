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
#include "gr_Image.h"
#include "xap_Frame.h"
#include "xap_EditMethods.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_LoadBindings.h"

/*****************************************************************/

AP_Clipboard* XAP_App::_pClipboard = NULL;

XAP_App::XAP_App(XAP_Args * pArgs, const char * szAppName) : m_hashClones(5)
{
	UT_ASSERT(szAppName && *szAppName);

	m_pArgs = pArgs;
	m_szAppName = szAppName;
	m_szAbiSuiteLibDir = NULL;
	m_pEMC = NULL;
	m_pBindingSet = NULL;
	m_pMenuActionSet = NULL;
	m_pToolbarActionSet = NULL;
}

XAP_App::~XAP_App(void)
{
	// run thru and destroy all frames on our window list.
	UT_VECTOR_PURGEALL(XAP_Frame *, m_vecFrames);

	FREEP(m_szAbiSuiteLibDir);
	DELETEP(m_pEMC);
	DELETEP(m_pBindingSet);
	DELETEP(m_pMenuActionSet);
	DELETEP(m_pToolbarActionSet);
}

UT_Bool XAP_App::initialize(void)
{
	// create application-wide resources that
	// are shared by everything.

#if 0 
	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_ASSERT(m_pBindingSet);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);
#endif
	// TODO use m_pArgs->{argc,argv} to process any command-line
	// TODO options that we need.

	return UT_TRUE;
}

const char * XAP_App::getApplicationTitleForTitleBar(void) const
{
	static char _title[512];

	// return a string that the platform-specific code
	// can copy to the title bar of a window.

	sprintf(_title, "%s (www.abisource.com)", m_szAppName);

	return _title;
}

const char * XAP_App::getApplicationName(void) const
{
	// return a string that the platform-specific code
	// can use as a class name for various window-manager-like
	// operations.
	return m_szAppName;
}

EV_EditMethodContainer * XAP_App::getEditMethodContainer(void) const
{
	return m_pEMC;
}

EV_EditBindingMap * XAP_App::getBindingMap(const char * szName)
{
	UT_ASSERT(m_pBindingSet);
	return m_pBindingSet->getMap(szName);
}

const EV_Menu_ActionSet * XAP_App::getMenuActionSet(void) const
{
	return m_pMenuActionSet;
}

const EV_Toolbar_ActionSet * XAP_App::getToolbarActionSet(void) const
{
	return m_pToolbarActionSet;
}

UT_Bool XAP_App::rememberFrame(XAP_Frame * pFrame, XAP_Frame * pCloneOf)
{
	UT_ASSERT(pFrame);

	// add this frame to our window list
	m_vecFrames.addItem(pFrame);

	// TODO: error-check the following for mem failures
	if (pCloneOf)
	{
		// locate vector of this frame's clones
		UT_HashEntry* pEntry = m_hashClones.findEntry(pCloneOf->getViewKey());

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
			XAP_Frame * f = (XAP_Frame *) pvClones->getNthItem(j);
			UT_ASSERT(f);

			f->setViewNumber(j+1);

			if (f != pFrame)
				f->updateTitle();
		}
	}
	
	// TODO do something here...
	return UT_TRUE;
}

UT_Bool XAP_App::forgetFrame(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	if (pFrame->getViewNumber() > 0)
	{
		// locate vector of this frame's clones
		UT_HashEntry* pEntry = m_hashClones.findEntry(pFrame->getViewKey());
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
			XAP_Frame * f = NULL;

			if (count == 1)
			{
				// remaining clone is now a singleton
				f = (XAP_Frame *) pvClones->getNthItem(count-1);
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
					f = (XAP_Frame *) pvClones->getNthItem(j);
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

UT_Bool XAP_App::forgetClones(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	if (pFrame->getViewNumber() == 0)
	{
		return forgetFrame(pFrame);
	}

	UT_Vector vClones;
	getClones(&vClones, pFrame);
	
	for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
	{
		XAP_Frame * f = (XAP_Frame *) vClones.getNthItem(i);
		forgetFrame(f);
	}

	return UT_TRUE;
}

UT_Bool XAP_App::getClones(UT_Vector *pvClonesCopy, XAP_Frame * pFrame)
{
	UT_ASSERT(pvClonesCopy);
	UT_ASSERT(pFrame);
	UT_ASSERT(pFrame->getViewNumber() > 0);

	// locate vector of this frame's clones
	UT_HashEntry* pEntry = m_hashClones.findEntry(pFrame->getViewKey());
	UT_ASSERT(pEntry);

	UT_Vector * pvClones = (UT_Vector *) pEntry->pData;
	UT_ASSERT(pvClones);

	return pvClonesCopy->copy(pvClones);
}

UT_Bool XAP_App::updateClones(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(pFrame->getViewNumber() > 0);

	// locate vector of this frame's clones
	UT_HashEntry* pEntry = m_hashClones.findEntry(pFrame->getViewKey());
	UT_ASSERT(pEntry);

	if (pEntry)
	{
		UT_Vector * pvClones = (UT_Vector *) pEntry->pData;
		UT_ASSERT(pvClones);

		UT_uint32 count = pvClones->getItemCount();
		UT_ASSERT(count > 0);
		XAP_Frame * f = NULL;

		for (UT_uint32 j=0; j<count; j++)
		{
			f = (XAP_Frame *) pvClones->getNthItem(j);
			UT_ASSERT(f);

			f->updateTitle();
		}
	}

	return UT_TRUE;
}

UT_uint32 XAP_App::getFrameCount(void) const
{
	return m_vecFrames.getItemCount();
}

XAP_Frame * XAP_App::getFrame(UT_uint32 ndx) const
{
	XAP_Frame * pFrame = NULL;
	
	if (ndx < m_vecFrames.getItemCount())
	{
		pFrame = (XAP_Frame *) m_vecFrames.getNthItem(ndx);
	}

	return pFrame;
}
	
UT_sint32 XAP_App::findFrame(XAP_Frame * pFrame)
{
	return m_vecFrames.findItem(pFrame);
}
	
UT_sint32 XAP_App::findFrame(const char * szFilename)
{
	if (!szFilename || !*szFilename)
		return -1;

	for (UT_uint32 i=0; i<getFrameCount(); i++)
	{
		XAP_Frame * f = getFrame(i);
		UT_ASSERT(f);
		const char * s = f->getFilename();

		if (s && *s && (0 == UT_stricmp(szFilename, s)))
		{
			return (UT_sint32) i;
		}
	}

	return -1;
}

AP_Clipboard* XAP_App::getClipboard(void)
{
	return _pClipboard;
}


void XAP_App::_setAbiSuiteLibDir(const char * sz)
{
	FREEP(m_szAbiSuiteLibDir);
	UT_cloneString((char *&)m_szAbiSuiteLibDir,sz);
}

const char * XAP_App::getAbiSuiteLibDir(void) const
{
	return m_szAbiSuiteLibDir;
}


