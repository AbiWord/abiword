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
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_timer.h"
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
#include "ev_Keyboard.h"
#include "ev_Mouse.h"
#include "xap_Strings.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"

/*****************************************************************/

XAP_Frame::XAP_Frame(XAP_App * app)
	: m_app(app),
	  m_pDoc(0),
	  m_pView(0),
	  m_pViewListener(0),
	  m_lid((AV_ListenerId)-1),
	  m_pScrollObj(0),
	  m_szMenuLayoutName(0),
	  m_szMenuLabelSetName(0),
	  m_szToolbarLabelSetName(0),
	  m_szToolbarAppearance(0),
	  m_nView(0),
	  m_iUntitled(0),
	  m_pMouse(0),
	  m_pKeyboard(0),
	  m_pScrollbarViewListener(0),
	  m_lidScrollbarViewListener((AV_ListenerId)-1),
	  m_zoomType(z_100),
	  m_pData(0),
	  m_pInputModes(0),
	  m_iIdAutoSaveTimer(0),
	  m_iAutoSavePeriod(0),
	  m_stAutoSaveExt(),
	  m_bBackupRunning(false)
{
	m_app->rememberFrame(this);
	memset(m_szTitle,0,sizeof(m_szTitle));
	memset(m_szNonDecoratedTitle,0,sizeof(m_szNonDecoratedTitle));
}

XAP_Frame::XAP_Frame(XAP_Frame * f)
	: m_app(f->m_app),                        // only clone a few things
	m_pDoc(REFP(f->m_pDoc)),
	m_pView(0),
	m_pViewListener(0),
	m_lid((AV_ListenerId)-1),
	m_pScrollObj(0),
	m_szMenuLayoutName(0),
	m_szMenuLabelSetName(0),
	m_szToolbarLabelSetName(0),
	m_szToolbarAppearance(0),
	m_nView(0),
	m_iUntitled(f->m_iUntitled),
	m_pMouse(0),
	m_pKeyboard(0),
	m_pScrollbarViewListener(0),
	m_lidScrollbarViewListener((AV_ListenerId)-1),
	m_zoomType(z_100),
	m_pData(0),
	m_pInputModes(0),
	m_iIdAutoSaveTimer(0),
	m_bBackupRunning(false)
{
	m_app->rememberFrame(this, f);
	memset(m_szTitle,0,sizeof(m_szTitle));
	memset(m_szNonDecoratedTitle,0,sizeof(m_szNonDecoratedTitle));
}

XAP_Frame::~XAP_Frame(void)
{
	// only delete the things that we created...
  	DELETEP(m_pKeyboard);
	DELETEP(m_pMouse);

	if (m_pView)
		m_pView->removeListener(m_lid);

	DELETEP(m_pView);
	DELETEP(m_pViewListener);

	UNREFP(m_pDoc);

	DELETEP(m_pScrollObj);
	DELETEP(m_pInputModes);

	DELETEP(m_pScrollbarViewListener);

	UT_VECTOR_FREEALL(char *,m_vecToolbarLayoutNames);
	
	FREEP(m_szMenuLayoutName);
	FREEP(m_szMenuLabelSetName);
	FREEP(m_szToolbarLabelSetName);
	FREEP(m_szToolbarAppearance);

	UT_VECTOR_PURGEALL(EV_Toolbar *, m_vecToolbars);

	if (m_iIdAutoSaveTimer != 0)
	{
		UT_Timer *timer = UT_Timer::findTimer(m_iIdAutoSaveTimer);
		if (timer != 0)
		{
			xxx_UT_DEBUGMSG(("Stopping timer [%d]\n", m_iIdAutoSaveTimer));
			timer->stop();
		}
		else
		{
			xxx_UT_DEBUGMSG(("Timer [%d] not found\n", m_iIdAutoSaveTimer));
		}
	}
}

/*****************************************************************/
// sequence number tracker for untitled documents

int XAP_Frame::s_iUntitled = 0;	
int XAP_Frame::_getNextUntitledNumber(void)
{
	return ++s_iUntitled;
}

/*****************************************************************/

