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
#include "xap_QNXFrame.h"
#include "ev_QNXToolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_QNXGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_QNXFrame.h"
#include "xap_QNXApp.h"
#include "ap_QNXTopRuler.h"
#include "ap_QNXLeftRuler.h"
#include "ap_QNXStatusBar.h"
#include "ap_QNXViewListener.h"
#include "ut_Xpm2Bitmap.h"

#include "ut_qnxHelper.h"

#ifdef ABISOURCE_LICENSED_TRADEMARKS
#include "abiword_48_tm.xpm"
#else
#include "abiword_48.xpm"
#endif


#if !defined(Pt_ARG_SCROLLBAR_POSITION)
#define Pt_ARG_SCROLLBAR_POSITION Pt_ARG_SCROLL_POSITION
#endif

/*****************************************************************/

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

void AP_QNXFrame::setZoomPercentage(UT_uint32 iZoom)
{
	_showDocument(iZoom);
}

UT_uint32 AP_QNXFrame::getZoomPercentage(void)
{
	return ((AP_FrameData*)m_pData)->m_pG->getZoomPercentage();
}

UT_Error AP_QNXFrame::_showDocument(UT_uint32 iZoom)
{
	UT_DEBUGMSG(("Frame: _showDocument \n"));

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

	GR_QNXGraphics * pG = NULL;
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

	pG = new GR_QNXGraphics(m_wTopLevelWindow, m_dArea, getApp());
	ENSUREP(pG);
	pG->setZoomPercentage(iZoom);
	
	pDocLayout = new FL_DocLayout((PD_Document *)(m_pDoc), pG);
	ENSUREP(pDocLayout);
  
	/*TF DIFF: The unix version has this commented out???*/
	pDocLayout->formatAll();

	pView = new FV_View(getApp(), this, pDocLayout);
	if (m_pView != NULL)
	{
		point = ((FV_View *) m_pView)->getPoint();
		pView->setFocus(m_pView->getFocus());		//Keep the same focus policy
	}
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

	if (!pView->addListener((AV_Listener *)pViewListener,&lid))
		goto Cleanup;

	if (!pView->addListener((AV_Listener *)pScrollbarViewListener,
							&lidScrollbarViewListener))
		goto Cleanup;

	nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	UT_uint32 k;
	for (k=0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.
		
		EV_QNXToolbar * pQNXToolbar = (EV_QNXToolbar *)m_vecToolbars.getNthItem(k);
		pQNXToolbar->bindListenerToView(pView);
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
	/*TF DIFF: Unix version checks the 
		  if ( ((AP_FrameData*)m_pData)->m_bShowRuler )
	  before showing the rulers.
	*/
	((AP_FrameData*)m_pData)->m_pTopRuler->setView(pView, iZoom);
	((AP_FrameData*)m_pData)->m_pLeftRuler->setView(pView, iZoom);
	((AP_FrameData*)m_pData)->m_pStatusBar->setView(pView);

	pView->setInsertMode(((AP_FrameData*)m_pData)->m_bInsertMode);
    ((FV_View *) m_pView)->setShowPara(((AP_FrameData*)m_pData)->m_bShowPara);
	
	unsigned short w, h;
	UT_QNXGetWidgetArea(m_dArea, NULL, NULL, &w, &h);
	UT_DEBUGMSG(("FRAME: Setting window to %d/%d ", w,h));
	m_pView->setWindowSize(w, h);

	setXScrollRange();
	setYScrollRange();
	updateTitle();

	PtContainerGiveFocus(m_dArea, NULL);

	if (point != 0) {
		((FV_View *) m_pView)->moveInsPtTo(point);
	}

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

	/*TF DIFF: Unix code to control the ruler looks like:
	  if ( ((AP_FrameData*)m_pData)->m_bShowRuler  ) {
	      if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
		...
	*/
	((AP_FrameData*)m_pData)->m_pTopRuler->draw(NULL);
	((AP_FrameData*)m_pData)->m_pLeftRuler->draw(NULL);
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

	UT_DEBUGMSG(("Frame: return from _showDocument false \n"));
	return UT_IE_ADDLISTENERERROR;
}

/*
 This function is called whenever we are re-sized to
 re-calculate the size/extent of the scroll bars.
 Once the size is calculated, it should send a new
 position event off.
*/
void AP_QNXFrame::setXScrollRange(void)
{
	int width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	int n, windowWidth;
	PtArg_t args[6];

	unsigned short tmp;
	UT_QNXGetWidgetArea(m_dArea, NULL, NULL, &tmp, NULL);
	windowWidth = tmp;

	int newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	int newmax = width - windowWidth; /* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	float slidersize;
	slidersize = (float)windowWidth / (float)newmax; 
	slidersize *= (float)windowWidth;

	n=0;
	PtSetArg(&args[n++], Pt_ARG_MAXIMUM, newmax, 0); 
	PtSetArg(&args[n++], Pt_ARG_INCREMENT, 20, 0); 
	PtSetArg(&args[n++], Pt_ARG_PAGE_INCREMENT, windowWidth, 0); 
	PtSetArg(&args[n++], Pt_ARG_SCROLLBAR_POSITION, newvalue, 0); 
	/* PtSetArg(&args[n++], Pt_ARG_SLIDER_SIZE, (int)slidersize, 0); */
	PtSetResources(m_hScroll, n, args);
	UT_DEBUGMSG(("X SLIDER SIZE CHANGE TO %f (max %d) ", slidersize, newmax));

	/*
	UT_Bool bDifferentPosition = (newvalue != (int)m_pHadj->value);
	UT_Bool bDifferentLimits = ((width-windowWidth) != (m_pHadj->upper-m_pHadj->page_size));
	*/
	UT_Bool bDifferentPosition = 1;
	UT_Bool bDifferentLimits = 1;

	//printf("Set X limits to %d -[%d]- %d \n", 0, newvalue, newmax);
	
	if (m_pView && (bDifferentPosition || bDifferentLimits)) {
		m_pView->sendHorizontalScrollEvent(newvalue, (long) width-windowWidth);
	}
}

void AP_QNXFrame::setYScrollRange(void)
{
	int height = ((AP_FrameData*)m_pData)->m_pDocLayout->getHeight();
	int n, windowHeight;
	PtArg_t args[6];

	unsigned short tmp;
	UT_QNXGetWidgetArea(m_dArea, NULL, NULL, NULL, &tmp);
	windowHeight = tmp;

	int newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	int newmax = height - windowHeight;	/* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	float slidersize;
	slidersize = (float)windowHeight / (float)newmax; 
	slidersize *= (float)windowHeight;

	n =0;
	PtSetArg(&args[n++], Pt_ARG_MAXIMUM, newmax, 0); 
	PtSetArg(&args[n++], Pt_ARG_INCREMENT, 20, 0); 
	PtSetArg(&args[n++], Pt_ARG_PAGE_INCREMENT, windowHeight, 0);
	PtSetArg(&args[n++], Pt_ARG_SCROLLBAR_POSITION, newvalue, 0);
	/* PtSetArg(&args[n++], Pt_ARG_SLIDER_SIZE, slidersize, 0);  */
	PtSetResources(m_vScroll, n, args);
	UT_DEBUGMSG(("Y SLIDER SIZE CHANGE TO %f (max %d) ", slidersize, newmax));

	/*
	UT_Bool bDifferentPosition = (newvalue != (int)m_pVadj->value);
	UT_Bool bDifferentLimits ((height-windowHeight) != (m_pVadj->upper-m_pVadj->page_size));
	*/
	UT_Bool bDifferentPosition = 1;
	UT_Bool bDifferentLimits = 1;

	//printf("Set Y limits to %d -[%d]- %d \n", 0, newvalue, newmax);

	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendVerticalScrollEvent(newvalue, (long) height-windowHeight);
}


AP_QNXFrame::AP_QNXFrame(XAP_QNXApp * app)
	: XAP_QNXFrame(app)
{
	// TODO
	m_pData = NULL;
}

AP_QNXFrame::AP_QNXFrame(AP_QNXFrame * f)
	: XAP_QNXFrame((XAP_QNXFrame *)(f))
{
	// TODO
	m_pData = NULL;
}

AP_QNXFrame::~AP_QNXFrame(void)
{
	killFrameData();
}

UT_Bool AP_QNXFrame::initialize(void)
{
	if (!initFrameData())
		return UT_FALSE;

	if (!XAP_QNXFrame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
								   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
								   AP_PREF_KEY_MenuLabelSet, AP_PREF_DEFAULT_MenuLabelSet,
								   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
								   AP_PREF_KEY_ToolbarLabelSet, AP_PREF_DEFAULT_ToolbarLabelSet))
		return UT_FALSE;

	_createTopLevelWindow();
	_showOrHideToolbars();
	_showOrHideStatusbar();
	PtRealizeWidget(m_wTopLevelWindow);
	PtDamageWidget(m_wTopLevelWindow);

	return UT_TRUE;
}

