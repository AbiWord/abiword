/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "ap_FrameData.h"
#include "xap_BeOSFrame.h"
#include "ev_BeOSToolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_BeOSGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_BeOSFrame.h"
#include "xap_BeOSApp.h"
#include "ap_BeOSTopRuler.h"
#include "ap_BeOSLeftRuler.h"
#include "ap_Prefs.h"
#include "ap_BeOSStatusBar.h"

#include "ev_BeOSKeyboard.h"
#include "ev_BeOSMouse.h"


/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

void AP_BeOSFrame::setZoomPercentage(UT_uint32 iZoom)
{
        _showDocument(iZoom);
	if (m_pBeDocView->Window()->Lock())
	{
		m_pBeDocView->Invalidate();
		m_pBeDocView->Window()->Unlock();
	}
}

UT_uint32 AP_BeOSFrame::getZoomPercentage(void)
{
        return ((AP_FrameData*)m_pData)->m_pG->getZoomPercentage();
}            

UT_Error AP_BeOSFrame::_showDocument(UT_uint32 iZoom)
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

	GR_BeOSGraphics * pG = NULL;
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
	
	//pG = new GR_BeOSGraphics(m_dArea->window, fontManager);
	pG = new GR_BeOSGraphics(getBeDocView(), getApp());
	ENSUREP(pG);
	pG->setZoomPercentage(iZoom);

	pDocLayout = new FL_DocLayout(static_cast<PD_Document *>(m_pDoc), pG);
	ENSUREP(pDocLayout);
  
//	pDocLayout->formatAll();

	pView = new FV_View(getApp(), this, pDocLayout);
	if (m_pView != NULL)
	{
		point = ((FV_View *) m_pView)->getPoint();
	}
	ENSUREP(pView);

	pView->focusChange(AV_FOCUS_HERE);

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
	ENSUREP(pScrollObj);
	pViewListener = new ap_ViewListener(this);
	ENSUREP(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	ENSUREP(pScrollbarViewListener);

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
		
		EV_BeOSToolbar * pBeOSToolbar = (EV_BeOSToolbar *)m_vecToolbars.getNthItem(k);
		pBeOSToolbar->bindListenerToView(pView);
		
		// We need to put the pointers to our toolbars into the frame data,
		// for use by the show/hide mechanism.
		UT_DEBUGMSG(("Inseting Toolbar %d into array\n", k));
		printf("Inseting Toolbar %d into array\n", k);
		static_cast<AP_FrameData *>(m_pData)->m_pToolbar[k] = pBeOSToolbar;
	}

	/****************************************************************
	*****************************************************************
	** If we reach this point, everything for the new document has
	** been created.  We can now safely replace the various fields
	** within the structure.  Nothing below this point should fail.
	*****************************************************************
	****************************************************************/
	
	// switch to new view, cleaning up previous settings
	if (((AP_FrameData*)m_pData)->m_pDocLayout) {
                pOldDoc = ((AP_FrameData*)m_pData)->m_pDocLayout->getDocument();
        }

		delete ((AP_FrameData*)m_pData)->m_pDocLayout;
        REPLACEP(((AP_FrameData*)m_pData)->m_pG, pG);
        ((AP_FrameData*)m_pData)->m_pDocLayout = pDocLayout;

        if (pOldDoc != m_pDoc) {
                UNREFP(pOldDoc);
        }       

	REPLACEP(m_pView, pView);
	REPLACEP(m_pScrollObj, pScrollObj);
	REPLACEP(m_pViewListener, pViewListener);
	m_lid = lid;
	REPLACEP(m_pScrollbarViewListener,pScrollbarViewListener);
	//m_lidScrollbarViewListener = lidScrollbarViewListener;

	m_pView->addScrollListener(m_pScrollObj);

	// Associate the new view with the existing TopRuler, LeftRuler.
	// Because of the binding to the actual on-screen widgets we do
	// not destroy and recreate the TopRuler, LeftRuler when we change
	// views, like we do for all the other objects.  We also do not
	// allocate the TopRuler, LeftRuler  here; that is done as the
	// frame is created.


	pView->setInsertMode(((AP_FrameData*)m_pData)->m_bInsertMode);
	((FV_View *) m_pView)->setShowPara(((AP_FrameData*)m_pData)->m_bShowPara);

	m_pBeDocView->Window()->Lock();
	m_pView->setWindowSize(m_pBeDocView->Bounds().Width() + 1,
			       m_pBeDocView->Bounds().Height() + 1);
	m_pBeDocView->Window()->Unlock();
	
	m_pBeDocView->Window()->PostMessage('inme');
	
	setXScrollRange();
	setYScrollRange();
	updateTitle();

	pDocLayout->fillLayouts();
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
 
