/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
 * Copyright (C) 2001 Hubert Figuiere
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

#include "ap_FrameData.h"
#include "ap_MacFrame.h"
#include "ev_MacToolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_MacGraphics.h"
#include "ap_Prefs.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_MacTopRuler.h"
#include "ap_MacLeftRuler.h"
#include "ap_MacStatusBar.h"
#include "xap_ViewListener.h"


AP_MacFrame::AP_MacFrame(XAP_MacApp * app)
	: XAP_MacFrame(app)
{
}

AP_MacFrame::AP_MacFrame(AP_MacFrame * f)
	: XAP_MacFrame(static_cast<XAP_MacFrame *>(f))
{
}

AP_MacFrame::~AP_MacFrame(void)
{
}

bool AP_MacFrame::initialize()
{
	if (!initFrameData())
		return false;

	if (!XAP_MacFrame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
					AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
					AP_PREF_KEY_MenuLabelSet, AP_PREF_DEFAULT_MenuLabelSet,
					AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
					AP_PREF_KEY_ToolbarLabelSet, AP_PREF_DEFAULT_ToolbarLabelSet))
		return false;

	_createTopLevelWindow();
	
	::ShowWindow (m_MacWindow);
	return true;
}

XAP_Frame *	AP_MacFrame::cloneFrame(void)
{
	AP_MacFrame * pClone = new AP_MacFrame(this);
	UT_Error error = UT_OK;
	UT_ASSERT(pClone);

	if (!pClone->initialize())
		goto Cleanup;
	error = pClone->_showDocument();
	if (error)
		goto Cleanup;

	pClone->show();

	return pClone;

Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		m_app->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}