// Does the initial show/hide of toolbars (based on the user prefs).
// This is needed because toggleBar is called only when the user
// (un)checks the show {Stantandard,Format,Extra} toolbar checkbox,
// and thus we have to manually call this function at startup.
void AP_QNXFrame::_showOrHideToolbars(void)
{
    UT_Bool *bShowBar = static_cast<AP_FrameData*> (m_pData)->m_bShowBar;

    for (UT_uint32 i = 0; i < m_vecToolbarLayoutNames.getItemCount(); i++)
    {
        // TODO: The two next lines are here to bind the EV_Toolbar to the
        // AP_FrameData, but their correct place are next to the toolbar creation (JCA)
        EV_QNXToolbar * pQNXToolbar = static_cast<EV_QNXToolbar *> (m_vecToolbars.getNthItem(i));
        static_cast<AP_FrameData*> (m_pData)->m_pToolbar[i] = pQNXToolbar;
        toggleBar(i, bShowBar[i]);
    }
}

// Does the initial show/hide of toolbars (based on the user prefs).
void AP_QNXFrame::_showOrHideStatusbar(void)
{
    UT_Bool bShowStatusBar = static_cast<AP_FrameData*> (m_pData)->m_bShowStatusBar;
    //  toggleStatusBar(bShowStatusBar);
}