bool XAP_Frame::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
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

	if ((pApp->getPrefsValue(szKeyBindingsKey,
				 (const XML_Char**)&szBindings)) && 
	    (szBindings) && (*szBindings))
		pBindingMap = m_app->getBindingMap(szBindings);
	if (!pBindingMap)
		pBindingMap = m_app->getBindingMap(szKeyBindingsDefaultValue);
	UT_ASSERT(pBindingMap);

	if (!m_pInputModes)
	{
		m_pInputModes = new XAP_InputModes();
		UT_ASSERT(m_pInputModes);
	}
	bool bResult;
	bResult = m_pInputModes->createInputMode(szBindings,pBindingMap);
	UT_ASSERT(bResult);
	bool bResult2;
	bResult2 = m_pInputModes->setCurrentMap(szBindings);
	UT_ASSERT(bResult2);
	
	//////////////////////////////////////////////////////////////////
	// select which menu bar we should use
	//////////////////////////////////////////////////////////////////

	const char * szMenuLayoutName = NULL;
	if ((pApp->getPrefsValue(szMenuLayoutKey,
				 (const XML_Char**)&szMenuLayoutName)) &&
	    (szMenuLayoutName) && (*szMenuLayoutName))
		;
	else
		szMenuLayoutName = szMenuLayoutDefaultValue;
	UT_cloneString((char *&)m_szMenuLayoutName,szMenuLayoutName);
	
	//////////////////////////////////////////////////////////////////
	// select language for menu labels
	//////////////////////////////////////////////////////////////////

	const char * szMenuLabelSetName = NULL;
	if ((pApp->getPrefsValue(szMenuLabelSetKey,
				 (const XML_Char**)&szMenuLabelSetName)) &&
	    (szMenuLabelSetName) && (*szMenuLabelSetName))
		;
	else
		szMenuLabelSetName = szMenuLabelSetDefaultValue;
	UT_cloneString((char *&)m_szMenuLabelSetName,szMenuLabelSetName);
	
	//////////////////////////////////////////////////////////////////
	// select which toolbars we should display
	//////////////////////////////////////////////////////////////////

	const char * szToolbarLayouts = NULL;
	if ((pApp->getPrefsValue(szToolbarLayoutsKey,
				 (const XML_Char**)&szToolbarLayouts)) &&
	    (szToolbarLayouts) && (*szToolbarLayouts))
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
	if ((pApp->getPrefsValue(szToolbarLabelSetKey,
				 (const XML_Char**)&szToolbarLabelSetName)) &&
	    (szToolbarLabelSetName) && (*szToolbarLabelSetName))
		;
	else
		szToolbarLabelSetName = szToolbarLabelSetDefaultValue;
	UT_cloneString((char *&)m_szToolbarLabelSetName,szToolbarLabelSetName);
	
	//////////////////////////////////////////////////////////////////
	// select the appearance of the toolbar buttons
	//////////////////////////////////////////////////////////////////

	const char * szToolbarAppearance = NULL;
	pApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,
			    (const XML_Char**)&szToolbarAppearance);
	UT_ASSERT((szToolbarAppearance) && (*szToolbarAppearance));
	UT_cloneString((char *&)m_szToolbarAppearance,szToolbarAppearance);

	//////////////////////////////////////////////////////////////////
	// select the auto save options
	//////////////////////////////////////////////////////////////////
	UT_String stTmp;
	bool autosave;

	pApp->getPrefsValue(XAP_PREF_KEY_AutoSaveFilePeriod, stTmp);
	m_iIdAutoSaveTimer = atoi(stTmp.c_str());
	pApp->getPrefsValue(XAP_PREF_KEY_AutoSaveFileExt, m_stAutoSaveExt);
	pApp->getPrefsValueBool(XAP_PREF_KEY_AutoSaveFile, &autosave);

	if (autosave)
		_createAutoSaveTimer();
	
	//////////////////////////////////////////////////////////////////
	// ... add other stuff here ...
	//////////////////////////////////////////////////////////////////

	return true;
}

extern "C" {
static void autoSaveCallback(UT_Timer *timer)
{
	UT_DEBUGMSG(("Autosaving doc...\n"));
	XAP_Frame *me = static_cast<XAP_Frame *> (timer->getInstanceData());

	if (me->isDirty())
	{
		UT_Error error = me->backup();

		if (!error)
			UT_DEBUGMSG(("Document saved\n"));
		else
			UT_DEBUGMSG(("Error [%d] saving document.\n", error));
	}
	else
	{
		 UT_DEBUGMSG(("Doc is not dirty\n"));
	}
}
}

void XAP_Frame::_createAutoSaveTimer()
{
	UT_Timer *timer = UT_Timer::static_constructor(autoSaveCallback, this);
	UT_String stPeriod;
	
	UT_ASSERT(m_app);
	m_app->getPrefsValue(XAP_PREF_KEY_AutoSaveFilePeriod, stPeriod);
	UT_ASSERT(stPeriod.empty() == false);
	m_iAutoSavePeriod = atoi(stPeriod.c_str());
	
	// stPeriod is in minutes, and we should use milliseconds
	timer->set(m_iAutoSavePeriod * 60000);
	m_iIdAutoSaveTimer = timer->getIdentifier();
	UT_DEBUGMSG(("Creating auto save timer [%d] with a timeout of [%d] minutes.\n", m_iIdAutoSaveTimer, m_iAutoSavePeriod));
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
		bool bResult;
		bResult = m_pInputModes->createInputMode(szName,pBindingMap);
		UT_ASSERT(bResult);
	}
	
	// note: derrived classes will need to update keyboard
	// note: and mouse after we return.

	UT_DEBUGMSG(("Setting InputMode to [%s] for the current window.\n",szName));
	
	bool bStatus = m_pInputModes->setCurrentMap(szName);
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
	
