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

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

void AP_BeOSFrame::setZoomPercentage(UT_uint32 iZoom)
{
        _showDocument(iZoom);
}

UT_uint32 AP_BeOSFrame::getZoomPercentage(void)
{
        return ((AP_FrameData*)m_pData)->m_pG->getZoomPercentage();
}            

UT_Bool AP_BeOSFrame::_showDocument(UT_uint32 iZoom)
{
	if (!m_pDoc)
	{
		UT_DEBUGMSG(("Can't show a non-existent document\n"));
		return UT_FALSE;
	}

	if (!((AP_FrameData*)m_pData))
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_FALSE;
	}

	GR_BEOSGraphics * pG = NULL;
	FL_DocLayout * pDocLayout = NULL;
	AV_View * pView = NULL;
	AV_ScrollObj * pScrollObj = NULL;
	ap_ViewListener * pViewListener = NULL;
	AD_Document * pOldDoc = NULL;
	ap_Scrollbar_ViewListener * pScrollbarViewListener = NULL;
	AV_ListenerId lid;
	AV_ListenerId lidScrollbarViewListener;
	UT_uint32 nrToolbars;

	
	//pG = new GR_BEOSGraphics(m_dArea->window, fontManager);
	pG = new GR_BEOSGraphics(getBeDocView());
	ENSUREP(pG);
	pG->setZoomPercentage(iZoom);

	pDocLayout = new FL_DocLayout(static_cast<PD_Document *>(m_pDoc), pG);
	ENSUREP(pDocLayout);
  
//	pDocLayout->formatAll();

	pView = new FV_View(getApp(), this, pDocLayout);
	ENSUREP(pView);

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
		
		EV_BeOSToolbar * pBeOSToolbar = (EV_BeOSToolbar *)m_vecBeOSToolbars.getNthItem(k);
		pBeOSToolbar->bindListenerToView(pView);
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

        REPLACEP(((AP_FrameData*)m_pData)->m_pG, pG);
        REPLACEP(((AP_FrameData*)m_pData)->m_pDocLayout, pDocLayout);
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
	((AP_FrameData*)m_pData)->m_pTopRuler->setView(pView);
	((AP_FrameData*)m_pData)->m_pLeftRuler->setView(pView);
	//((AP_FrameData*)m_pData)->m_pStatusBar->setView(pView);
	
	m_pBeDocView->Window()->Lock();
	m_pView->setWindowSize(m_pBeDocView->Bounds().Width(),
			       m_pBeDocView->Bounds().Height());
	m_pBeDocView->Window()->Unlock();

	setXScrollRange();
	setYScrollRange();
	updateTitle();

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
	((AP_FrameData*)m_pData)->m_pTopRuler->draw(NULL);
        ((AP_FrameData*)m_pData)->m_pLeftRuler->draw(NULL);
	//((AP_FrameData*)m_pData)->m_pStatusBar->draw();

	return UT_TRUE;

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

	return UT_FALSE;
}

