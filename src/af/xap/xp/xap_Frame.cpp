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
#include "ut_debugmsg.h"
#include "ap_App.h"
#include "ap_Frame.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "ap_LoadBindings.h"
#include "ap_Menu_Layouts.h"
#include "ap_Menu_LabelSet.h"
#include "gr_Graphics.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"


#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

AP_Frame::AP_Frame(AP_App * app)
{
	m_app = app;

	m_pDoc = NULL;
	m_pDocLayout = NULL;
	m_pView = NULL;
	m_pScrollObj = NULL;
	m_pG = NULL;
	m_pEBM = NULL;
	m_pEEM = NULL;
	m_pMenuLayout = NULL;
	m_pMenuLabelSet = NULL;

	m_app->rememberFrame(this);
}

AP_Frame::~AP_Frame(void)
{
	// BUGBUG: commented out to avoid circular nastiness in app destructor
//	m_app->forgetFrame(this);
	
	// only delete the things that we created...

	DELETEP(m_pView);
	DELETEP(m_pDocLayout);		// NOTE: this also nukes m_pDoc

	DELETEP(m_pScrollObj);
	DELETEP(m_pG);
	DELETEP(m_pEBM);
	DELETEP(m_pEEM);
	DELETEP(m_pMenuLayout);
	DELETEP(m_pMenuLabelSet);
}

UT_Bool AP_Frame::initialize(int * /*pArgc*/, char *** /*pArgv*/)
{
	UT_Bool bResult;

	// choose which set of key- and mouse-bindings to load
	
	char * szBindings = "default";
	// TODO override szBindings from argc,argv.
	bResult = AP_LoadBindings(szBindings,m_app->getEditMethodContainer(),&m_pEBM);
	UT_ASSERT(bResult && (m_pEBM != NULL));

	// create a EventMapper state-machine to process our events

	m_pEEM = new EV_EditEventMapper(m_pEBM);
	UT_ASSERT(m_pEEM);

	// select which menu bar we should use

	char * szMenuLayout = "Main";
	// TODO override szMenuLayout using argc,argv
	m_pMenuLayout = AP_CreateMenuLayout(szMenuLayout);
	UT_ASSERT(m_pMenuLayout);

	// select language for menu labels
	char * szLanguage = "EnUS";
	// TODO override szLanguage using argc,argv
	m_pMenuLabelSet = AP_CreateMenuLabelSet(szLanguage);
	UT_ASSERT(m_pMenuLabelSet);

	// ... add other stuff here ...

	return UT_TRUE;
}

const EV_Menu_Layout * AP_Frame::getMenuLayout(void) const
{
	return m_pMenuLayout;
}

const EV_Menu_LabelSet * AP_Frame::getMenuLabelSet(void) const
{
	return m_pMenuLabelSet;
}

const EV_EditEventMapper * AP_Frame::getEditEventMapper(void) const
{
	return m_pEEM;
}

AP_App * AP_Frame::getApp(void) const
{
	return m_app;
}

FV_View * AP_Frame::getCurrentView(void) const
{
	// TODO i called this ...Current... in anticipation of having
	// TODO more than one view (think splitter windows) in this
	// TODO frame.  but i'm just guessing right now....
	
	return m_pView;
}

UT_Bool AP_Frame::loadDocument(const char * szFilename)
{
	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	PD_Document * pNewDoc = new PD_Document();
	UT_ASSERT(pNewDoc);
	
	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		goto ReplaceDocument;
	}

	if (pNewDoc->readFromFile(szFilename))
		goto ReplaceDocument;
	
	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	delete pNewDoc;
	return UT_FALSE;

ReplaceDocument:
	// NOTE: prior document is bound to m_pDocLayout, which gets discarded by subclass
	m_pDoc = pNewDoc;
	return UT_TRUE;
}