AD_Document * XAP_Frame::getCurrentDoc(void) const
{
	return m_pDoc;
}

const char * XAP_Frame::getFilename(void) const
{
	return m_pDoc->getFilename();
}

bool XAP_Frame::isDirty(void) const
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

	sprintf(buf, "%p", (void *)m_pDoc);

	return buf;
}

const char * XAP_Frame::getTitle(int len) const
{
	// Returns the pathname being edited (with view and dirty bit
	// '*' adornments), if all that fits. If it doesn't fit,
	// returns the tail of the string that does fit. Would be
	// better to chop it at a pathname separator boundary.
	if ((int)strlen(m_szTitle) <= len)
	{
		return m_szTitle;
	}
	else
	{
		return m_szTitle + (strlen(m_szTitle)-len);
	}
}

const char * XAP_Frame::getTempNameFromTitle(void) const
{
	// extract a filename or pathname from the title.
	// strip off all of the title's window decorations (the ":1 *").

	return m_szNonDecoratedTitle;
}
	
bool XAP_Frame::updateTitle()
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
		// Note: The previous version of this code blew up
		// when assertions were turned on and more than 244
		// characters were to be copied. I now copy only
		// the last 244 characters in that case. I do not know
		// where "244" comes from - the buffer being copied into
		// is currently 512 bytes. However, even 244 characters
		// is way too long for a title bar, so no reason to
		// increase the size to fit more snugly into the buffer.
		// Here's that previous assertion:
		// UT_ASSERT(strlen(szName) < 245); // TODO need #define for this number

		// Check that the buffer (with generous room for
		// later decorations) is big enough.
		UT_ASSERT(sizeof(m_szTitle) > 245 + 30); 

		if (strlen(szName) <= 244)
		{
			strcpy(m_szTitle, szName); 
		}
		else 
		{
			// copy the tail of the pathname, the useful part.
			// would be a bit more useful to break it at a	
			// pathname component separator.
			strcpy(m_szTitle, szName + (strlen(szName) - 244));
		}
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

	return true;
}

void XAP_Frame::setZoomPercentage(UT_uint32 /* iZoom */)
{
	// default does nothing.  see subclasses for override
}

UT_uint32 XAP_Frame::getZoomPercentage(void)
{
	return 100;	// default implementation
}

EV_Toolbar *  XAP_Frame::getToolbar(UT_uint32 ibar)
{
	if(ibar >= m_vecToolbars.getItemCount())
		return NULL;
	return (EV_Toolbar *) m_vecToolbars.getNthItem(ibar);
}

bool XAP_Frame::repopulateCombos(void)
{
  //
// Update the styles combo box.
//
	EV_Toolbar * pTbar = NULL;
	UT_uint32 ibar = 0;
	do
	{
		pTbar = getToolbar(ibar++);
		if(pTbar)
			pTbar->repopulateStyles();
	}
	while(pTbar);
	return true;
}

void XAP_Frame::_createToolbars(void)
{
	bool bResult;
	UT_uint32 nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (UT_uint32 k=0; k < nrToolbars; k++)
	{
		EV_Toolbar * pToolbar
				= _newToolbar(m_app, this,
					(const char *)m_vecToolbarLayoutNames.getNthItem(k),
					(const char *)m_szToolbarLabelSetName);
		UT_ASSERT(pToolbar);
		bResult = pToolbar->synthesize();
		UT_ASSERT(bResult);
		
		m_vecToolbars.addItem(pToolbar);
	}
}

EV_Mouse * XAP_Frame::getMouse(void)
{
	return m_pMouse;
}

EV_Keyboard * XAP_Frame::getKeyboard(void)
{
	return m_pKeyboard;
}

void XAP_Frame::setAutoSaveFile(bool b)
{
	if (b && !m_iIdAutoSaveTimer)
	{
		UT_Timer *timer = UT_Timer::static_constructor(autoSaveCallback, this);
		UT_ASSERT(m_iAutoSavePeriod != 0);
		timer->set(m_iAutoSavePeriod * 60000);
		m_iIdAutoSaveTimer = timer->getIdentifier();
	}

	if (!b && m_iIdAutoSaveTimer)
	{
		// TODO: We're leaking UT_Timer objects.  We should
		// TODO: give the posibility to erase a UT_Timer...
		// TODO: something like UT_Timer::eraseTimer(...) should
		// TODO: do the work (we should change the sign. of findTimer).
		UT_Timer *timer = UT_Timer::findTimer(m_iIdAutoSaveTimer);
		if (timer)
			timer->stop();
	}
}