/*****************************************************************/

UT_Bool AP_QNXFrame::initFrameData(void)
{
	UT_ASSERT(!((AP_FrameData*)m_pData));

	AP_FrameData* pData = new AP_FrameData(m_pQNXApp);

	m_pData = (void*)pData;
	return (pData ? UT_TRUE : UT_FALSE);
}

void AP_QNXFrame::killFrameData(void)
{
	AP_FrameData* pData = (AP_FrameData*) m_pData;
	DELETEP(pData);
	m_pData = NULL;
}

UT_Error AP_QNXFrame::_loadDocument(const char * szFilename, IEFileType ieft)
{
	UT_DEBUGMSG(("Frame: _loadDocument \n"));
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

	UT_Error err; 
	err = pNewDoc->readFromFile(szFilename, ieft);
	if (err == UT_OK)
		goto ReplaceDocument;
	
	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	UNREFP(pNewDoc);
	return err;

ReplaceDocument:
	getApp()->forgetClones(this);

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}
	
XAP_Frame * AP_QNXFrame::cloneFrame(void)
{
	AP_QNXFrame * pClone = new AP_QNXFrame(this);
	UT_Error error = UT_OK;
	ENSUREP(pClone);

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
		m_pQNXApp->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}

UT_Error AP_QNXFrame::loadDocument(const char * szFilename, int ieft)
{
	UT_Bool bUpdateClones;
	UT_Vector vClones;
	XAP_App * pApp = getApp();

	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}

	UT_Error err;
	err = _loadDocument(szFilename, (IEFileType) ieft); 
	if (err != UT_OK)
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
			AP_QNXFrame * pFrame = (AP_QNXFrame *) vClones.getNthItem(i);
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
 These functions are called whenever the position of the scrollbar
 might have changed.  Either from someone typeing in the window or
 because the window resized, or because the user grabbed the scrool
 bar and moved it.
*/
void AP_QNXFrame::_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 /*xrange*/)
{
	PtArg_t args[1];
	//printf("Static X scroll function  \n");
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_QNXFrame * pQNXFrame = (AP_QNXFrame *)(pData);
	AV_View * pView = pQNXFrame->getCurrentView();
	
	//Do some range checking ...

	PtSetArg(&args[0], Pt_ARG_SCROLLBAR_POSITION, xoff, 0);
	PtSetResources(pQNXFrame->m_hScroll, 1, args);

	pView->setXScrollOffset(xoff);
}