void AP_MacFrame::setStatusMessage(const char * szMsg)
{
	((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}                                                                        

UT_Error AP_MacFrame::loadDocument(const char * szFilename, int ieft)
{
	bool bUpdateClones;
	UT_Vector vClones;
	XAP_App * pApp = getApp();

	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}

	UT_Error err;
	err = _loadDocument(szFilename, (IEFileType) ieft); 
	if (err)
	{
		// we could not load the document.
		// we cannot complain to the user here, we don't know
		// if the app is fully up yet.  we force our caller
		// to deal with the problem.
		return err;
	}

	pApp->rememberFrame(this);
	if (bUpdateClones)
	{
		for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_MacFrame * pFrame = (AP_MacFrame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}

bool AP_MacFrame::initFrameData(void)
{
	UT_ASSERT(!((AP_FrameData*)m_pData));

	UT_ASSERT(m_app);
	
	AP_FrameData* pData = new AP_FrameData(m_app);
	m_pData = (void*) pData;
	
	return (pData ? true : false);
}

void AP_MacFrame::killFrameData(void)
{
	DELETEP((AP_FrameData*)m_pData);
	m_pData = NULL;
}

void AP_MacFrame::setXScrollRange(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
}

void AP_MacFrame::setYScrollRange(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
}


UT_Error AP_MacFrame::_showDocument(UT_uint32 iZoom)
{
	if (!m_pDoc)
	{
		UT_DEBUGMSG(("Can't show a non-existent document\n"));
		return UT_IE_FILENOTFOUND;
	}

	if (!((AP_FrameData*)m_pData))
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_IE_IMPORTERROR;
	}

	GR_MacGraphics * pG = NULL;
	FL_DocLayout * pDocLayout = NULL;
	AV_View * pView = NULL;
	AV_ScrollObj * pScrollObj = NULL;
	ap_ViewListener * pViewListener = NULL;
	AD_Document * pOldDoc = NULL;
	ap_Scrollbar_ViewListener * pScrollbarViewListener = NULL;
	AV_ListenerId lid;
	AV_ListenerId lidScrollbarViewListener;
	UT_uint32 nrToolbars;
	UT_uint32 point = 0;
//	bool bFocus;
//	XAP_UnixFontManager * fontManager = ((XAP_UnixApp *) getApp())->getFontManager();
	
	pG = new GR_MacGraphics(m_MacWindowPort, /*fontManager,*/ getApp());
	UT_ASSERT(pG);
	pG->setZoomPercentage(iZoom);
	
	pDocLayout = new FL_DocLayout(static_cast<PD_Document *>(m_pDoc), pG);
	UT_ASSERT(pDocLayout);
  
//	pDocLayout->formatAll();

	pView = new FV_View(getApp(), this, pDocLayout);
	if (m_pView != NULL)
	{
		point = ((FV_View *) m_pView)->getPoint();
	}
	UT_ASSERT(pView);
//	bFocus=GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(m_wTopLevelWindow),"toplevelWindowFocus"));
//	pView->setFocus(bFocus && (gtk_grab_get_current()==NULL || gtk_grab_get_current()==m_wTopLevelWindow) ? AV_FOCUS_HERE : !bFocus && gtk_grab_get_current()!=NULL && isTransientWindow(GTK_WINDOW(gtk_grab_get_current()),GTK_WINDOW(m_wTopLevelWindow)) ?  AV_FOCUS_NEARBY : AV_FOCUS_NONE);
	// The "AV_ScrollObj pScrollObj" receives
	// send{Vertical,Horizontal}ScrollEvents
	// from both the scroll-related edit methods
	// and from the UI callbacks.
	// 
	// The "ap_ViewListener pViewListener" receives
	// change notifications as the document changes.
	// This ViewListener is responsible for keeping
	// the title-bar up to date (primarily title
	// changes, dirty indicator, and window number).
	// ON UNIX ONLY: we subclass this with ap_UnixViewListener
	// ON UNIX ONLY: so that we can deal with X-Selections.
	//
	// The "ap_Scrollbar_ViewListener pScrollbarViewListener"
	// receives change notifications as the doucment changes.
	// This ViewListener is responsible for recalibrating the
	// scrollbars as pages are added/removed from the document.
	//
	// Each Toolbar will also get a ViewListener so that
	// it can update toggle buttons, and other state-indicating
	// controls on it.
	//
	// TODO we ***really*** need to re-do the whole scrollbar thing.
	// TODO we have an addScrollListener() using an m_pScrollObj
	// TODO and a View-Listener, and a bunch of other widget stuff.
	// TODO and its very confusing.
	
	pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
	UT_ASSERT(pScrollObj);
	pViewListener = new ap_ViewListener(this);
	UT_ASSERT(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	UT_ASSERT(pScrollbarViewListener);

	if (!pView->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		goto Cleanup;

	if (!pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		goto Cleanup;

	nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (UT_uint32 k=0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.
		
		EV_MacToolbar * pMacToolbar = (EV_MacToolbar *)m_vecToolbars.getNthItem(k);
		pMacToolbar->bindListenerToView(pView);
	}

	/****************************************************************
	*****************************************************************
	** If we reach this point, everything for the new document has
	** been created.  We can now safely replace the various fields
	** within the structure.  Nothing below this point should fail.
	*****************************************************************
	****************************************************************/
	
	// switch to new view, cleaning up previous settings
	if (((AP_FrameData*)m_pData)->m_pDocLayout)
	{
		pOldDoc = ((AP_FrameData*)m_pData)->m_pDocLayout->getDocument();
	}

	REPLACEP(((AP_FrameData*)m_pData)->m_pG, pG);
	REPLACEP(((AP_FrameData*)m_pData)->m_pDocLayout, pDocLayout);
	if (pOldDoc != m_pDoc)
	{
		UNREFP(pOldDoc);
	}
	REPLACEP(m_pView, pView);
        if(getApp()->getViewSelection())
	       getApp()->setViewSelection(pView);
	REPLACEP(m_pScrollObj, pScrollObj);
	REPLACEP(m_pViewListener, pViewListener);
	m_lid = lid;
	REPLACEP(m_pScrollbarViewListener,pScrollbarViewListener);
	m_lidScrollbarViewListener = lidScrollbarViewListener;

	m_pView->addScrollListener(m_pScrollObj);

	// Associate the new view with the existing TopRuler, LeftRuler.
	// Because of the binding to the actual on-screen widgets we do
	// not destroy and recreate the TopRuler, LeftRuler when we change
	// views, like we do for all the other objects.  We also do not
	// allocate the TopRuler, LeftRuler  here; that is done as the
	// frame is created.
	if ( ((AP_FrameData*)m_pData)->m_bShowRuler )
	{
	  if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
	    ((AP_FrameData*)m_pData)->m_pTopRuler->setView(pView, iZoom);
	  if ( ((AP_FrameData*)m_pData)->m_pLeftRuler )
		((AP_FrameData*)m_pData)->m_pLeftRuler->setView(pView, iZoom);
	}

	if ( ((AP_FrameData*)m_pData)->m_pStatusBar )
	  ((AP_FrameData*)m_pData)->m_pStatusBar->setView(pView);

	pView->setInsertMode(((AP_FrameData*)m_pData)->m_bInsertMode);
    ((FV_View *) m_pView)->setShowPara(((AP_FrameData*)m_pData)->m_bShowPara);
	
	m_pView->setWindowSize(m_winBounds.bottom - m_winBounds.top, 
                               m_winBounds.right - m_winBounds.left);
	setXScrollRange();
	setYScrollRange();
	updateTitle();

	if (point != 0)
		((FV_View *) m_pView)->moveInsPtTo(point);
#if 1
	/*
	  UPDATE:  this code is back, but I'm leaving these comments as
	  an audit trail.  See bug 99.  This only happens when loading
	  a document into an empty window -- the case where a frame gets
	  reused.  TODO consider putting an expose into ap_EditMethods.cpp
	  instead of a draw() here.
	*/
	
	/*
	  I've removed this once again.  (Eric)  I replaced it with a call
	  to draw() which is now in the configure event handler in the GTK
	  section of the code.  See me if this causes problems.
	*/
	m_pView->draw();
#endif	
	
	if ( ((AP_FrameData*)m_pData)->m_bShowRuler  ) 
	{
		if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
			((AP_FrameData*)m_pData)->m_pTopRuler->draw(NULL);

		if ( ((AP_FrameData*)m_pData)->m_pLeftRuler )
			((AP_FrameData*)m_pData)->m_pLeftRuler->draw(NULL);
	}

	((AP_FrameData*)m_pData)->m_pStatusBar->draw();
	
	return UT_OK;

Cleanup:
	// clean up anything we created here
	DELETEP(pG);
	DELETEP(pDocLayout);
	DELETEP(pView);
	DELETEP(pViewListener);
	DELETEP(pScrollObj);
	DELETEP(pScrollbarViewListener);

	// change back to prior document
	UNREFP(m_pDoc);
	m_pDoc = ((AP_FrameData*)m_pData)->m_pDocLayout->getDocument();

	return UT_IE_ADDLISTENERERROR;
}


UT_Error AP_MacFrame::_loadDocument(const char * szFilename, IEFileType ieft)
{
	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document();
	UT_ASSERT(pNewDoc);
	
	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		m_iUntitled = _getNextUntitledNumber();
		goto ReplaceDocument;
	}
	UT_Error errorCode;
	errorCode = pNewDoc->readFromFile(szFilename, ieft);
	if (!errorCode)
		goto ReplaceDocument;
	
	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	UNREFP(pNewDoc);
	return errorCode;

ReplaceDocument:
	getApp()->forgetClones(this);

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}

UT_Error AP_MacFrame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
}


void AP_MacFrame::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 /*yrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_MacFrame * pMacFrame = static_cast<AP_MacFrame *>(pData);
	AV_View * pView = pMacFrame->getCurrentView();
	
	// we've been notified (via sendVerticalScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.
#if 0
	gfloat yoffNew = (gfloat)yoff;
	gfloat yoffMax = pUnixFrame->m_pVadj->upper - pUnixFrame->m_pVadj->page_size;
	if (yoffMax <= 0)
		yoffNew = 0;
	else if (yoffNew > yoffMax)
		yoffNew = yoffMax;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(pUnixFrame->m_pVadj),yoffNew);
	pView->setYScrollOffset((UT_sint32)yoffNew);
#endif
}


void AP_MacFrame::_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 /*xrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_MacFrame * pMacFrame = static_cast<AP_MacFrame *>(pData);
	AV_View * pView = pMacFrame->getCurrentView();
	
	// we've been notified (via sendScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.
#if 0
	gfloat xoffNew = (gfloat)xoff;
	gfloat xoffMax = pUnixFrame->m_pHadj->upper - pUnixFrame->m_pHadj->page_size;
	if (xoffMax <= 0)
		xoffNew = 0;
	else if (xoffNew > xoffMax)
		xoffNew = xoffMax;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(pUnixFrame->m_pHadj),xoffNew);
	pView->setXScrollOffset((UT_sint32)xoffNew);
#endif
}


void AP_MacFrame::toggleTopRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);
		
	AP_MacTopRuler * pMacTopRuler = NULL;

	UT_DEBUGMSG(("AP_MacTopRuler::toggleTopRuler %d, %d\n", 
		     bRulerOn, pFrameData->m_pTopRuler));

	if ( bRulerOn )
	{
		UT_ASSERT(!pFrameData->m_pTopRuler);

		pMacTopRuler = new AP_MacTopRuler(this);
		UT_ASSERT(pMacTopRuler);
//		m_topRuler = pMacTopRuler->createWidget();

		// get the width from the left ruler and stuff it into the 
		// top ruler.

//		if (((AP_FrameData*)m_pData)->m_pLeftRuler)
//		  pMacTopRuler->setOffsetLeftRuler(((AP_FrameData*)m_pData)->m_pLeftRuler->getWidth());
//		else
//		  pMacTopRuler->setOffsetLeftRuler(0);

		// attach everything	
        UT_DEBUGMSG(("Actually do the stuff: %s:%d\n", __FILE__, __LINE__));
//		gtk_table_attach(GTK_TABLE(m_innertable), m_topRuler, 0, 2, 0,
//				 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
//				 (GtkAttachOptions)(GTK_FILL),
//				 0, 0);

		pMacTopRuler->setView(m_pView);
	}
	else {
		// delete the actual widgets
//		gtk_object_destroy( GTK_OBJECT(m_topRuler) );
        UT_DEBUGMSG(("Actually do the stuff: %s:%d\n", __FILE__, __LINE__));	
        DELETEP(((AP_FrameData*)m_pData)->m_pTopRuler);
//		m_topRuler = NULL;
    }

	((AP_FrameData*)m_pData)->m_pTopRuler = pMacTopRuler;
}

void AP_MacFrame::toggleLeftRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);

	AP_MacLeftRuler * pMacLeftRuler = NULL;

	UT_DEBUGMSG(("AP_MacFrame::toggleLeftRuler %d, %d\n", 
		     bRulerOn, pFrameData->m_pLeftRuler));

	if (bRulerOn) {
		pMacLeftRuler = new AP_MacLeftRuler(this);
		UT_ASSERT(pMacLeftRuler);
//		m_leftRuler = pMacLeftRuler->createWidget();

        UT_DEBUGMSG(("Actually do the stuff: %s:%d\n", __FILE__, __LINE__));

//		gtk_table_attach(GTK_TABLE(m_innertable), m_leftRuler, 0, 1, 1, 2,
//				 (GtkAttachOptions)(GTK_FILL),
//				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
//				 0,0);
		pMacLeftRuler->setView(m_pView);
	  }
	else
	  {
//	    if (m_leftRuler && GTK_IS_OBJECT(m_leftRuler))
//		gtk_object_destroy( GTK_OBJECT(m_leftRuler) );
        UT_DEBUGMSG(("Actually do the stuff: %s:%d\n", __FILE__, __LINE__));
	    
	    DELETEP(((AP_FrameData*)m_pData)->m_pLeftRuler);
//	    m_leftRuler = NULL;
	  }

	((AP_FrameData*)m_pData)->m_pLeftRuler = pMacLeftRuler;
}
