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
 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "xap_ViewListener.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "xap_Menu_Layouts.h"
#include "xap_Menu_LabelSet.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "xap_Scrollbar_ViewListener.h"
#include "xap_Strings.h"

/*****************************************************************/

XAP_Frame::XAP_Frame(XAP_App * app)
{
	m_app = app;

	m_pData = NULL;
	m_pDoc = NULL;
	m_pView = NULL;
	m_pViewListener = NULL;
	m_pScrollObj = NULL;
	m_szMenuLayoutName = NULL;
	m_szMenuLabelSetName = NULL;
	m_szToolbarLabelSetName = NULL;
	m_szToolbarAppearance = NULL;
	m_iUntitled = 0;
	m_nView = 0;
	m_pScrollbarViewListener = NULL;
	m_pInputModes = NULL;
	m_app->rememberFrame(this);
	memset(m_szTitle,0,sizeof(m_szTitle));
	memset(m_szNonDecoratedTitle,0,sizeof(m_szNonDecoratedTitle));

	m_lid = (AV_ListenerId) -1;
}

XAP_Frame::XAP_Frame(XAP_Frame * f)
{
	// only clone a few things
	m_app = f->m_app;
	m_pDoc = f->m_pDoc;
	m_iUntitled = f->m_iUntitled;

	// everything else gets recreated
	m_pData = NULL;
	m_pView = NULL;
	m_pViewListener = NULL;
	m_pScrollObj = NULL;
	m_szMenuLayoutName = NULL;
	m_szMenuLabelSetName = NULL;
	m_szToolbarLabelSetName = NULL;
	m_szToolbarAppearance = NULL;
	m_nView = 0;
	m_pScrollbarViewListener = NULL;
	m_pInputModes = NULL;
	
	m_app->rememberFrame(this, f);
	memset(m_szTitle,0,sizeof(m_szTitle));
	memset(m_szNonDecoratedTitle,0,sizeof(m_szNonDecoratedTitle));

	m_lid = (AV_ListenerId) -1;
}

XAP_Frame::~XAP_Frame(void)
{
	// only delete the things that we created...

	if (m_pView)
		m_pView->removeListener(m_lid);

	DELETEP(m_pView);
	DELETEP(m_pViewListener);

	if (m_nView==0)
		DELETEP(m_pDoc);

	DELETEP(m_pScrollObj);
	DELETEP(m_pInputModes);

	DELETEP(m_pScrollbarViewListener);

	UT_VECTOR_PURGEALL(char *,m_vecToolbarLayoutNames);
	
	FREEP(m_szMenuLayoutName);
	FREEP(m_szMenuLabelSetName);
	FREEP(m_szToolbarLabelSetName);
	FREEP(m_szToolbarAppearance);
}

/*****************************************************************/
// sequence number tracker for untitled documents

int XAP_Frame::s_iUntitled = 0;	
int XAP_Frame::_getNextUntitledNumber(void)
{
	return ++s_iUntitled;
}

/*****************************************************************/