void AP_QNXFrame::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 /*yrange*/)
{
	PtArg_t args[1];
	//printf("Static Y scroll function  \n");

	// this is a static callback function and doesn't have a 'this' pointer.
	AP_QNXFrame * pQNXFrame = (AP_QNXFrame *)(pData);
	AV_View * pView = pQNXFrame->getCurrentView();
	
	//Do some range checking ...

	PtSetArg(&args[0], Pt_ARG_SCROLLBAR_POSITION, yoff, 0);
	PtSetResources(pQNXFrame->m_vScroll, 1, args);

	pView->setYScrollOffset(yoff);
}
	
#if 0
static int _resize_mda(PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	PtWidget_t *raw = (PtWidget_t *)data;
	PtContainerCallback_t *cbinfo = (PtContainerCallback_t *)(info->cbdata);

	UT_DEBUGMSG(("SUCKY RESIZING to %d,%d %d,%d \n",
		cbinfo->new_size.ul.x, cbinfo->new_size.ul.y,
		cbinfo->new_size.lr.x, cbinfo->new_size.lr.y));
	PtArg_t args[2];
	PtSetArg(&args[0], Pt_ARG_WIDTH, cbinfo->new_size.lr.x - cbinfo->new_size.ul.x, 0);
	PtSetArg(&args[1], Pt_ARG_HEIGHT, cbinfo->new_size.lr.y - cbinfo->new_size.ul.y, 0);
	PtSetResources(raw, 2, args);
	return Pt_CONTINUE;
}
#endif

PtWidget_t * AP_QNXFrame::_createDocumentWindow(void)
{
	PtWidget_t *group;
	PhArea_t area, savedarea;
	void * data = this;

	PtArg_t args[10];
	int n;

	/*TF DIFF: There is code here to not show
               the rulers, checked by
		UT_Bool bShowRulers = ((AP_FrameData*)m_pData)->m_bShowRuler;
	*/


#define SCROLLBAR_WIDTHHEIGHT 20
	// Strip the scrollbarwidth off the right and bottom
	// so that the scrollbars overlap the rulers
	savedarea = m_AvailableArea;
#if !defined(SCROLL_SMALLER_THAN_RULER) 
	m_AvailableArea.size.h -= SCROLLBAR_WIDTHHEIGHT; 
	m_AvailableArea.size.w -= SCROLLBAR_WIDTHHEIGHT; 
#endif

	// create the top ruler
	AP_QNXTopRuler * pQNXTopRuler = new AP_QNXTopRuler(this);
	UT_ASSERT(pQNXTopRuler);
	m_topRuler = pQNXTopRuler->createWidget();
	((AP_FrameData*)m_pData)->m_pTopRuler = pQNXTopRuler;

	// create the left ruler
	AP_QNXLeftRuler * pQNXLeftRuler = new AP_QNXLeftRuler(this);
	UT_ASSERT(pQNXLeftRuler);
	m_leftRuler = pQNXLeftRuler->createWidget();
	((AP_FrameData*)m_pData)->m_pLeftRuler = pQNXLeftRuler;

	// get the width from the left ruler and stuff it into the top ruler.
	pQNXTopRuler->setOffsetLeftRuler(pQNXLeftRuler->getWidth());

	// create the scrollbars horizontal then vertical

	n = 0;
#if defined(SCROLL_SMALLER_THAN_RULER) 
	area.size.w = SCROLLBAR_WIDTHHEIGHT;
	area.size.h = m_AvailableArea.size.h - area.size.w;
	area.pos.y = m_AvailableArea.pos.y;
	area.pos.x = m_AvailableArea.pos.x + m_AvailableArea.size.w - area.size.w;
	m_AvailableArea.size.w -= area.size.w;
#else
	area.size.w = SCROLLBAR_WIDTHHEIGHT;
	area.size.h = savedarea.size.h - area.size.w;
	area.pos.y = savedarea.pos.y;
	area.pos.x = savedarea.pos.x + savedarea.size.w - area.size.w;
#endif
	PtSetArg(&args[n], Pt_ARG_AREA, &area, 0); n++;
#define _VS_ANCHOR_ (Pt_LEFT_ANCHORED_RIGHT | Pt_RIGHT_ANCHORED_RIGHT | \
		     Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_BOTTOM)
	PtSetArg(&args[n], Pt_ARG_ANCHOR_FLAGS, _VS_ANCHOR_, _VS_ANCHOR_); n++;
