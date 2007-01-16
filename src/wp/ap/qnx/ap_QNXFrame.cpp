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

/*****************************************************************/

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

void AP_QNXFrame::setXScrollRange(void)
{
	AP_QNXFrameImpl * pQNXFrameImpl = static_cast<AP_QNXFrameImpl *>(getFrameImpl());
	GR_Graphics * pGr = pQNXFrameImpl->getFrame ()->getCurrentView ()->getGraphics ();

	int width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	int n, windowWidth;
	PtArg_t args[6];

	unsigned short tmp;
	UT_QNXGetWidgetArea(pQNXFrameImpl->m_dArea, NULL, NULL, &tmp, NULL);
	windowWidth = pGr->tlu(tmp);

	int newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	int newmax = width - windowWidth; /* upper - page_size */
	if (newmax <= 0)
	{
	newmax=0;
	newvalue=0;
	}
	if (newvalue > newmax)
		newvalue = newmax;

	n=0;
	PtSetArg(&args[n++], Pt_ARG_MAXIMUM, newmax, 0); 
	PtSetArg(&args[n++], Pt_ARG_INCREMENT, pGr->tlu(20), 0); 
	PtSetArg(&args[n++], Pt_ARG_PAGE_INCREMENT, windowWidth, 0); 
	PtSetArg(&args[n++], Pt_ARG_GAUGE_VALUE, newvalue, 0); 
	PtSetResources(pQNXFrameImpl->m_hScroll, n, args);

	bool bDifferentPosition = true;
	bool bDifferentLimits = true;

	if (m_pView && (bDifferentPosition || bDifferentLimits)) {
		m_pView->sendHorizontalScrollEvent(newvalue, (long) width-windowWidth);
	}
}

void AP_QNXFrame::setYScrollRange(void)
{
	AP_QNXFrameImpl	*pQNXFrameImpl = static_cast<AP_QNXFrameImpl *>(getFrameImpl());	
	GR_Graphics * pGr = pQNXFrameImpl->getFrame ()->getCurrentView ()->getGraphics ();

	int n, windowHeight;
	PtArg_t args[6];

	int height = ((AP_FrameData*)m_pData)->m_pDocLayout->getHeight();

	unsigned short tmp;
	UT_QNXGetWidgetArea(pQNXFrameImpl->m_dArea, NULL, NULL, NULL, &tmp);
	windowHeight = pGr->tlu(tmp);

	int newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	int newmax = height - windowHeight;	/* upper - page_size */
	if (newmax <= 0)
	{
		newmax=0;
		newvalue=0;
	}
	n =0;
	PtSetArg(&args[n++], Pt_ARG_MAXIMUM, newmax, 0); 
	PtSetArg(&args[n++], Pt_ARG_INCREMENT, pGr->tlu(20), 0); 
	PtSetArg(&args[n++], Pt_ARG_PAGE_INCREMENT, windowHeight, 0);
	PtSetArg(&args[n++], Pt_ARG_GAUGE_VALUE, newvalue, 0);
	PtSetResources(pQNXFrameImpl->m_vScroll, n, args);

	/*
	bool bDifferentPosition = (newvalue != (int)m_pVadj->value);
	bool bDifferentLimits ((height-windowHeight) != (m_pVadj->upper-m_pVadj->page_size));
	*/
	bool bDifferentPosition = 1;
	bool bDifferentLimits = 1;

	//printf("Set Y limits to %d -[%d]- %d \n", 0, newvalue, newmax);

	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendVerticalScrollEvent(newvalue, (long) height-windowHeight);
}


AP_QNXFrame::AP_QNXFrame(XAP_QNXApp * app)
	: AP_Frame(new AP_QNXFrameImpl(this,app),app)
{
	m_pData = NULL;
	setFrameLocked(false);
}

AP_QNXFrame::AP_QNXFrame(AP_QNXFrame * f)
	: AP_Frame(static_cast<AP_Frame*>(f))
{
	// TODO
	m_pData = NULL;
}

AP_QNXFrame::~AP_QNXFrame(void)
{
	killFrameData();
}