//Toolbars Show/Hide

	for (UT_uint32 k=0; k < nrToolbars; k++)
	{
		m_pBeDocView->Window()->Lock();
		if(! static_cast<AP_FrameData *>(m_pData)->m_bShowBar[k] ) {
			toggleBar(k, false);
		}
		m_pBeDocView->Window()->Unlock();
	}
	

//Ruler Show/Hide

	if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
	{
		((AP_FrameData*)m_pData)->m_pTopRuler->setView(pView, iZoom);
		((AP_FrameData*)m_pData)->m_pTopRuler->draw(NULL);
	}

	if ( ((AP_FrameData*)m_pData)->m_pLeftRuler )
	{
		((AP_FrameData*)m_pData)->m_pLeftRuler->setView(pView, iZoom);
		((AP_FrameData*)m_pData)->m_pLeftRuler->draw(NULL);
	}

	if (! ((AP_FrameData*)m_pData)->m_bShowRuler )
	{
		m_pBeDocView->Window()->Lock();
		toggleRuler(false);
		m_pBeDocView->Window()->Unlock();
	}
	
//StatusBar Show/Hide

	if (((AP_FrameData*)m_pData)->m_pStatusBar)
	{
		((AP_FrameData*)m_pData)->m_pStatusBar->setView(pView);
		((AP_FrameData*)m_pData)->m_pStatusBar->draw();
	}
	if (! ((AP_FrameData*)m_pData)->m_bShowStatusBar)
	{
		m_pBeDocView->Window()->Lock();
		toggleStatusBar(false);
		m_pBeDocView->Window()->Unlock();
	}

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

/*
 This function actually sets up the value of the scroll bar in 
 terms of it max/min and step size, and will set an initial
 value of the widget by calling the scroll event which
 will eventually call _scrollFuncX()
*/
void AP_BeOSFrame::setXScrollRange(void)
{
	int width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	be_Window *pBWin = (be_Window*)getTopLevelWindow();

	if (!pBWin->Lock()) {
		return;
	}

    int windowWidth = (int)pBWin->m_pbe_DocView->Bounds().Width();
    pBWin->Unlock();

	int newvalue = (m_pView) ? m_pView->getXScrollOffset() : 0;
	/* This is a real dilemma ... should the maximum of the
       scrollbar be set to the document width, or should it
       be set to the document width - window width since that
       is all you are going to scroll? I think it should be
       set to width - window width myself.  */
	int newmax = width - windowWidth; /* upper - page_size */ 
	int differentPosition = 0;

	/* Adjust the bounds of maximum and value as required */
	if (newmax <= 0) {
		newvalue = newmax = 0;
	}
	else if (newvalue > newmax) {
		newvalue = newmax;
	}

    BScrollBar *hscroll = pBWin->m_hScroll;
	if (hscroll->Window()->Lock()) {
    	hscroll->SetSteps(20.0, windowWidth);
    	hscroll->SetRange(0, newmax);
		differentPosition = (hscroll->Value() != newvalue);
    	hscroll->Window()->Unlock(); 
	}

	if (m_pView && differentPosition /*|| differentLimits */) {
		m_pView->sendHorizontalScrollEvent(newvalue);
	}
}