UT_Bool XAP_Frame::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
							  const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
							  const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
							  const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
							  const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue)
{
	XAP_App * pApp = getApp();

	//////////////////////////////////////////////////////////////////
	// choose which set of key- and mouse-bindings to load
	// create a EventMapper state-machine to process our events
	//////////////////////////////////////////////////////////////////

	const char * szBindings = NULL;
	EV_EditBindingMap * pBindingMap = NULL;

	if ((pApp->getPrefsValue(szKeyBindingsKey,&szBindings)) && (szBindings) && (*szBindings))
		pBindingMap = m_app->getBindingMap(szBindings);
	if (!pBindingMap)
		pBindingMap = m_app->getBindingMap(szKeyBindingsDefaultValue);
	UT_ASSERT(pBindingMap);

	if (!m_pInputModes)
	{
		m_pInputModes = new XAP_InputModes();
		UT_ASSERT(m_pInputModes);
	}
	UT_Bool bResult = m_pInputModes->createInputMode(szBindings,pBindingMap);
	UT_ASSERT(bResult);
	UT_Bool bResult2 = m_pInputModes->setCurrentMap(szBindings);
	UT_ASSERT(bResult2);
	
	//////////////////////////////////////////////////////////////////
	// select which menu bar we should use
	//////////////////////////////////////////////////////////////////

	const char * szMenuLayoutName = NULL;
	if ((pApp->getPrefsValue(szMenuLayoutKey,&szMenuLayoutName)) && (szMenuLayoutName) && (*szMenuLayoutName))
		;
	else
		szMenuLayoutName = szMenuLayoutDefaultValue;
	UT_cloneString((char *&)m_szMenuLayoutName,szMenuLayoutName);
	
	//////////////////////////////////////////////////////////////////
	// select language for menu labels
	//////////////////////////////////////////////////////////////////

	const char * szMenuLabelSetName = NULL;
	if ((pApp->getPrefsValue(szMenuLabelSetKey,&szMenuLabelSetName)) && (szMenuLabelSetName) && (*szMenuLabelSetName))
		;
	else
		szMenuLabelSetName = szMenuLabelSetDefaultValue;
	UT_cloneString((char *&)m_szMenuLabelSetName,szMenuLabelSetName);
	
	//////////////////////////////////////////////////////////////////
	// select which toolbars we should display
	//////////////////////////////////////////////////////////////////

	const char * szToolbarLayouts = NULL;
	if ((pApp->getPrefsValue(szToolbarLayoutsKey,&szToolbarLayouts)) && (szToolbarLayouts) && (*szToolbarLayouts))
		;
	else
		szToolbarLayouts = szToolbarLayoutsDefaultValue;

	// take space-delimited list and call addItem() for each name in the list.
	
	{
		char * szTemp;
		UT_cloneString(szTemp,szToolbarLayouts);
		UT_ASSERT(szTemp);
		for (char * p=strtok(szTemp," "); (p); p=strtok(NULL," "))
		{
			char * szTempName;
			UT_cloneString(szTempName,p);
			m_vecToolbarLayoutNames.addItem(szTempName);
		}
		free(szTemp);
	}
	
	//////////////////////////////////////////////////////////////////
	// select language for the toolbar labels.
	// i'm not sure if it would ever make sense to
	// deviate from what we set the menus to, but
	// we can if we have to.
	// all toolbars will have the same language.
	//////////////////////////////////////////////////////////////////

	const char * szToolbarLabelSetName = NULL;
	if ((pApp->getPrefsValue(szToolbarLabelSetKey,&szToolbarLabelSetName)) && (szToolbarLabelSetName) && (*szToolbarLabelSetName))
		;
	else
		szToolbarLabelSetName = szToolbarLabelSetDefaultValue;
	UT_cloneString((char *&)m_szToolbarLabelSetName,szToolbarLabelSetName);
	
	//////////////////////////////////////////////////////////////////
	// select the appearance of the toolbar buttons
	//////////////////////////////////////////////////////////////////

	const char * szToolbarAppearance = NULL;
	if ((pApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szToolbarAppearance)) && (szToolbarAppearance) && (*szToolbarAppearance))
		;
	else
		szToolbarAppearance = XAP_PREF_DEFAULT_ToolbarAppearance;
	UT_cloneString((char *&)m_szToolbarAppearance,szToolbarAppearance);
	
	//////////////////////////////////////////////////////////////////
	// ... add other stuff here ...
	//////////////////////////////////////////////////////////////////

	return UT_TRUE;
}

EV_EditEventMapper * XAP_Frame::getEditEventMapper(void) const
{
	UT_ASSERT(m_pInputModes);
	return m_pInputModes->getCurrentMap();
}

UT_sint32 XAP_Frame::setInputMode(const char * szName)
{
	UT_ASSERT(m_pInputModes);
	const char * szCurrentName = m_pInputModes->getCurrentMapName();
	if (UT_stricmp(szName,szCurrentName) == 0)
		return -1;					// already set, no change required

	EV_EditEventMapper * p = m_pInputModes->getMapByName(szName);
	if (!p)
	{
		// map not previously loaded -- we need to install it first

		EV_EditBindingMap * pBindingMap = m_app->getBindingMap(szName);
		UT_ASSERT(pBindingMap);
		UT_Bool bResult = m_pInputModes->createInputMode(szName,pBindingMap);
		UT_ASSERT(bResult);
	}
	
	// note: derrived classes will need to update keyboard
	// note: and mouse after we return.

	UT_DEBUGMSG(("Setting InputMode to [%s] for the current window.\n",szName));
	
	UT_Bool bStatus = m_pInputModes->setCurrentMap(szName);
	getCurrentView()->notifyListeners(AV_CHG_INPUTMODE);

	return (bStatus);
}

const char * XAP_Frame::getInputMode(void) const
{
	return m_pInputModes->getCurrentMapName();
}

XAP_App * XAP_Frame::getApp(void) const
{
	return m_app;
}

AV_View * XAP_Frame::getCurrentView(void) const
{
	// TODO i called this ...Current... in anticipation of having
	// TODO more than one view (think splitter windows) in this
	// TODO frame.  but i'm just guessing right now....
	
	return m_pView;
}

const char * XAP_Frame::getFilename(void) const
{
	return m_pDoc->getFilename();
}

UT_Bool XAP_Frame::isDirty(void) const
{
	return m_pDoc->isDirty();
}

void XAP_Frame::setViewNumber(UT_uint32 n)
{
	m_nView = n;
}

UT_uint32 XAP_Frame::getViewNumber(void) const
{
	return m_nView;
}