bool AP_QNXFrame::initialize(XAP_FrameMode frameMode)
{

	AP_QNXFrameImpl *pFrameImpl = static_cast<AP_QNXFrameImpl *>(getFrameImpl());


	setFrameMode(frameMode);
	setFrameLocked(false);

	if (!initFrameData())
		return false;

	if (!XAP_Frame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
								   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet,
								   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
								   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet))
		return false;
	UT_DEBUGMSG(("AP_QNXFrame: Creating toplevel window!\n"));
	pFrameImpl->_createWindow();
	return true;
}

	
XAP_Frame * AP_QNXFrame::cloneFrame(void)
{
	AP_QNXFrame * pClone = new AP_QNXFrame(this);
	ENSUREP(pClone);
	return static_cast<XAP_Frame *>(pClone);

Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		static_cast<XAP_App *>(m_pApp)->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
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
	AP_QNXFrameImpl	*pQNXFrameImpl = static_cast<AP_QNXFrameImpl *>(pQNXFrame->getFrameImpl());	

	//Do some range checking ...

	PtSetArg(&args[0], Pt_ARG_GAUGE_VALUE, xoff, 0);
	PtSetResources(pQNXFrameImpl->m_hScroll, 1, args);

	pView->setXScrollOffset(xoff);
}

void AP_QNXFrame::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 /*yrange*/)
{
	PtArg_t args[1];
	//printf("Static Y scroll function  \n");

	// this is a static callback function and doesn't have a 'this' pointer.
	AP_QNXFrame * pQNXFrame = (AP_QNXFrame *)(pData);
	AV_View * pView = pQNXFrame->getCurrentView();
	AP_QNXFrameImpl	*pQNXFrameImpl = static_cast<AP_QNXFrameImpl *>(pQNXFrame->getFrameImpl());		

	//Do some range checking ...

	PtSetArg(&args[0], Pt_ARG_GAUGE_VALUE, yoff, 0);
	PtSetResources(pQNXFrameImpl->m_vScroll, 1, args);

	pView->setYScrollOffset(yoff);
}
	

//This might be the place to do our co-ordinate conversions ...
void AP_QNXFrame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{
	printf("TODO: Translate Document To Screen %d,%d \n", x, y);
	UT_ASSERT_NOT_REACHED();
}