void AP_BeOSFrame::setYScrollRange(void)
{
	int height = ((AP_FrameData*)m_pData)->m_pDocLayout->getHeight();
	be_Window *pBWin = (be_Window*)getTopLevelWindow();

    if (!pBWin->Lock()) {
		return;
	}
    int windowHeight = (int)pBWin->m_pbe_DocView->Bounds().Height();
    pBWin->Unlock();

	int newvalue = (m_pView) ? m_pView->getYScrollOffset() : 0;
	int newmax = height - windowHeight; /* upper - page_size */ 
	int differentPosition = 0;

	/* Adjust the bounds of maximum and value as required */
	if (newmax <= 0) {
		newvalue = newmax = 0;
	}
	else if (newvalue > newmax) {
		newvalue = newmax;
	}

    BScrollBar *vscroll = pBWin->m_vScroll;
    if (vscroll->Window()->Lock()) {
    	vscroll->SetSteps(20.0, windowHeight);
    	vscroll->SetRange(0, newmax);
		differentPosition = (vscroll->Value() != newvalue);
    	vscroll->Window()->Unlock();        
	}

	if (m_pView && differentPosition /*|| differentLimits */) {
		m_pView->sendVerticalScrollEvent(newvalue);
	}
}


AP_BeOSFrame::AP_BeOSFrame(XAP_BeOSApp * app)
	: XAP_BeOSFrame(app)
{
	// TODO
}

AP_BeOSFrame::AP_BeOSFrame(AP_BeOSFrame * f)
	: XAP_BeOSFrame(static_cast<XAP_BeOSFrame *>(f))
{
	// TODO
}

AP_BeOSFrame::~AP_BeOSFrame()
{
	killFrameData();
}

bool AP_BeOSFrame::initialize()
{
	if (!initFrameData())
		return false;

	if (!XAP_BeOSFrame::initialize(AP_PREF_KEY_KeyBindings, AP_PREF_DEFAULT_KeyBindings,
								   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
								   AP_PREF_KEY_MenuLabelSet, AP_PREF_DEFAULT_MenuLabelSet,
								   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
								   AP_PREF_KEY_ToolbarLabelSet, AP_PREF_DEFAULT_ToolbarLabelSet))
		return false;

	_createTopLevelWindow();

	//At this point in time the BeOS widgets are all
	//realized so we should be able to go and attach
	//the various input filters to them.
	ev_BeOSKeyboard * pBeOSKeyboard = static_cast<ev_BeOSKeyboard *>(m_pKeyboard);
	pBeOSKeyboard->synthesize(m_pBeOSApp, this);

	ev_BeOSMouse * pBeOSMouse = static_cast<ev_BeOSMouse *>(m_pMouse);
	pBeOSMouse->synthesize(m_pBeOSApp, this);

	//Actually show the window to the world
	m_pBeWin->Show();
 	//getTopLevelWindow()->Show();

	return true;
}

/*****************************************************************/

bool AP_BeOSFrame::initFrameData(void)
{
	UT_ASSERT(!m_pData);

	m_pData = new AP_FrameData(m_pBeOSApp);
	
	return (m_pData ? true : false);
}

void AP_BeOSFrame::killFrameData(void)
{
	DELETEP(m_pData);
	m_pData = NULL;
}

UT_Error AP_BeOSFrame::_loadDocument(const char * szFilename, IEFileType ieft,
				     bool createNew)
	