void XAP_Frame::setAutoSaveFilePeriod(int min)
{
	m_iAutoSavePeriod = min;
	
	if (m_iIdAutoSaveTimer != 0)
	{
		// I know, it looks weird... I just want to restart the timer
		setAutoSaveFile(false);
		setAutoSaveFile(true);
	}
}

void XAP_Frame::setAutoSaveFileExt(const UT_String &stExt)
{
	m_stAutoSaveExt = stExt;
}

XAP_Dialog_MessageBox::tAnswer XAP_Frame::showMessageBox(const char *szMessage,
											  XAP_Dialog_MessageBox::tButtons buttons,
											  XAP_Dialog_MessageBox::tAnswer default_answer)
{
	raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(getDialogFactory());

	XAP_Dialog_MessageBox * pDialog
		= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage(szMessage);

	pDialog->setButtons(buttons);
	pDialog->setDefaultAnswer(default_answer);

	pDialog->runModal(this);

	XAP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);

	return ans;
}


XAP_Dialog_MessageBox::tAnswer XAP_Frame::showMessageBox(XAP_String_Id id,
											  XAP_Dialog_MessageBox::tButtons buttons,
											  XAP_Dialog_MessageBox::tAnswer default_answer,
											  const char *p_str1)
{
	raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(getDialogFactory());

	XAP_Dialog_MessageBox * pDialog
		= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	const XAP_StringSet * pSS = getApp()->getStringSet();

	if(p_str1)
	{
		pDialog->setMessage((char*)pSS->getValue(id), p_str1);
	}
	else
	{
		pDialog->setMessage((char*)pSS->getValue(id));
	}


	pDialog->setButtons(buttons);
	pDialog->setDefaultAnswer(default_answer);

	pDialog->runModal(this);

	XAP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);

	return ans;
}

UT_Error XAP_Frame::backup()
{
	if (m_bBackupRunning)
		return UT_OK;
	m_bBackupRunning = true;

	UT_String &ext = m_stAutoSaveExt;
	UT_String oldName(m_pDoc->getFilename() ? m_pDoc->getFilename() : "");
	UT_String backupName;

	if (oldName.empty())
	{
		const XAP_StringSet * pSS = m_app->getStringSet();
		const char *szTmp = pSS->getValue(XAP_STRING_ID_UntitledDocument);
		int iLen = strlen(szTmp) + 10;
		char *szTmp2 = new char[iLen];

		sprintf(szTmp2, szTmp, m_iUntitled);
		oldName = szTmp2;
		delete[] (szTmp2);
		UT_DEBUGMSG(("Untitled.  We will give it the name [%s]\n", oldName.c_str()));
	}
	else
		UT_DEBUGMSG(("Filename [%s]\n", oldName.c_str()));

	backupName = oldName + ext;
	UT_Error error = m_pDoc->saveAs(backupName, m_pDoc->getLastType(), false);
	UT_DEBUGMSG(("File %s saved.\n", backupName.c_str()));

	m_bBackupRunning = false;
	return error;
}

void XAP_Frame::updateZoom(void)
{
	UT_uint32 newZoom = 100;
	switch( getZoomType() )
	{
	case z_PAGEWIDTH:
		newZoom = m_pView->calculateZoomPercentForPageWidth();
		if      (newZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MINIMUM_ZOOM;
		else if (newZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MAXIMUM_ZOOM;
		setZoomPercentage( newZoom );
		break;
	case z_WHOLEPAGE:
		newZoom = m_pView->calculateZoomPercentForWholePage() ;
		if      (newZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MINIMUM_ZOOM;
		else if (newZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MAXIMUM_ZOOM;
		setZoomPercentage( newZoom );
		break;
	default:
       ;
   }
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_InputModes::XAP_InputModes(void)
{
	m_indexCurrentEventMap = 0;
}

XAP_InputModes::~XAP_InputModes(void)
{
	UT_ASSERT(m_vecEventMaps.getItemCount() == m_vecNames.getItemCount());

	UT_VECTOR_PURGEALL(EV_EditEventMapper *, m_vecEventMaps);
	UT_VECTOR_FREEALL(char *, m_vecNames);
}

bool XAP_InputModes::createInputMode(const char * szName,
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

	bool b1;
	b1 = (m_vecEventMaps.addItem(pEEM) == 0);
	bool b2;
	b2 = (m_vecNames.addItem(szDup) == 0);
    UT_ASSERT(b1 && b2);

	return true;
}

bool XAP_InputModes::setCurrentMap(const char * szName)
{
	UT_uint32 kLimit = m_vecNames.getItemCount();
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
		if (UT_stricmp(szName,(const char *)m_vecNames.getNthItem(k)) == 0)
		{
			m_indexCurrentEventMap = k;
			return true;
		}

	return false;
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

