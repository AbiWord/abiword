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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 

#include <glib/gstdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
#include "ut_std_string.h"
#include "ut_growbuf.h"
#include "ut_timer.h"
#include "ut_go_file.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_FrameImpl.h"
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
#include "ev_Toolbar.h"
#include "ev_Menu.h"
#include "xap_Strings.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_Toolbar_Layouts.h"
#include "ut_sleep.h"

#ifdef _MSC_VER
// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

XAP_Frame::XAP_Frame(XAP_FrameImpl *pFrameImpl)
	: m_pDoc(nullptr),
	  m_pView(nullptr),
	  m_pViewListener(nullptr),
	  m_lid(static_cast<AV_ListenerId>(-1)),
	  m_pScrollObj(nullptr),
	  m_nView(0),
	  m_iUntitled(0),
	  m_pScrollbarViewListener(nullptr),
	  m_lidScrollbarViewListener(static_cast<AV_ListenerId>(-1)),
	  m_zoomType(z_PAGEWIDTH),
	  m_pData(nullptr),
	  m_bHideMenuScroll(false),
	  m_iIdAutoSaveTimer(0),
	  m_iAutoSavePeriod(0),
	  m_stAutoSaveExt(),
	  m_bBackupRunning(false),
	  m_bBackupInProgress(false),
	  m_isrcId(0),
	  m_isrcTBNr(0),
	  m_idestId(0),
	  m_idestTBNr(0),
	  m_bisDragging(false),
	  m_bHasDropped(false),
	  m_bHasDroppedTB(false),
	  m_bFirstDraw(false),
	  m_bShowStatusbar(true),
	  m_bShowMenubar(true),
	  m_bIsFrameLocked(false),
	  m_pFrameImpl(pFrameImpl),
	  m_iZoomPercentage(100)
{
	XAP_App::getApp()->rememberFrame(this);
//	UT_DEBUGMSG(("Remembering UnCloned Frame \n"));
//	UT_ASSERT_HARMLESS(0);
}

XAP_Frame::XAP_Frame(XAP_Frame * f)
	: m_pDoc(REFP(f->m_pDoc)),
	m_pView(nullptr),
	m_pViewListener(nullptr),
	m_lid(static_cast<AV_ListenerId>(-1)),
	m_pScrollObj(nullptr),
	m_nView(0),
	m_iUntitled(f->m_iUntitled),
	m_pScrollbarViewListener(nullptr),
	m_lidScrollbarViewListener(static_cast<AV_ListenerId>(-1)),
	m_zoomType(f->m_zoomType),
	m_pData(nullptr),
	m_bHideMenuScroll(f->m_bHideMenuScroll),
	m_iIdAutoSaveTimer(0),
	m_iAutoSavePeriod(f->m_iAutoSavePeriod),
	m_bBackupRunning(false),
	m_bBackupInProgress(false),
	m_isrcId(0),
	m_isrcTBNr(0),
	m_idestId(0),
	m_idestTBNr(0),
	m_bisDragging(false),
	m_bHasDropped(false),
	m_bHasDroppedTB(false),
	m_bFirstDraw(false),
	m_bShowStatusbar(f->m_bShowStatusbar),
	m_bIsFrameLocked(false),
	m_pFrameImpl(f->m_pFrameImpl->createInstance(this)),
	m_iZoomPercentage(f->m_iZoomPercentage)
{
	XAP_App::getApp()->rememberFrame(this, f);
//	UT_DEBUGMSG(("Remembering Cloned Frame \n"));
//	UT_ASSERT_HARMLESS(0);
}