const char * XAP_Frame::getViewKey(void) const
{
	/*
		We want a string key which uniquely identifies a AD_Document instance, 
		so that we can match up top-level views on the same document.  
		
		We can't use the filename, since it might not exist (untitled43) and
		is likely to change when the document is saved.

		So, we just use the AD_Document pointer.  :-)
	*/

	// The buffer must be wide enough to hold character representation
	// of a pointer on any platform.  For Intel that would be 32-bits, which
	// would be 8 chars plus a null.  Double that for 64.
	// Why "+3"?  For the "0x" and the null. 
	static char buf[(sizeof(void *) * 2) + 3];

	sprintf(buf, "%p", m_pDoc);

	return buf;
}

const char * XAP_Frame::getTitle(int len) const
{
	// TODO: chop down to fit desired size?
	UT_ASSERT((int)strlen(m_szTitle) < len);
	return m_szTitle;
}

const char * XAP_Frame::getTempNameFromTitle(void) const
{
	// extract a filename or pathname from the title.
	// strip off all of the title's window decorations (the ":1 *").

	return m_szNonDecoratedTitle;
}
	
UT_Bool XAP_Frame::updateTitle()
{
	/*
		The document title for this window has changed, so we need to:

		1. Update m_szTitle accordingly.	(happens here)
		2. Update the window title.			(happens in subclass)

		Note that we don't need to update the contents of the Window menu, 
		because that happens dynamically at menu pop-up time.  
	*/

	const char* szName = m_pDoc->getFilename();

	if (szName && *szName)
	{
		UT_ASSERT(strlen(szName) < 245); // TODO need #define for this number
		strcpy(m_szTitle, szName); 
	}
	else
	{
		UT_ASSERT(m_iUntitled);
		const XAP_StringSet * pSS = m_app->getStringSet();
		
		sprintf(m_szTitle, pSS->getValue(XAP_STRING_ID_UntitledDocument), m_iUntitled);
	}

	strcpy(m_szNonDecoratedTitle, m_szTitle);
	
	if (m_nView)
	{
		// multiple top-level views, so append : & view number
		char buf[6];
		UT_ASSERT(m_nView < 10000);
		sprintf(buf, ":%d", m_nView);
		strcat(m_szTitle, buf);
	}

	if (m_pDoc->isDirty())
	{
		// append " *"
		strcat(m_szTitle, " *");
	}

	return UT_TRUE;
}

void XAP_Frame::setZoomPercentage(UT_uint32 /* iZoom */)
{
	// default does nothing.  see subclasses for override
}

UT_uint32 XAP_Frame::getZoomPercentage(void)
{
	return 100;	// default implementation
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_InputModes::XAP_InputModes(void)
{
	m_indexCurrentEventMap = 0;
}

XAP_InputModes::~XAP_InputModes(void)
{
	UT_sint32 kLimit = m_vecEventMaps.getItemCount();
	UT_sint32 jLimit = m_vecNames.getItemCount();
	UT_ASSERT(kLimit == jLimit);

	UT_VECTOR_PURGEALL(EV_EditEventMapper *, m_vecEventMaps);
	UT_VECTOR_FREEALL(char *, m_vecNames);
}

UT_Bool XAP_InputModes::createInputMode(const char * szName,
										EV_EditBindingMap * pBindingMap)
{
	UT_ASSERT(szName && *szName);
	UT_ASSERT(pBindingMap);
	
	char * szDup = NULL;
	EV_EditEventMapper * pEEM = NULL;

	UT_cloneString(szDup,szName);
	UT_ASSERT(szDup);
	
	pEEM = new EV_EditEventMapper(pBindingMap);
	UT_ASSERT(pEEM);

	UT_Bool b1 = (m_vecEventMaps.addItem(pEEM) == 0);
	UT_Bool b2 = (m_vecNames.addItem(szDup) == 0);
	UT_ASSERT(b1 && b2);

	return UT_TRUE;
}

UT_Bool XAP_InputModes::setCurrentMap(const char * szName)
{
	UT_uint32 kLimit = m_vecNames.getItemCount();
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
		if (UT_stricmp(szName,(const char *)m_vecNames.getNthItem(k)) == 0)
		{
			m_indexCurrentEventMap = k;
			return UT_TRUE;
		}

	return UT_FALSE;
}

EV_EditEventMapper * XAP_InputModes::getCurrentMap(void) const
{
	return (EV_EditEventMapper *)m_vecEventMaps.getNthItem(m_indexCurrentEventMap);
}

const char * XAP_InputModes::getCurrentMapName(void) const
{
	return (const char *)m_vecNames.getNthItem(m_indexCurrentEventMap);
}

EV_EditEventMapper * XAP_InputModes::getMapByName(const char * szName) const
{
	UT_uint32 kLimit = m_vecNames.getItemCount();
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
		if (UT_stricmp(szName,(const char *)m_vecNames.getNthItem(k)) == 0)
			return (EV_EditEventMapper *)m_vecEventMaps.getNthItem(k);

	return NULL;
}