#define _VS_STRETCH_ (Pt_GROUP_STRETCH_HORIZONTAL | Pt_GROUP_STRETCH_VERTICAL)
	PtSetArg(&args[n], Pt_ARG_GROUP_FLAGS, _VS_STRETCH_, _VS_STRETCH_); n++;
	group = PtCreateWidget(PtGroup, getTopLevelWindow(), n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_SCROLLBAR_FLAGS, Pt_SCROLLBAR_FOCUSED | 0 /*Vertical*/, 
									 		     Pt_SCROLLBAR_FOCUSED | 0 /*Vertical*/); 
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS);
	PtSetArg(&args[n++], Pt_ARG_ORIENTATION, 0 /*Vertical*/, 0); 
	m_vScroll = PtCreateWidget(PtScrollbar, group, n, args);
	PtAddCallback(m_vScroll, Pt_CB_SCROLL_MOVE, _fe::vScrollChanged, this);

	n = 0;
#if defined(SCROLL_SMALLER_THAN_RULER) 
	area.size.h = SCROLLBAR_WIDTHHEIGHT;
	area.size.w = m_AvailableArea.size.w;
	area.pos.y = m_AvailableArea.pos.y + m_AvailableArea.size.h - area.size.h;
	area.pos.x = m_AvailableArea.pos.x;
	m_AvailableArea.size.h -= area.size.h;
#else
	area.size.h = SCROLLBAR_WIDTHHEIGHT;
	area.size.w = savedarea.size.w - SCROLLBAR_WIDTHHEIGHT;
	area.pos.y = savedarea.pos.y + savedarea.size.h - area.size.h;
	area.pos.x = savedarea.pos.x;
#endif
	PtSetArg(&args[n], Pt_ARG_AREA, &area, 0); n++;
#define _HS_ANCHOR_ (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | \
		     Pt_TOP_ANCHORED_BOTTOM | Pt_BOTTOM_ANCHORED_BOTTOM)
	PtSetArg(&args[n], Pt_ARG_ANCHOR_FLAGS, _HS_ANCHOR_, _HS_ANCHOR_); n++;
#define _HS_STRETCH_ (Pt_GROUP_STRETCH_HORIZONTAL | Pt_GROUP_STRETCH_VERTICAL)
	PtSetArg(&args[n], Pt_ARG_GROUP_FLAGS, _HS_STRETCH_, _HS_STRETCH_); n++;
	group = PtCreateWidget(PtGroup, getTopLevelWindow(), n, args);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_SCROLLBAR_FLAGS, Pt_SCROLLBAR_FOCUSED | 1 /*Horizontal*/,
									 			 Pt_SCROLLBAR_FOCUSED | 1 /*Horizontal*/); 
	PtSetArg(&args[n++], Pt_ARG_FLAGS, 0, Pt_GETS_FOCUS); 
	PtSetArg(&args[n++], Pt_ARG_ORIENTATION, 1 /*Horizontal*/, 0); 
	m_hScroll = PtCreateWidget(PtScrollbar, group, n, args);
	PtAddCallback(m_hScroll, Pt_CB_SCROLL_MOVE, _fe::hScrollChanged, this);

	// create a drawing area in the for our document window.

	area.pos.x = m_AvailableArea.pos.x;
	area.pos.y = m_AvailableArea.pos.y;
	area.size.w = m_AvailableArea.size.w; 
	area.size.h = m_AvailableArea.size.h;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0); 
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, Pt_GROUP_VERTICAL);
	PtSetArg(&args[n++], Pt_ARG_FILL_COLOR, Pg_TRANSPARENT, 0); 
#define _DA_ANCHOR_ (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT | \
		     Pt_TOP_ANCHORED_TOP | Pt_BOTTOM_ANCHORED_BOTTOM)
	PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS, _DA_ANCHOR_, _DA_ANCHOR_);