void AP_BeOSFrame::setXScrollRange(void)
{
#if 0
	int width = m_pData->m_pDocLayout->getWidth();
	int windowWidth = GTK_WIDGET(m_dArea)->allocation.width;

	int newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	int newmax = width - windowWidth; /* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	UT_Bool bDifferentPosition = (newvalue != (int)m_pHadj->value);
	UT_Bool bDifferentLimits = ((width-windowWidth) != (m_pHadj->upper-m_pHadj->page_size));
	
	m_pHadj->value = newvalue;
	m_pHadj->lower = 0.0;
	m_pHadj->upper = (gfloat) width;
	m_pHadj->step_increment = 20.0;
	m_pHadj->page_increment = (gfloat) windowWidth;
	m_pHadj->page_size = (gfloat) windowWidth;
	gtk_signal_emit_by_name(GTK_OBJECT(m_pHadj), "changed");

	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendHorizontalScrollEvent(newvalue,m_pHadj->upper-m_pHadj->page_size);
#endif
	int width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	be_Window *pBWin = (be_Window*)getTopLevelWindow();
        pBWin->Lock();
        int windowWidth = pBWin->m_pbe_DocView->Bounds().Width();
        pBWin->Unlock();
        BScrollBar *hscroll = pBWin->m_hScroll;

        hscroll->Window()->Lock();
        hscroll->SetSteps(20.0, windowWidth);
        hscroll->SetRange(0, width);
        hscroll->SetValue((m_pView) ? m_pView->getXScrollOffset() : 0);
        hscroll->Window()->Unlock(); 
}

void AP_BeOSFrame::setYScrollRange(void)
{
#if 0
	int height = m_pData->m_pDocLayout->getHeight();
	int windowHeight = GTK_WIDGET(m_dArea)->allocation.height;

	int newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	int newmax = height - windowHeight;	/* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	UT_Bool bDifferentPosition = (newvalue != (int)m_pVadj->value);
	UT_Bool bDifferentLimits ((height-windowHeight) != (m_pVadj->upper-m_pVadj->page_size));
	
	m_pVadj->value = newvalue;
	m_pVadj->lower = 0.0;
	m_pVadj->upper = (gfloat) height;
	m_pVadj->step_increment = 20.0;
	m_pVadj->page_increment = (gfloat) windowHeight;
	m_pVadj->page_size = (gfloat) windowHeight;
	gtk_signal_emit_by_name(GTK_OBJECT(m_pVadj), "changed");

	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendVerticalScrollEvent(newvalue,m_pVadj->upper-m_pVadj->page_size);
#endif
	int height = ((AP_FrameData*)m_pData)->m_pDocLayout->getHeight();
	be_Window *pBWin = (be_Window*)getTopLevelWindow();
        pBWin->Lock();
        int windowHeight = pBWin->m_pbe_DocView->Bounds().Height();
        pBWin->Unlock();
        BScrollBar *vscroll = pBWin->m_vScroll;

        vscroll->Window()->Lock();
        vscroll->SetSteps(20.0, windowHeight);
        vscroll->SetRange(0, height);
        vscroll->SetValue((m_pView) ? m_pView->getYScrollOffset() : 0);
        vscroll->Window()->Unlock();        
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

UT_Bool AP_BeOSFrame::initialize()
{
	if (!initFrameData())
		return UT_FALSE;

	if (!XAP_BeOSFrame::initialize(AP_PREF_KEY_KeyBindings,
				       AP_PREF_DEFAULT_KeyBindings,
                                       AP_PREF_KEY_MenuLayout, 
				       AP_PREF_DEFAULT_MenuLayout,
                                       AP_PREF_KEY_MenuLabelSet, 
				       AP_PREF_DEFAULT_MenuLabelSet,
                                       AP_PREF_KEY_ToolbarLayouts, 
				       AP_PREF_DEFAULT_ToolbarLayouts,
                                       AP_PREF_KEY_ToolbarLabelSet, 
				       AP_PREF_DEFAULT_ToolbarLabelSet))
		return UT_FALSE;

	_createTopLevelWindow();

	//At this point in time the BeOS widgets are all
	//realized so we should be able to go and attach
	//the various input filters to them.
	m_pBeOSKeyboard->synthesize(m_pBeOSApp, this);
        m_pBeOSMouse->synthesize(m_pBeOSApp, this);

	//Actually show the window to the world
	m_pBeWin->Show();
 	//getTopLevelWindow()->Show();

	return UT_TRUE;
}

/*****************************************************************/

UT_Bool AP_BeOSFrame::initFrameData(void)
{
	UT_ASSERT(!m_pData);

	m_pData = new AP_FrameData();
	
	return (m_pData ? UT_TRUE : UT_FALSE);
}

void AP_BeOSFrame::killFrameData(void)
{
	DELETEP(m_pData);
	m_pData = NULL;
}

UT_Bool AP_BeOSFrame::_loadDocument(const char * szFilename, IEFileType ieft)
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

	if (pNewDoc->readFromFile(szFilename, ieft))
		goto ReplaceDocument;
	
	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	UNREFP(pNewDoc);
	return UT_FALSE;

ReplaceDocument:
	getApp()->forgetClones(this);

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_TRUE;
}
	