{
	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document(getApp());
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

	if (createNew)
	  {
	    // we have a file name but couldn't load it
	    pNewDoc->newDocument();

	    // here, we want to open a new document if it doesn't exist.
	    // errorCode could also take several other values, indicating
	    // that the document exists but for some reason we could not
	    // open it. in those cases, we do not wish to overwrite the
	    // existing documents, but instead open a new blank document.
	    // this fixes bug 1668 - DAL
	    if ( UT_IE_FILENOTFOUND == errorCode )
	      errorCode = pNewDoc->saveAs(szFilename, ieft);
	  }
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

UT_Error AP_BeOSFrame::importDocument(const char * szFilename, int ieft, bool markClean) {
	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document(getApp());
	UT_ASSERT(pNewDoc);
	
	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		m_iUntitled = _getNextUntitledNumber();
		goto ReplaceDocument;
	}
	UT_Error errorCode;
	errorCode = pNewDoc->importFile(szFilename, ieft, markClean);
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
	
XAP_Frame * AP_BeOSFrame::buildFrame(XAP_Frame * pF)
{
	UT_Error error = UT_OK;
	AP_BeOSFrame * pFrame = static_cast<AP_BeOSFrame *>(pF);
	ENSUREP(pFrame);
	return pFrame;

	if (!pFrame->initialize())
		goto Cleanup;

	error = pFrame->_showDocument();
	if (error)
		goto Cleanup;

	pFrame->show();
	return static_cast<XAP_Frame *>(pFrame);
								                             
Cleanup:
	// clean up anything we created here
	if (pFrame)
	{
		m_pBeOSApp->forgetFrame(pFrame);
		delete pFrame;
	}

	return NULL;
}

XAP_Frame * AP_BeOSFrame::cloneFrame(void)
{
	AP_BeOSFrame * pClone = new AP_BeOSFrame(this);
	ENSUREP(pClone);
	return pClone;

Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		m_pBeOSApp->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}
	
UT_Error AP_BeOSFrame::loadDocument(const char * szFilename, int ieft)
{
  return loadDocument(szFilename, ieft, false);
}

UT_Error AP_BeOSFrame::loadDocument(const char * szFilename, int ieft, bool createNew)
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
	err = _loadDocument(szFilename, (IEFileType) ieft, createNew); 
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
			AP_BeOSFrame * pFrame = (AP_BeOSFrame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}


/*
  We've been notified (via sendScrollEvent()) of a scroll (probably
  a keyboard motion or user scroll event).  push the new values into 
  the scrollbar widgets (with clamping).  Then cause the view to scroll.
*/
void AP_BeOSFrame::_scrollFuncX(void * pData, 
								UT_sint32 xoff, 
								UT_sint32 /*xrange*/)
{
        // this is a static callback function and doesn't have a 'this' pointer.
        AP_BeOSFrame * pBeOSFrame = static_cast<AP_BeOSFrame *>(pData);
        AV_View * pView = pBeOSFrame->getCurrentView();

		//Actually set the scroll bar value ...
		pView->setXScrollOffset(xoff);

    	BScrollBar *hscroll = pBeOSFrame->m_hScroll;
    	if (hscroll->Window()->Lock()) {
			float min, max;
			hscroll->GetRange(&min, &max);
			if (xoff < min)	xoff = (UT_sint32)min;
			if (xoff > max)	xoff = (UT_sint32)max;
  			hscroll->SetValue(xoff);
    		hscroll->Window()->Unlock(); 
		}
}
void AP_BeOSFrame::_scrollFuncY(void * pData, 
								UT_sint32 yoff, 
								UT_sint32 /*yrange*/)
{
        // this is a static callback function and doesn't have a 'this' pointer.
        AP_BeOSFrame * pBeOSFrame = static_cast<AP_BeOSFrame *>(pData);
        AV_View * pView = pBeOSFrame->getCurrentView();

		//Actually set the scroll bar value ...
		pView->setYScrollOffset(yoff);

    	BScrollBar *vscroll = pBeOSFrame->m_vScroll;
    	if (vscroll->Window()->Lock()) {
			float min, max;
			vscroll->GetRange(&min, &max);
			if (yoff < min)	yoff = (UT_sint32)min;
			if (yoff > max)	yoff = (UT_sint32)max;
  			vscroll->SetValue(yoff);
    		vscroll->Window()->Unlock(); 
		}
}

void AP_BeOSFrame::setStatusMessage(const char * szMsg)
{
	((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}                                                                        


/*****
 This is here as an abstraction of the Be _createDocumentWindow() code
 which sits in the xap code.  In the worse case we just create a
 plain view, in the best case we actually put neat stuff here
*****/
be_DocView *be_Window::_createDocumentWindow() 
{
	BRect r;
	
    //Set up the scroll bars on the outer edges of the document area
    r = m_winRectAvailable;
    r.bottom -= (B_H_SCROLL_BAR_HEIGHT + 1 + STATUS_BAR_HEIGHT);
    r.left = r.right - B_V_SCROLL_BAR_WIDTH;
    m_vScroll = new TFScrollBar(m_pBeOSFrame, r,
                                "VertScroll", NULL, 0, 100, B_VERTICAL);
    AddChild(m_vScroll);

    r = m_winRectAvailable;
    r.top = r.bottom - (B_H_SCROLL_BAR_HEIGHT + 1 + STATUS_BAR_HEIGHT);
    r.bottom-=(1 + STATUS_BAR_HEIGHT);
    r.right -= B_V_SCROLL_BAR_WIDTH;
    m_hScroll = new TFScrollBar(m_pBeOSFrame, r,
                                "HortScroll", NULL, 0, 100, B_HORIZONTAL);
    AddChild(m_hScroll);
    m_pBeOSFrame->setScrollBars(m_hScroll, m_vScroll);
    m_winRectAvailable.bottom -= (B_H_SCROLL_BAR_HEIGHT + 2 + STATUS_BAR_HEIGHT);
    m_winRectAvailable.right -= B_V_SCROLL_BAR_WIDTH +1;

	//Create the Top and Left Rulers (need a width here)
#define TOP_HEIGHT 32
#define LEFT_WIDTH 32
	// create the top ruler
	r = m_winRectAvailable;
	r.bottom = r.top + TOP_HEIGHT - 1;
	AP_BeOSTopRuler * pBeOSTopRuler = new AP_BeOSTopRuler(m_pBeOSFrame);
	UT_ASSERT(pBeOSTopRuler);
	pBeOSTopRuler->createWidget(r);
	((AP_FrameData*)m_pBeOSFrame->getFrameData())->m_pTopRuler = pBeOSTopRuler;
	m_winRectAvailable.top = r.bottom + 1;

	// create the left ruler
	r = m_winRectAvailable;
	r.right = r.left + LEFT_WIDTH - 1;
	AP_BeOSLeftRuler * pBeOSLeftRuler = new AP_BeOSLeftRuler(m_pBeOSFrame);
	UT_ASSERT(pBeOSLeftRuler);
	pBeOSLeftRuler->createWidget(r);
	((AP_FrameData*)m_pBeOSFrame->getFrameData())->m_pLeftRuler = pBeOSLeftRuler;
	m_winRectAvailable.left = r.right + 1;

	// get the width from the left ruler and stuff it into the top ruler.
	pBeOSTopRuler->setOffsetLeftRuler(pBeOSLeftRuler->getWidth());

	//Add the document view in the remaining space
	m_pbe_DocView = new be_DocView(m_winRectAvailable, "MainDocView",
                                   B_FOLLOW_ALL, B_WILL_DRAW);
	//m_pbe_DocView->SetViewColor(0,120, 255);
	//m_pbe_DocView->SetViewColor(B_TRANSPARENT_32_BIT);
	//Add the view to both frameworks (Be and Abi)
	AddChild(m_pbe_DocView);
	m_pBeOSFrame->setBeDocView(m_pbe_DocView);

	//Without this we never get any key inputs
	m_pbe_DocView->WindowActivated(true); // So the cursor shows up.
	m_pbe_DocView->MakeFocus(true);
	
	return(m_pbe_DocView);                                    
}

BView * be_Window::_createStatusBarWindow() 
{
	AP_BeOSStatusBar *pStatusBar = new AP_BeOSStatusBar(m_pBeOSFrame);
	BView *pStatusBarView;
	UT_ASSERT(pStatusBar);
	static_cast<AP_FrameData*>(m_pBeOSFrame->m_pData)->m_pStatusBar = pStatusBar;
	BRect r;
	r = Bounds();
	r.top = r.bottom - STATUS_BAR_HEIGHT;
	pStatusBarView = pStatusBar->createWidget(r);
	AddChild(pStatusBarView);
	
	return pStatusBarView;	
}	

UT_Error AP_BeOSFrame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
}

void AP_BeOSFrame::toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleBar %d\n", bBarOn));

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);
	UT_ASSERT(pFrameData->m_pToolbar);
	
	EV_Toolbar *pToolbar = pFrameData->m_pToolbar[iBarNb];
	
	UT_ASSERT(pToolbar);

	int height = 36;//tempolary

	if (bBarOn)
	{
		pToolbar->show();
		height *= -1;
		printf("Show Toolbar #%d\n", iBarNb);
	}
	else	// turning toolbar off
	{
		pToolbar->hide();
		printf("Hide Toolbar #%d\n", iBarNb);
	}

	m_pBeWin->FindView("TopRuler")->MoveBy(0, height * -1);
	m_pBeWin->FindView("LeftRuler")->MoveBy(0, height * -1);
	m_pBeWin->FindView("LeftRuler")->ResizeBy(0, height);

	m_pBeWin->Lock();
	be_DocView *pView = getBeDocView();

	if(pView)
	{
		pView->MoveBy(0, height * -1);
		pView->ResizeBy(0, height);

		if (iBarNb = 0)
		{
			pFrameData->m_pToolbar[0]->moveby(height * - 1);//NOTE Why? different 1
			pFrameData->m_pToolbar[2]->moveby(height * - 1);
		}
		if (iBarNb = 1)
			pFrameData->m_pToolbar[2]->moveby(height * - 1);

	}
	m_pBeWin->Unlock();
	m_vScroll->MoveBy(0, height * -1);
	m_vScroll->ResizeBy(0, height);
}