#define _DA_STRETCH_ (Pt_GROUP_STRETCH_VERTICAL | Pt_GROUP_STRETCH_HORIZONTAL)
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, _DA_STRETCH_, _DA_STRETCH_);
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	group = PtCreateWidget(PtGroup, getTopLevelWindow(), n, args);
	if (!group) {
		printf("Can't get the MDA group \n");
	}
	PtAddCallback(group, Pt_CB_RESIZE, &(_fe::resize), this);
	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_DIM, &area.size, 0); 
	//If we set to transparent, then we don't properly re-draw areas
	PtSetArg(&args[n++], Pt_ARG_FILL_COLOR, Pg_TRANSPARENT, 0); 
	PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
	PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &(_fe::expose), 1); 
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_GETS_FOCUS, Pt_GETS_FOCUS); 
	m_dArea = PtCreateWidget(PtRaw, group, n, args); 
	if (!m_dArea) {
		printf("ERROR: Can't create the document area \n");
	}
	PtAddEventHandler(m_dArea, Ph_EV_KEY, _fe::key_press_event, this);
	PtAddEventHandler(m_dArea, Ph_EV_PTR_MOTION_BUTTON, _fe::motion_notify_event, this);
	PtAddEventHandler(m_dArea, Ph_EV_BUT_PRESS, _fe::button_press_event, this);
	PtAddEventHandler(m_dArea, Ph_EV_BUT_RELEASE, _fe::button_release_event, this);

	return(group);
}

//This might be the place to do our co-ordinate conversions ...
void AP_QNXFrame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{
	printf("TODO: Translate Document To Screen %d,%d \n", x, y);
}

PtWidget_t * AP_QNXFrame::_createStatusBarWindow(void)
{
	AP_QNXStatusBar * pQNXStatusBar = new AP_QNXStatusBar(this);
	UT_ASSERT(pQNXStatusBar);

	((AP_FrameData *)m_pData)->m_pStatusBar = pQNXStatusBar;
	
	PtWidget_t * w = pQNXStatusBar->createWidget();

	return w;
}

void AP_QNXFrame::setStatusMessage(const char * szMsg)
{
	((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}

void AP_QNXFrame::_setWindowIcon(void)
{
#if 0
	UT_DEBUGMSG(("TODO: Fix the setting of the ICON "));

	// attach program icon to window
	PtWidget_t * window = getTopLevelWindow();
	UT_ASSERT(window);

	// create a pixmap from our included data
	PhImage_t *pImage;
	if (!(UT_Xpm2Bitmap((const char **)abiword_48_xpm, 0xdeadbeef, &pImage))) {
		return;
	}

	PtArg_t args[5];
	int		n;

	n = 0;
	PtWidget_t *icon = PtCreateWidget(PtIcon, window, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_LABEL_TYPE, Pt_IMAGE, Pt_IMAGE);
	PtSetArg(&args[n++], Pt_ARG_LABEL_DATA, pImage, sizeof(*pImage));
	PtSetArg(&args[n++], Pt_ARG_DIM, &pImage->size, 0);
	PtCreateWidget(PtLabel, icon, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_ICON_WINDOW, icon, sizeof(*icon));
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, Ph_WM_RENDER_ASICON, Ph_WM_RENDER_ASICON);
	PtSetResources(window, n, args);
#endif
}

UT_Error AP_QNXFrame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
}

void AP_QNXFrame::toggleBar(UT_uint32 iBarNb, UT_Bool bBarOn) {
    AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
    UT_ASSERT(pFrameData);

    if (bBarOn) {
        pFrameData->m_pToolbar[iBarNb]->show();
    }
    else {
        pFrameData->m_pToolbar[iBarNb]->hide();
    }
}

void AP_QNXFrame::toggleRuler(UT_Bool bRulerOn) {
	UT_DEBUGMSG(("TODO: Toggle ruler code "));
}

void AP_QNXFrame::toggleStatusBar(UT_Bool bStatusBarOn) {
    AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
    UT_ASSERT(pFrameData);

    if (bStatusBarOn) {
        pFrameData->m_pStatusBar->show();
    }
    else {
        pFrameData->m_pStatusBar->hide();
    }
}

void AP_QNXFrame::setDocumentFocus() {
	PtContainerGiveFocus(m_dArea, NULL);
}
