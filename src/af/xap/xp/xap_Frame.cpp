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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_ViewListener.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "xap_LoadBindings.h"
#include "xap_Menu_Layouts.h"
#include "xap_Menu_LabelSet.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "xap_Scrollbar_ViewListener.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

AP_Frame::AP_Frame(AP_App * app)
{
	m_app = app;

	m_pData = NULL;
	m_pDoc = NULL;
	m_pView = NULL;
	m_pViewListener = NULL;
	m_pScrollObj = NULL;
	m_pEBM = NULL;
	m_pEEM = NULL;
	m_szMenuLayoutName = NULL;
	m_szMenuLabelSetName = NULL;
	m_iUntitled = 0;
	m_nView = 0;
	m_pScrollbarViewListener = NULL;
	
	m_app->rememberFrame(this);
	memset(m_szTitle,0,sizeof(m_szTitle));
	memset(m_szNonDecoratedTitle,0,sizeof(m_szNonDecoratedTitle));
}

AP_Frame::AP_Frame(AP_Frame * f)
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
	m_pEBM = NULL;
	m_pEEM = NULL;
	m_szMenuLayoutName = NULL;
	m_szMenuLabelSetName = NULL;
	m_nView = 0;
	m_pScrollbarViewListener = NULL;
	
	m_app->rememberFrame(this, f);
	memset(m_szTitle,0,sizeof(m_szTitle));
	memset(m_szNonDecoratedTitle,0,sizeof(m_szNonDecoratedTitle));
}

AP_Frame::~AP_Frame(void)
{
	// only delete the things that we created...

	if (m_pView)
		m_pView->removeListener(m_lid);

	DELETEP(m_pView);
	DELETEP(m_pViewListener);

	killFrameData();

	if (m_nView==0)
		DELETEP(m_pDoc);

	DELETEP(m_pScrollObj);
	DELETEP(m_pEBM);
	DELETEP(m_pEEM);

	DELETEP(m_pScrollbarViewListener);
}

// sequence number tracker for untitled documents
int AP_Frame::s_iUntitled = 0;	

UT_Bool AP_Frame::initialize(void)
{
	UT_Bool bResult;

	if (!initFrameData())
		return UT_FALSE;

	// choose which set of key- and mouse-bindings to load
	
	char * szBindings = "default";
	// TODO override szBindings from m_app->m_pArgs->{argc,argv}.
	bResult = AP_LoadBindings(szBindings,m_app->getEditMethodContainer(),&m_pEBM);
	UT_ASSERT(bResult && (m_pEBM != NULL));

	// create a EventMapper state-machine to process our events

	m_pEEM = new EV_EditEventMapper(m_pEBM);
	UT_ASSERT(m_pEEM);

	// select which menu bar we should use
	// TODO override szMenuLayout using m_app->m_pArgs->{argc,argv}.
	m_szMenuLayoutName = "Main";

	// select language for menu labels
	// TODO override szLanguage using m_app->m_pArgs->{argc,argv}.
	m_szMenuLabelSetName = "EnUS";

	// select which toolbars we should display
	// TODO add an addItem() call for each toolbar we want to have.
	// TODO optionally allow override from m_app->m_pArgs->{argc,argv}.
	m_vecToolbarLayoutNames.addItem("FileEditOps");
	m_vecToolbarLayoutNames.addItem("FormatOps");
	
	// select language for the toolbar labels.
	// i'm not sure if it would ever make sense to
	// deviate from what we set the menus to, but
	// we can if we have to.
	// all toolbars will have the same language.
	m_szToolbarLabelSetName = m_szMenuLabelSetName;
	
	// ... add other stuff here ...

	return UT_TRUE;
}

const EV_EditEventMapper * AP_Frame::getEditEventMapper(void) const
{
	return m_pEEM;
}

AP_App * AP_Frame::getApp(void) const
{
	return m_app;
}

AV_View * AP_Frame::getCurrentView(void) const
{
	// TODO i called this ...Current... in anticipation of having
	// TODO more than one view (think splitter windows) in this
	// TODO frame.  but i'm just guessing right now....
	
	return m_pView;
}

const char * AP_Frame::getFilename(void) const
{
	return m_pDoc->getFilename();
}

UT_Bool AP_Frame::isDirty(void) const
{
	return m_pDoc->isDirty();
}

void AP_Frame::setViewNumber(UT_uint32 n)
{
	m_nView = n;
}

UT_uint32 AP_Frame::getViewNumber(void) const
{
	return m_nView;
}

const char * AP_Frame::getViewKey(void) const
{
	/*
		We want a string key which uniquely identifies a AD_Document instance, 
		so that we can match up top-level views on the same document.  
		
		We can't use the filename, since it might not exist (untitled43) and
		is likely to change when the document is saved.

		So, we just use the AD_Document pointer.  :-)
	*/
	static char buf[9];

	sprintf(buf, "%08lx", (UT_uint32)m_pDoc);

	return buf;
}

const char * AP_Frame::getTitle(int len) const
{
	// TODO: chop down to fit desired size?
	UT_ASSERT((int)strlen(m_szTitle) < len);
	return m_szTitle;
}

const char * AP_Frame::getTempNameFromTitle(void) const
{
	// extract a filename or pathname from the title.
	// strip off all of the title's window decorations (the ":1 *").

	return m_szNonDecoratedTitle;
}
	
UT_Bool AP_Frame::updateTitle()
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
		sprintf(m_szTitle, "Untitled%d", m_iUntitled);
	}

	strcpy(m_szNonDecoratedTitle, m_szTitle);
	
	if (m_nView)
	{
		// multiple top-level views, so append : & view number
		char buf[6];
		UT_ASSERT(m_nView < 10000);
		sprintf(buf, ":%ld", m_nView);
		strcat(m_szTitle, buf);
	}

	if (m_pDoc->isDirty())
	{
		// append " *"
		strcat(m_szTitle, " *");
	}

	return UT_TRUE;
}