void AP_BeOSFrame::toggleStatusBar(bool bStatusBarOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleStatusBar %d\n", bStatusBarOn));

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);

	m_pBeWin->Lock();

	int height = STATUS_BAR_HEIGHT;
	
	if (bStatusBarOn)
	{
		pFrameData->m_pStatusBar->show();
		height *= -1;
		printf("Show Statusbar\n");
	}
	else    // turning status bar off
	{
		pFrameData->m_pStatusBar->hide();
		printf("Hide Statusbar\n");
	}

	be_DocView *pView = getBeDocView();
	if(pView)
	{
		pView->ResizeBy(0, height);
	}
	m_pBeWin->Unlock();
	m_pBeWin->FindView("LeftRuler")->ResizeBy(0, height);
	m_hScroll->MoveBy(0, height);
	m_vScroll->ResizeBy(0, height);
}

void AP_BeOSFrame::toggleRuler(bool bRulerOn)
{
	toggleTopRuler(bRulerOn);
	toggleLeftRuler(bRulerOn);
}

void AP_BeOSFrame::toggleTopRuler(bool bRulerOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleRuler %d", bRulerOn));	
	UT_ASSERT(pFrameData);

	be_DocView *pView = getBeDocView();
	BRect rect = m_pBeWin->FindView("TopRuler")->Frame();
	int height = (int)rect.Height() + 1;

	if (bRulerOn)
	{
		printf("Show Top Ruler\n");
		m_pBeWin->FindView("TopRuler")->Show();
		if(pView)
		{
			pView->ResizeBy(0, height * -1);
			pView->MoveBy(0, height);
		}
	}
	else
	{
		printf("Hide Top Ruler\n");
		m_pBeWin->FindView("TopRuler")->Hide();
		if(pView)
		{
			pView->ResizeBy(0, height);
			pView->MoveBy(0, height * -1);
		}
	}
}