XAP_Frame * AP_BeOSFrame::cloneFrame(void)
{
	AP_BeOSFrame * pClone = new AP_BeOSFrame(this);
	ENSUREP(pClone);

	if (!pClone->initialize())
		goto Cleanup;

	if (!pClone->_showDocument())
		goto Cleanup;

	pClone->show();

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

UT_Bool AP_BeOSFrame::loadDocument(const char * szFilename, int ieft)
{
	UT_Bool bUpdateClones;
	UT_Vector vClones;
	XAP_App * pApp = getApp();

	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}

	if (! _loadDocument(szFilename,(IEFileType)ieft))
	{
		// we could not load the document.
		// we cannot complain to the user here, we don't know
		// if the app is fully up yet.  we force our caller
		// to deal with the problem.
		return UT_FALSE;
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

void AP_BeOSFrame::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 /*yrange*/)
{
        // this is a static callback function and doesn't have a 'this' pointer.

        AP_BeOSFrame * pBeOSFrame = static_cast<AP_BeOSFrame *>(pData);
        AV_View * pView = pBeOSFrame->getCurrentView();

        // we've been notified (via sendVerticalScrollEvent()) of a scroll (probably
        // a keyboard motion).  push the new values into the scrollbar widgets
        // (with clamping).  then cause the view to scroll.

        float yoffNew = yoff;
        float yoffMax, max;
	be_Window *pBWin = (be_Window*)pBeOSFrame->getTopLevelWindow();
        pBWin->m_vScroll->GetRange(&yoffMax, &max);
        yoffMax = max - pBWin->m_vScroll->Value();
        if (yoffMax <= 0)
                yoffNew = 0;
        else if (yoffNew > yoffMax)
                yoffNew = yoffMax;
        pView->setYScrollOffset((UT_sint32)yoffNew); 
}

void AP_BeOSFrame::_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 /*xrange*/)
{
        // this is a static callback function and doesn't have a 'this' pointer.

        AP_BeOSFrame * pBeOSFrame = static_cast<AP_BeOSFrame *>(pData);
        AV_View * pView = pBeOSFrame->getCurrentView();

        // we've been notified (via sendScrollEvent()) of a scroll (probably
        // a keyboard motion).  push the new values into the scrollbar widgets
        // (with clamping).  then cause the view to scroll.
        float xoffNew = xoff;
        //float xoffMax = pBeOSFrame->m_pHadj->upper - pBeOSFrame->m_pHadj->page_size;
        float xoffMax, max;
	be_Window *pBWin = (be_Window*)pBeOSFrame->getTopLevelWindow();
        pBWin->m_hScroll->GetRange(&xoffMax, &max);
        xoffMax = max - pBWin->m_hScroll->Value();
        if (xoffMax <= 0)
                xoffNew = 0;
        else if (xoffNew > xoffMax)
                xoffNew = xoffMax;
        pView->setXScrollOffset((UT_sint32)xoffNew); 
}

void AP_BeOSFrame::setStatusMessage(const char * szMsg)
{
	printf("FRAME:Set Status Message not yet supported \n");
//        ((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}                                                                        


/*****
 This is here as an abstraction of the Be _createDocumentWindow() code
 which sits in the xap code.  In the worse case we just create a
 plain view, in the best case we actually put neat stuff here
*****/
be_DocView *be_Window::_createDocumentWindow() {
	BRect r;
	
        //Set up the scroll bars on the outer edges of the document area
        r = m_winRectAvailable;
        r.bottom -= B_H_SCROLL_BAR_HEIGHT;
        r.left = r.right - B_V_SCROLL_BAR_WIDTH;
        m_vScroll = new TFScrollBar(m_pBeOSFrame, r,
                                    "VertScroll", NULL, 0, 100, B_VERTICAL);
        AddChild(m_vScroll);

        r = m_winRectAvailable;
        r.top = r.bottom - B_H_SCROLL_BAR_HEIGHT;
        r.right -= B_V_SCROLL_BAR_WIDTH;
        m_hScroll = new TFScrollBar(m_pBeOSFrame, r,
                                    "HortScroll", NULL, 0, 100, B_HORIZONTAL);
        AddChild(m_hScroll);
        m_pBeOSFrame->setScrollBars(m_hScroll, m_vScroll);
        m_winRectAvailable.bottom -= B_H_SCROLL_BAR_HEIGHT +1;
        m_winRectAvailable.right -= B_V_SCROLL_BAR_WIDTH +1;

	//Create the Top and Left Rulers (need a width here)
#define TOP_HEIGHT 32
#define LEFT_WIDTH 32
	// create the top ruler
	r = m_winRectAvailable;
	r.bottom = r.top + TOP_HEIGHT;
	AP_BeOSTopRuler * pBeOSTopRuler = new AP_BeOSTopRuler(m_pBeOSFrame);
	UT_ASSERT(pBeOSTopRuler);
	pBeOSTopRuler->createWidget(r);
	((AP_FrameData*)m_pBeOSFrame->getFrameData())->m_pTopRuler = pBeOSTopRuler;
	m_winRectAvailable.top = r.bottom +1;

	// create the left ruler
	r = m_winRectAvailable;
	r.right = r.left + LEFT_WIDTH;
	AP_BeOSLeftRuler * pBeOSLeftRuler = new AP_BeOSLeftRuler(m_pBeOSFrame);
	UT_ASSERT(pBeOSLeftRuler);
	pBeOSLeftRuler->createWidget(r);
	((AP_FrameData*)m_pBeOSFrame->getFrameData())->m_pLeftRuler = pBeOSLeftRuler;
	m_winRectAvailable.left = r.right +1;

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
        m_pbe_DocView->MakeFocus(true);
        return(m_pbe_DocView);                                    
}

UT_Bool AP_BeOSFrame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
}