void AP_QNXFrame::setStatusMessage(const char * szMsg)
{
	((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}

void AP_QNXFrame::toggleBar(UT_uint32 iBarNb, bool bBarOn) {
	int		before, after;
	unsigned short *height;

    AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
    UT_ASSERT(pFrameData);

	PtGetResource(static_cast<XAP_QNXFrameImpl *>(getFrameImpl())->getTBGroupWidget(), Pt_ARG_HEIGHT, &height, 0);
	before = *height;

    if (bBarOn) {
        pFrameData->m_pToolbar[iBarNb]->show();
    }
    else {
        pFrameData->m_pToolbar[iBarNb]->hide();
    }

	PtExtentWidgetFamily(static_cast<XAP_QNXFrameImpl *>(getFrameImpl())->getTBGroupWidget());
	PtGetResource(static_cast<XAP_QNXFrameImpl *>(getFrameImpl())->getTBGroupWidget(), Pt_ARG_HEIGHT, &height, 0);
	after = *height;

	static_cast<AP_QNXFrameImpl *>(getFrameImpl())->_reflowLayout(0, before - after, 0, 0);
}

void AP_QNXFrame::toggleTopRuler(bool bRulerOn)
{
	unsigned short *height;
	AP_QNXTopRuler *pTopRuler = (AP_QNXTopRuler *)(((AP_FrameData *)m_pData)->m_pTopRuler);
	PtGetResource(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_topRuler, Pt_ARG_HEIGHT, &height, 0);
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();

	if (bRulerOn) {
			PtRealizeWidget(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_topRuler);
			static_cast<AP_QNXFrameImpl *>(getFrameImpl())->_reflowLayout(0, 0, -(*height), 0);
			FV_View * pView = static_cast<FV_View *>(m_pView);
			UT_uint32 iZoom = pView->getGraphics()->getZoomPercentage();
			static_cast<AP_TopRuler *>(pTopRuler)->setView(m_pView,iZoom);
			pView->setTopRuler(pTopRuler);
	} else {
			PtUnrealizeWidget(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_topRuler);
			PtSetResource(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_topRuler, Pt_ARG_FLAGS, Pt_DELAY_REALIZE, Pt_DELAY_REALIZE);
			static_cast<AP_QNXFrameImpl *>(getFrameImpl())->_reflowLayout(0, 0, *height, 0);

			static_cast<FV_View *>(m_pView)->setTopRuler(NULL);

	}
}

void AP_QNXFrame::toggleLeftRuler(bool bRulerOn)
{
	unsigned short *width;
	AP_QNXLeftRuler *pLeftRuler = (AP_QNXLeftRuler *)(((AP_FrameData *)m_pData)->m_pLeftRuler);

	PtGetResource(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_leftRuler, Pt_ARG_WIDTH, &width, 0);
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();

	if (bRulerOn) {
		    AP_LeftRuler * pLeft = pFrameData->m_pLeftRuler;
			PtRealizeWidget(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_leftRuler);
			static_cast<AP_QNXFrameImpl *>(getFrameImpl())->_reflowLayout(0, 0, 0, - (*width));
			FV_View * pView = static_cast<FV_View *>(m_pView);
			UT_uint32 iZoom = pView->getGraphics()->getZoomPercentage();
			static_cast<AP_LeftRuler *>(pLeftRuler)->setView(m_pView,iZoom);
			pView->setLeftRuler(pLeft);
			setYScrollRange();
	} else {
			PtUnrealizeWidget(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_leftRuler);
			PtSetResource(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_leftRuler, Pt_ARG_FLAGS, Pt_DELAY_REALIZE, Pt_DELAY_REALIZE);
			static_cast<AP_QNXFrameImpl *>(getFrameImpl())->_reflowLayout(0, 0, 0, *width);

			static_cast<FV_View *>(m_pView)->setLeftRuler(NULL);
			}
	}

void AP_QNXFrame::toggleRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);

	toggleLeftRuler(bRulerOn && (pFrameData->m_pViewMode == VIEW_PRINT));
	toggleTopRuler(bRulerOn);
}


void AP_QNXFrame::toggleStatusBar(bool bStatusBarOn) {
    AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
    UT_ASSERT(pFrameData);

    if (bStatusBarOn) {
        pFrameData->m_pStatusBar->show();
    }
    else {
        pFrameData->m_pStatusBar->hide();
    }

}

UT_sint32	AP_QNXFrame::_getDocumentAreaWidth()
{
	unsigned short *width;
	PtGetResource(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_dArea,Pt_ARG_WIDTH,&width,0);
	return *width;
}

UT_sint32	AP_QNXFrame::_getDocumentAreaHeight()
{
	unsigned short *height;
	PtGetResource(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_dArea,Pt_ARG_HEIGHT,&height,0);
	return *height;

}


bool AP_QNXFrame::_createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom)
{

	//pG = new GR_QNXGraphics(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->getTopLevelWindow(),static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_dArea,getApp());

	GR_QNXAllocInfo ai(static_cast<AP_QNXFrameImpl *>(getFrameImpl())->getTopLevelWindow(),
					   static_cast<AP_QNXFrameImpl *>(getFrameImpl())->m_dArea,getApp());
	pG = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);

	UT_ASSERT(pG);
	pG->setZoomPercentage(iZoom);
	return true;
}

void AP_QNXFrame::_setViewFocus(AV_View *pView)
{

}

void AP_QNXFrame::_bindToolbars(AV_View *pView)
{
	static_cast<AP_QNXFrameImpl *>(getFrameImpl())->_bindToolbars(pView);
}

bool AP_QNXFrame::_createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj, 
					     ap_ViewListener *& pViewListener, ap_Scrollbar_ViewListener *& pScrollbarViewListener,
					     AV_ListenerId &lid, AV_ListenerId &lidScrollbarViewListener)
{

	pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
	UT_ASSERT(pScrollObj);

	pViewListener = new ap_QNXViewListener(this);
	UT_ASSERT(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	UT_ASSERT(pScrollbarViewListener);
	
	if (!pView->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		return false;
	if (!pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		return false;

	return true;


}