void AP_BeOSFrame::toggleLeftRuler(bool bRulerOn)
{
	UT_DEBUGMSG(("AP_BeOSFrame::toggleRuler %d", bRulerOn));
	UT_ASSERT(pFrameData);

	be_DocView *pView = getBeDocView();
	BRect rect = m_pBeWin->FindView("LeftRuler")->Frame();
	int width = (int)rect.Width() + 1;

	if (bRulerOn)
	{
		printf("Show Left Ruler\n");
		m_pBeWin->FindView("LeftRuler")->Show();
		if(pView)
		{
			pView->ResizeBy(width * -1, 0);
			pView->MoveBy(width, 0);
		}
	}
	else
	{
		printf("Hide Left Ruler\n");
		m_pBeWin->FindView("LeftRuler")->Hide();
		if(pView)
		{
			pView->ResizeBy(width , 0);
			pView->MoveBy(width * -1, 0);
		}
	}
}

void AP_BeOSFrame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{ 
	// translate the given document mouse coordinates into absolute screen coordinates.

	BPoint pt(x,y);// = { x, y };

	m_pBeWin->Lock();
	
	be_DocView *pView = getBeDocView();
	
	if(pView)
	{
		pView->ConvertToScreen(&pt);
	}
	
	m_pBeWin->Unlock();
	
	x = (UT_sint32)pt.x;
	y = (UT_sint32)pt.y;
}