XAP_Frame::~XAP_Frame(void)
{
	/* if we're auto-saving files and now we're exiting normally
	 * delete/unlink the file
	 */
	if (!m_stAutoSaveNamePrevious.empty())
	{
		_removeAutoSaveFile();
	}

	// only delete the things that we created...
	// I do not like this; we should be deleting all our members,
	// since they are no-one else's bussines (Tomas, Jan 30, 2003)

	if (m_pView)
		m_pView->removeListener(m_lid);


	DELETEP(m_pFrameImpl);

	DELETEP(m_pViewListener);
	DELETEP(m_pView);

	UNREFP(m_pDoc);

	DELETEP(m_pScrollObj);

	DELETEP(m_pScrollbarViewListener);

	if (m_iIdAutoSaveTimer != 0)
	{
		UT_Timer *timer = UT_Timer::findTimer(m_iIdAutoSaveTimer);
		if (timer != nullptr)
		{
			UT_DEBUGMSG(("Stopping Autosave timer [%d]\n", m_iIdAutoSaveTimer));
			timer->stop();
			DELETEP(timer);
		}
		else
		{
			UT_DEBUGMSG(("Autosave Timer [%d] not found\n", m_iIdAutoSaveTimer));
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

bool XAP_Frame::initialize(const char * /*szKeyBindingsKey*/, const char * /*szKeyBindingsDefaultValue*/,
						   const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
						   const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
						   const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
						   const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue)
{
	XAP_App * pApp = XAP_App::getApp();


	//////////////////////////////////////////////////////////////////
	// select which menu bar we should use
	//////////////////////////////////////////////////////////////////

	std::string menuLayoutName;
	if (pApp->getPrefsValue(szMenuLayoutKey, menuLayoutName) &&
	    !menuLayoutName.empty()) {
		;
	} else {
		menuLayoutName = szMenuLayoutDefaultValue;
	}
	m_pFrameImpl->m_szMenuLayoutName = g_strdup(menuLayoutName.c_str());

	//////////////////////////////////////////////////////////////////
	// select language for menu labels
	//////////////////////////////////////////////////////////////////

	std::string menuLabelSetName;
	if (pApp->getPrefsValue(szMenuLabelSetKey, menuLabelSetName) &&
	    !menuLabelSetName.empty()) {
		;
	} else {
		menuLabelSetName = szMenuLabelSetDefaultValue;
	}
	m_pFrameImpl->m_szMenuLabelSetName = g_strdup(menuLabelSetName.c_str());

	//////////////////////////////////////////////////////////////////
	// select which toolbars we should display
	//////////////////////////////////////////////////////////////////

	std::string toolbarLayouts;
	if (pApp->getPrefsValue(szToolbarLayoutsKey, toolbarLayouts) &&
	    !toolbarLayouts.empty()) {
		;
	} else {
		toolbarLayouts = szToolbarLayoutsDefaultValue;
	}
	// take space-delimited list and call addItem() for each name in the list.

	{
		char * szTemp;
		szTemp = g_strdup(toolbarLayouts.c_str());
		UT_ASSERT(szTemp);
		for (char * p=strtok(szTemp," "); (p); p=strtok(NULL," "))
		{
			char * szTempName;
			szTempName = g_strdup(p);
			m_pFrameImpl->m_vecToolbarLayoutNames.addItem(szTempName);
		}
		g_free(szTemp);
	}

	//////////////////////////////////////////////////////////////////
	// select language for the toolbar labels.
	// i'm not sure if it would ever make sense to
	// deviate from what we set the menus to, but
	// we can if we have to.
	// all toolbars will have the same language.
	//////////////////////////////////////////////////////////////////

	std::string toolbarLabelSetName;
	if (pApp->getPrefsValue(szToolbarLabelSetKey, toolbarLabelSetName) &&
	    !toolbarLabelSetName.empty()) {
		;
	} else {
		toolbarLabelSetName = szToolbarLabelSetDefaultValue;
	}
	m_pFrameImpl->m_szToolbarLabelSetName = g_strdup(toolbarLabelSetName.c_str());

	//////////////////////////////////////////////////////////////////
	// select the appearance of the toolbar buttons
	//////////////////////////////////////////////////////////////////

	std::string toolbarAppearance;
	pApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance, toolbarAppearance);
	UT_ASSERT(!toolbarAppearance.empty());
	m_pFrameImpl->m_szToolbarAppearance = g_strdup(toolbarAppearance.c_str());

	//////////////////////////////////////////////////////////////////
	// select the auto save options
	//////////////////////////////////////////////////////////////////
	std::string stTmp;
	bool autosave = true;

	pApp->getPrefsValue(XAP_PREF_KEY_AutoSaveFileExt, m_stAutoSaveExt);
	pApp->getPrefsValueBool(XAP_PREF_KEY_AutoSaveFile, autosave);

	if (autosave)
		_createAutoSaveTimer();
	setAutoSaveFile(autosave);

	//////////////////////////////////////////////////////////////////
	// select the default zoom settings
	//////////////////////////////////////////////////////////////////
	pApp->getPrefsValue(XAP_PREF_KEY_ZoomType, stTmp);
	UT_DEBUGMSG(("Zoom type from prefs is %s \n",stTmp.c_str()));
	UT_uint32 iZoom = 100;
	if( g_ascii_strcasecmp( stTmp.c_str(), "100" ) == 0 )
	{
		m_zoomType = z_100;
		iZoom = 100;
	}
	else if( g_ascii_strcasecmp( stTmp.c_str(), "75" ) == 0 )
	{
		m_zoomType = z_75;
		iZoom = 75;
	}
	else if( g_ascii_strcasecmp( stTmp.c_str(), "200" ) == 0 )
	{
		m_zoomType = z_200;
		iZoom = 200;
	}
	else if( g_ascii_strcasecmp( stTmp.c_str(), "Width" ) == 0 )
	{
		m_zoomType = z_PAGEWIDTH;
		std::string zoom;
		pApp->getPrefsValue(XAP_PREF_KEY_ZoomPercentage, zoom);
		if (!zoom.empty()) {
			iZoom = atoi(zoom.c_str());
			if (iZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) {
				iZoom = 100;
			} else if (iZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) {
				iZoom = 100;
			}
		} else {
			iZoom = 100;
		}
	}
	else if( g_ascii_strcasecmp( stTmp.c_str(), "Page" ) == 0 )
	{
		m_zoomType = z_WHOLEPAGE;
		std::string zoom;
		pApp->getPrefsValue(XAP_PREF_KEY_ZoomPercentage, zoom);
		if (!zoom.empty()) {
			iZoom = atoi(zoom.c_str());
			if (iZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) {
				iZoom = 100;
			} else if (iZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) {
				iZoom = 100;
			}
		} else {
			iZoom = 100;
		}
	}
	else
	{
		iZoom = atoi( stTmp.c_str() );

		// These limits are defined in xap_Dlg_Zoom.h
		if ((iZoom <= XAP_DLG_ZOOM_MAXIMUM_ZOOM) && (iZoom >= XAP_DLG_ZOOM_MINIMUM_ZOOM))
		{
			m_zoomType = z_PERCENT;
			XAP_Frame::setZoomPercentage( iZoom );
		}
		else
		  m_zoomType = z_100;
	}
	XAP_Frame::setZoomPercentage( iZoom );

	
	//////////////////////////////////////////////////////////////////
	// ... add other stuff here ...
	//////////////////////////////////////////////////////////////////

	// initialize our helper
	m_pFrameImpl->_initialize();

	return true;
}

extern "C" {
static void autoSaveCallback(UT_Worker *wkr)
{
	xxx_UT_DEBUGMSG(("Autosaving doc...\n"));
	XAP_Frame *me = static_cast<XAP_Frame *> (wkr->getInstanceData());
	AD_Document * pDoc = me->getCurrentDoc();
	if(pDoc && pDoc->isPieceTableChanging())
	{
		UT_DEBUGMSG(("PieceTable is changing no backup made \n"));
	}
	if (me->isDirty())
	{
		UT_Error error = me->backup();

		if (!error) {
			xxx_UT_DEBUGMSG(("Document Auto saved\n"));
		}
		else {
			xxx_UT_DEBUGMSG(("Error [%d] saving document.\n", error));
		}
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
	std::string stPeriod;
	
	bool bFound = XAP_App::getApp()->getPrefsValue(XAP_PREF_KEY_AutoSaveFilePeriod, stPeriod);

	if(!bFound || stPeriod.empty())
		m_iAutoSavePeriod = atoi(XAP_PREF_DEFAULT_AutoSaveFilePeriod);
	else
		m_iAutoSavePeriod = atoi(stPeriod.c_str());

	if(m_iAutoSavePeriod < 1)
		m_iAutoSavePeriod = 1;

	// stPeriod is in minutes, and we should use milliseconds
	timer->set(m_iAutoSavePeriod * 60000);
	m_iIdAutoSaveTimer = timer->getIdentifier();
	UT_DEBUGMSG(("Creating auto save timer [%d] with a timeout of [%d] minutes.\n", m_iIdAutoSaveTimer, m_iAutoSavePeriod));
}

void XAP_Frame::_removeAutoSaveFile()
{
	const char *filename = NULL;
	gboolean bURI = UT_go_path_is_uri(m_stAutoSaveNamePrevious.c_str());

	if(bURI)
	{
		filename = UT_go_filename_from_uri(m_stAutoSaveNamePrevious.c_str());
	}
	else
	{
		// It shouldn't be a file name here, but handle it nonetheless
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		filename = m_stAutoSaveNamePrevious.c_str();
	}

	if(filename)
	{
		UT_DEBUGMSG(("DOM: removing backup file %s\n", filename));
		int res = g_unlink(filename);

		if(res == -1)
		{
			UT_DEBUGMSG(("Failed to unlink old backup file %s\n", filename));
		}

		// only g_free in the UT_go_filename_from_uri case
		if(bURI)
			FREEP(filename);
	}
}

/*!
 * This starts the auto Updater for the view
 */
void XAP_FrameImpl::_startViewAutoUpdater(void)
{
	if(m_ViewAutoUpdaterID == 0)
	{
		m_ViewAutoUpdater = UT_Timer::static_constructor(viewAutoUpdater, this);
		m_ViewAutoUpdater->set(500);
		m_ViewAutoUpdaterID = m_ViewAutoUpdater->getIdentifier();
		m_ViewAutoUpdater->start();
		m_pFrame->m_bFirstDraw = false;
	}
}

/*!
 * This static function updates the current view in frame while the layout
 * is filling.
 */
void /* static*/ XAP_FrameImpl::viewAutoUpdater(UT_Worker *wkr)
{
	XAP_FrameImpl *pFrameImpl = static_cast<XAP_FrameImpl *> (wkr->getInstanceData());
	XAP_App *pApp = XAP_App::getApp();
	const XAP_StringSet * pSS = pApp->getStringSet();
	std::string msg;
	pSS->getValue(XAP_STRING_ID_MSG_BuildingDoc, pApp->getDefaultEncoding(),msg);
	pFrameImpl->_setCursor(GR_Graphics::GR_CURSOR_WAIT);
	AV_View * pView = pFrameImpl->m_pFrame->getCurrentView();
	UT_DEBUGMSG(("SEVIOR: frame view updater \n"));
	if(!pView)
	{
		pFrameImpl->m_pFrame->setCursor(GR_Graphics::GR_CURSOR_DEFAULT);
		pFrameImpl->m_ViewAutoUpdater->stop();
		pFrameImpl->m_ViewAutoUpdaterID = 0;
		DELETEP(pFrameImpl->m_ViewAutoUpdater);
		return;
	}
	if(!pView->isLayoutFilling() && (pView->getPoint() > 0))
	{
		GR_Graphics * pG = pView->getGraphics();
		pG->setCursor(GR_Graphics::GR_CURSOR_DEFAULT);
		pFrameImpl->m_pFrame->setCursor(GR_Graphics::GR_CURSOR_DEFAULT);
		pView->setCursorToContext();
		pFrameImpl->m_ViewAutoUpdater->stop();
		pFrameImpl->m_ViewAutoUpdaterID = 0;
		DELETEP(pFrameImpl->m_ViewAutoUpdater);
		pView->queueDraw();
		pG->flush();
		return;
	}
	if(!pView->isLayoutFilling() && !pFrameImpl->m_pFrame->m_bFirstDraw)
	{
		GR_Graphics * pG = pView->getGraphics();
		pG->setCursor(GR_Graphics::GR_CURSOR_WAIT);
		pFrameImpl->_setCursor(GR_Graphics::GR_CURSOR_WAIT);
		pFrameImpl->m_pFrame->setStatusMessage ( static_cast<const gchar *>(msg.c_str()) );
		pG->flush();
		return;
	}
	GR_Graphics * pG = pView->getGraphics();
	pG->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	pFrameImpl->_setCursor(GR_Graphics::GR_CURSOR_WAIT);
	pFrameImpl->m_pFrame->setStatusMessage ( static_cast<const gchar *>(msg.c_str()) );

	if(pView->getPoint() > 0)
	{
		pView->updateLayout();
		if(!pFrameImpl->m_pFrame->m_bFirstDraw)
		{
			pView->queueDraw();
			pFrameImpl->m_pFrame->m_bFirstDraw = true;
		}
		else
		{
			pView->updateScreen();
		}
	}
	pG->flush();
}

UT_RGBColor XAP_Frame::getColorSelBackground () const
{
  return m_pFrameImpl->getColorSelBackground ();
}

UT_RGBColor XAP_Frame::getColorSelForeground () const
{
  return m_pFrameImpl->getColorSelForeground ();
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
	if (m_pDoc == NULL) return NULL;
	return m_pDoc->getFilename().c_str();
}

bool XAP_Frame::isDirty(void) const
{
	if (m_pDoc == NULL) return false;
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

	sprintf(buf, "%p", static_cast<void *>(m_pDoc));

	return buf;
}

const std::string & XAP_Frame::getTitle() const
{
	return m_sTitle;
}

const char * XAP_Frame::getNonDecoratedTitle() const
{
	return m_sNonDecoratedTitle.c_str();
}

void XAP_Frame::setZoomPercentage(UT_uint32 iZoom)
{
	m_iZoomPercentage = iZoom;
	XAP_App * pApp = XAP_App::getApp();
	UT_return_if_fail(pApp);
	XAP_Prefs * pPrefs = pApp->getPrefs();
	UT_return_if_fail(pPrefs);
	XAP_PrefsScheme * pScheme = pPrefs->getCurrentScheme(true);
	UT_return_if_fail(pScheme);
	std::string sZoom = UT_std_string_sprintf("%d",iZoom);
	if(getZoomType() == z_PAGEWIDTH)
	{
		pScheme->setValue(XAP_PREF_KEY_ZoomType,"Width");
	}
	else if(getZoomType() == z_WHOLEPAGE)
	{
		pScheme->setValue(XAP_PREF_KEY_ZoomType,"Page");
	}
	else
	{
		pScheme->setValue(XAP_PREF_KEY_ZoomType,sZoom.c_str());
	}
	UT_DEBUGMSG(("zoom is set to %s \n",sZoom.c_str()));
	pScheme->setValue(XAP_PREF_KEY_ZoomPercentage,sZoom.c_str());
}

UT_uint32 XAP_Frame::getZoomPercentage(void)
{
	return m_iZoomPercentage;
}

EV_Toolbar *  XAP_Frame::getToolbar(UT_sint32 ibar)
{
	if(ibar >= m_pFrameImpl->m_vecToolbars.getItemCount())
		return NULL;
	return m_pFrameImpl->m_vecToolbars.getNthItem(ibar);
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

void XAP_FrameImpl::_createToolbars(void)
{
	bool bResult;
	UT_sint32 nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (UT_sint32 k=0; k < nrToolbars; k++)
	{
		EV_Toolbar * pToolbar = m_pFrame->_newToolbar(m_pFrame,
							      reinterpret_cast<const char *>(m_vecToolbarLayoutNames.getNthItem(k)),
							      reinterpret_cast<const char *>(m_szToolbarLabelSetName));
		UT_continue_if_fail(pToolbar);
		bResult = pToolbar->synthesize();
		UT_ASSERT(bResult);
		
		m_vecToolbars.addItem(pToolbar);
	}
	UT_UNUSED(bResult); // TODO deal with the result
}

UT_sint32 XAP_Frame::findToolbarNr(EV_Toolbar * pTB)
{
	UT_sint32 i = 0;
	bool bFound =  false;
	for(i =0; !bFound && (i < m_pFrameImpl->m_vecToolbars.getItemCount()); i++)
	{
		EV_Toolbar * pTmp = getToolbar(i);
		if(pTmp == pTB)
		{
			bFound  = true;
			break;
		}
	}
	if(bFound)
	{
		return static_cast<UT_sint32>(i);
	}
	return -1;
}

void XAP_Frame::setAutoSaveFile(bool b)
{
	m_bBackupRunning = b;
	if (b && !m_iIdAutoSaveTimer)
	{
		UT_Timer *timer = UT_Timer::static_constructor(autoSaveCallback, this);
		if(m_iAutoSavePeriod < 1)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			m_iAutoSavePeriod = 1;
		}

		timer->set(m_iAutoSavePeriod * 60000);
		m_iIdAutoSaveTimer = timer->getIdentifier();
		timer->start();
		return;
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
	if(b)
	{
		UT_Timer *timer = UT_Timer::findTimer(m_iIdAutoSaveTimer);
		if(m_iAutoSavePeriod < 1)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			m_iAutoSavePeriod = 1;
		}

		// reset the timer, because the interval might have changed (Bug 9329)
		timer->set(m_iAutoSavePeriod * 60000);
		timer->start();
		return;
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

void XAP_Frame::setAutoSaveFileExt(const std::string &stExt)
{
	m_stAutoSaveExt = stExt;
}

XAP_Dialog_MessageBox * XAP_Frame::createMessageBox(XAP_String_Id id,
						    XAP_Dialog_MessageBox::tButtons buttons,
						    XAP_Dialog_MessageBox::tAnswer default_answer,
						    ...)
{
  	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(getDialogFactory());

	XAP_Dialog_MessageBox * pDialog
		= static_cast<XAP_Dialog_MessageBox *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_return_val_if_fail(pDialog, NULL);

	if (id > 0) {
		char * szNewMessage = static_cast<char *>(g_try_malloc(sizeof(char) * 256));
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
		std::string s;
		pSS->getValue(id, XAP_App::getApp()->getDefaultEncoding(), s);

		va_list args;
		va_start(args, default_answer);
		vsnprintf(szNewMessage, 256, s.c_str(), args);
		va_end(args);

		pDialog->setMessage("%s", szNewMessage);

		// XAP_MessageBox makes a copy of the message, so g_free it
		FREEP(szNewMessage);
	}
	pDialog->setButtons(buttons);
	pDialog->setDefaultAnswer(default_answer);

	return pDialog;
}
XAP_Dialog_MessageBox::tAnswer XAP_Frame::showMessageBox(XAP_Dialog_MessageBox * pDialog)
{
	raise();

	pDialog->runModal(this);
	XAP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();
	delete pDialog;

	return ans;
}

XAP_Dialog_MessageBox::tAnswer XAP_Frame::showMessageBox(XAP_String_Id id,
							 XAP_Dialog_MessageBox::tButtons buttons,
							 XAP_Dialog_MessageBox::tAnswer default_answer)
{
  XAP_Dialog_MessageBox * pDialog = createMessageBox(id, buttons, default_answer);
  return showMessageBox(pDialog);
}

XAP_Dialog_MessageBox::tAnswer XAP_Frame::showMessageBox(XAP_String_Id id,
							 XAP_Dialog_MessageBox::tButtons buttons,
							 XAP_Dialog_MessageBox::tAnswer default_answer,
							 const char * sz)
{
  XAP_Dialog_MessageBox * pDialog = createMessageBox(id, buttons, default_answer, sz);
  return showMessageBox(pDialog);
}

XAP_Dialog_MessageBox::tAnswer XAP_Frame::showMessageBox(const std::string & msg,
							 XAP_Dialog_MessageBox::tButtons buttons,
							 XAP_Dialog_MessageBox::tAnswer default_answer)
{
	return showMessageBox(msg.c_str(), buttons, default_answer);
}

XAP_Dialog_MessageBox::tAnswer XAP_Frame::showMessageBox(const char * szMessage,
							 XAP_Dialog_MessageBox::tButtons buttons,
							 XAP_Dialog_MessageBox::tAnswer default_answer)
{
  XAP_Dialog_MessageBox * pDialog = createMessageBox(0, buttons, default_answer);
  pDialog->setMessage(szMessage);
  return showMessageBox(pDialog);
}

std::string XAP_Frame::makeBackupName(const char* szExt)
{
  std::string ext(szExt ? szExt : m_stAutoSaveExt);
  std::string oldName(m_pDoc->getFilename());
  std::string backupName;
  UT_DEBUGMSG(("In make Backup name. Old Name is (%s) \n",oldName.c_str()));
  if (oldName.empty())
  {
      const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
      std::string sTmp;
      pSS->getValue(XAP_STRING_ID_UntitledDocument, XAP_App::getApp()->getDefaultEncoding(), sTmp);
      oldName = UT_std_string_sprintf(sTmp.c_str(), m_iUntitled);

      UT_DEBUGMSG(("Untitled.  We will give it the name [%s]\n", oldName.c_str()));
  }
  else {
    UT_DEBUGMSG(("Filename [%s]\n", oldName.c_str()));
  }
  
  backupName = oldName + ext;

  const char* uri = NULL;
  gboolean bURI = UT_go_path_is_uri(backupName.c_str());

  if(!bURI)
    uri = UT_go_filename_to_uri(backupName.c_str());

  if(uri)
  {
    backupName = uri;
    FREEP(uri);
  }

  UT_DEBUGMSG(("DOM: created backup filename (%s)\n", backupName.c_str()));

  return backupName;
}

/**
 * It saves the current document with an extension stExt.
 * If the extension is empty, then it save the document with
 * the default extension (as defined in the preferences dialog box)
 */
UT_Error XAP_Frame::backup(const char* szExt, UT_sint32 iEFT)
{
	if (m_bBackupInProgress)
		return UT_OK;

	if (!m_pDoc)
	{
		UT_DEBUGMSG(("File NOT saved! doc is NULL.\n"));
		return UT_OK;
	}

	m_bBackupInProgress = true;

	std::string backupName = makeBackupName ( szExt );

	if (m_stAutoSaveNamePrevious.size() && (backupName != m_stAutoSaveNamePrevious))
	{
		/* If the user does a Save-As to rename the file then the auto-save name also changes, so
		 * need to remove the old backup file...
		 */
		_removeAutoSaveFile();
	}
	m_stAutoSaveNamePrevious = backupName;
	
	UT_Error error;
//
// Don't put this auto-save in the most recent list.
//
	XAP_App::getApp()->getPrefs()->setIgnoreNextRecent();
	
	if(iEFT < 0)
	{
	        iEFT = 1; // *.abw format
		error = m_pDoc->saveAs(backupName.c_str(), iEFT, false);
	}
	else
	{
		error = m_pDoc->saveAs(backupName.c_str(), iEFT, false);
	}

	if(error == UT_OK)
	{
		UT_DEBUGMSG(("File %s saved.\n", backupName.c_str()));
	}
	else
	{
		// TODO: alert the user
		UT_DEBUGMSG(("File backup failed.\n"));
	}

	m_bBackupInProgress = false;
	return error;
}

void XAP_Frame::quickZoom(void)
{
	if( !m_pView ) return;
	UT_uint32 newZoom = 100;
	switch( getZoomType() )
	{
	case z_PAGEWIDTH:
		newZoom = m_pView->calculateZoomPercentForPageWidth();
		if      (newZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MINIMUM_ZOOM;
		else if (newZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MAXIMUM_ZOOM;
		XAP_Frame::setZoomPercentage( newZoom );
		quickZoom( newZoom );
		break;
	case z_WHOLEPAGE:
		newZoom = m_pView->calculateZoomPercentForWholePage() ;
		if      (newZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MINIMUM_ZOOM;
		else if (newZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MAXIMUM_ZOOM;
		XAP_Frame::setZoomPercentage( newZoom );
		quickZoom( newZoom );
		break;
	default:
		m_pView->updateScreen(false);
       ;
   }
}

void XAP_Frame::updateZoom(void)
{
	if( !m_pView ) return;
	UT_uint32 newZoom = 100;
	switch( getZoomType() )
	{
	case z_PAGEWIDTH:
		newZoom = m_pView->calculateZoomPercentForPageWidth();
		if      (newZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MINIMUM_ZOOM;
		else if (newZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MAXIMUM_ZOOM;
		XAP_Frame::setZoomPercentage( newZoom );
		quickZoom( newZoom );
//		setZoomPercentage( newZoom );
		break;
	case z_WHOLEPAGE:
		newZoom = m_pView->calculateZoomPercentForWholePage() ;
		if      (newZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MINIMUM_ZOOM;
		else if (newZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) newZoom = XAP_DLG_ZOOM_MAXIMUM_ZOOM;
		XAP_Frame::setZoomPercentage( newZoom );
		quickZoom( newZoom );
//		setZoomPercentage( newZoom );
		break;
	default:
       ;
   }
}

/*!
 * This method rebuilds all the toolbars in the frame. Useful for when the
 * user wants to revert to default toolbars.
 */
void XAP_Frame::rebuildAllToolbars(void)
{
	UT_uint32 count = m_pFrameImpl->m_vecToolbars.getItemCount();
	UT_uint32 i =0;
	for(i=0; i< count; i++)
	{
		m_pFrameImpl->_rebuildToolbar(i);
	}
}


/*!
 * Record stuff for start of drag.
\param XAP_Toolbar_Id srcId - source of Toolbar Icon.
\param EV_Toolbar * pTBSrc pointer to toolbar class that contains the icon.
*/
void XAP_Frame::dragBegin(XAP_Toolbar_Id srcId, EV_Toolbar * pTBsrc)
{
	m_isrcId = srcId;
	m_isrcTBNr = findToolbarNr(pTBsrc);
	m_bisDragging = true;
	m_bHasDropped = false;
	m_bHasDroppedTB = false;
	m_idestId = 0;
	m_idestTBNr = 0;
}
	
/*
 * Record the XP stuff from drop event recorded from the toolbars onto an icon
 */
void XAP_Frame::dragDropToIcon(XAP_Toolbar_Id srcId,XAP_Toolbar_Id destId, EV_Toolbar * pTBsrc, EV_Toolbar * pTBdest)
{
	UT_UNUSED(srcId);
	UT_UNUSED(pTBsrc);
	UT_ASSERT(m_isrcId == srcId);
	UT_ASSERT(m_isrcTBNr == findToolbarNr(pTBsrc));
	m_idestId = destId;
	m_idestTBNr = findToolbarNr(pTBdest);
	m_bHasDropped = true;
}

/*
 * Record the XP stuff from drop event recorded from the toolbars onto a bare
 * toolbar
 */
void XAP_Frame::dragDropToTB(XAP_Toolbar_Id srcId,EV_Toolbar * pTBsrc, EV_Toolbar * pTBdest)
{
	UT_UNUSED(srcId);
	UT_UNUSED(pTBsrc);
	UT_ASSERT(m_isrcId == srcId);
	UT_ASSERT(m_isrcTBNr == findToolbarNr(pTBsrc));
	m_idestTBNr = findToolbarNr(pTBdest);
	m_bHasDroppedTB = true;
}

time_t XAP_Frame::getTimeSinceSave() const
{
	return m_pDoc->getTimeSinceSave();
}

XAP_FrameMode XAP_Frame::getFrameMode()
{
	return m_pFrameImpl->m_iFrameMode;
}
void XAP_Frame::setFrameMode(XAP_FrameMode iFrameMode)
{
	m_pFrameImpl->m_iFrameMode = iFrameMode;
}

void XAP_Frame::dragText()
{
  m_pFrameImpl->dragText();
}
