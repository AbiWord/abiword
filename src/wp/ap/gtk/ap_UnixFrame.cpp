/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "ap_FrameData.h"
#include "ap_Frame.h"
#include "ev_UnixToolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_UnixCairoGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_UnixFrame.h"
#include "ap_UnixFrameImpl.h"
#include "xap_UnixApp.h"
#include "ap_UnixTopRuler.h"
#include "ap_UnixLeftRuler.h"
#include "ap_UnixStatusBar.h"
#include "ap_UnixViewListener.h"
#include "xap_UnixDialogHelper.h"
#if 1
#include "ev_UnixMenuBar.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "ev_Menu_Actions.h"
#endif


/*****************************************************************/

#define ENSUREP_RF(p)            do { UT_ASSERT(p); if (!p) return false; } while (0)
#define ENSUREP_C(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/
#include "ap_UnixApp.h"


void AP_UnixFrame::setXScrollRange(void)
{
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
	UT_return_if_fail(pFrameImpl);
	GR_Graphics * pGr = pFrameImpl->getFrame ()->getCurrentView ()->getGraphics ();

	int width = 0;
	if(m_pData) //this isn't guaranteed in AbiCommand
		width = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getWidth();

	int windowWidth = 0;
	GtkAllocation allocation;
	gtk_widget_get_allocation(GTK_WIDGET(pFrameImpl->m_dArea),&allocation);
	if(pFrameImpl->m_dArea) //this isn't guaranteed in AbiCommand
		windowWidth = static_cast<int>(pGr->tluD (allocation.width));

	int newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	int newmax = width - windowWidth; /* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	bool bDifferentPosition = false;
	bool bDifferentLimits = false;
	if(pFrameImpl->m_pHadj) //this isn't guaranteed in AbiCommand
	{
		bDifferentPosition = (newvalue != gtk_adjustment_get_value(pFrameImpl->m_pHadj));
		bDifferentLimits = ((width-windowWidth) != gtk_adjustment_get_upper(pFrameImpl->m_pHadj)-
						                        gtk_adjustment_get_page_size(pFrameImpl->m_pHadj));
	}


	if (m_pView && (bDifferentPosition || bDifferentLimits))
	{
		pFrameImpl->_setScrollRange(apufi_scrollX, newvalue, static_cast<gfloat>(width), static_cast<gfloat>(windowWidth));
		m_pView->sendHorizontalScrollEvent(newvalue, 
										   static_cast<UT_sint32>
												(gtk_adjustment_get_upper(pFrameImpl->m_pHadj)-
												 gtk_adjustment_get_page_size(pFrameImpl->m_pHadj)));
	}
}

void AP_UnixFrame::setYScrollRange(void)
{
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
	UT_return_if_fail(pFrameImpl);
	GR_Graphics * pGr = pFrameImpl->getFrame ()->getCurrentView ()->getGraphics ();

	int height = 0;
	if(m_pData) //this isn't guaranteed in AbiCommand
		height = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getHeight();

	int windowHeight = 0;
	GtkAllocation allocation;
	gtk_widget_get_allocation(GTK_WIDGET(pFrameImpl->m_dArea),&allocation);
	if(pFrameImpl->m_dArea) //this isn't guaranteed in AbiCommand
		windowHeight = static_cast<int>(pGr->tluD (allocation.height));

	int newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	int newmax = height - windowHeight;	/* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	bool bDifferentPosition = false;
	UT_sint32 diff = 0;
	if(pFrameImpl->m_pVadj) //this isn't guaranteed in AbiCommand
	{
		bDifferentPosition = (newvalue != static_cast<UT_sint32>(gtk_adjustment_get_value(pFrameImpl->m_pVadj) +0.5));
		diff = static_cast<UT_sint32>(gtk_adjustment_get_upper(pFrameImpl->m_pVadj)-
		                              gtk_adjustment_get_page_size(pFrameImpl->m_pVadj) +0.5);
	}


	if(bDifferentPosition)
	{
		UT_sint32 iDU = pGr->tdu( static_cast<UT_sint32>(gtk_adjustment_get_value(pFrameImpl->m_pVadj) +0.5) - newvalue);
		if(iDU == 0)
		{
			bDifferentPosition = false;
			gtk_adjustment_set_value(pFrameImpl->m_pVadj, static_cast<gdouble>(newvalue));
		}
	}
	bool bDifferentLimits = ((height-windowHeight) != diff);
	
	if (m_pView && (bDifferentPosition || bDifferentLimits))
	{
		pFrameImpl->_setScrollRange(apufi_scrollY, newvalue, static_cast<gfloat>(height), static_cast<gfloat>(windowHeight));
		m_pView->sendVerticalScrollEvent(newvalue, 
										 static_cast<UT_sint32>
											   (gtk_adjustment_get_upper(pFrameImpl->m_pVadj) -
												gtk_adjustment_get_page_size(pFrameImpl->m_pVadj)));
	}
}


AP_UnixFrame::AP_UnixFrame()
: AP_Frame(new AP_UnixFrameImpl(this))
{
	m_pData = NULL;
	setFrameLocked(false);
#ifdef LOGFILE
	fprintf(getlogfile(),"New unix frame with app \n");
	fprintf(getlogfile(),"Number of frames in app %d \n",pApp->getFrameCount());
#endif
}

AP_UnixFrame::AP_UnixFrame(AP_UnixFrame * f)
	: AP_Frame(static_cast<AP_Frame *>(f))
{
	m_pData = NULL;
#ifdef LOGFILE
	fprintf(getlogfile(),"New unix frame with frame \n");
#endif
}

AP_UnixFrame::~AP_UnixFrame()
{
	killFrameData();
}

XAP_Frame * AP_UnixFrame::cloneFrame()
{
	AP_Frame * pClone = new AP_UnixFrame(this);
	ENSUREP_C(pClone);
	return static_cast<XAP_Frame *>(pClone);

 Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		XAP_App::getApp()->forgetFrame(pClone);
		delete pClone;
	}
	return NULL;
}

bool AP_UnixFrame::initialize(XAP_FrameMode frameMode)
{
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
#if DEBUG
	if(frameMode == XAP_NormalFrame)
	{
		UT_DEBUGMSG(("AP_UnixFrame::initialize!!!! NormalFrame this %p \n",this));
	}
	else if(frameMode == XAP_NoMenusWindowLess)
	{
		UT_DEBUGMSG(("AP_UnixFrame::initialize!!!! NoMenus No Window \n"));
	}
	else if(frameMode == XAP_WindowLess)
	{
		UT_DEBUGMSG(("AP_UnixFrame::initialize!!!! No Window with menus \n"));
	}
#endif
	setFrameMode(frameMode);
	setFrameLocked(false);

	if (!initFrameData())
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	UT_DEBUGMSG(("AP_UnixFrame:: Initializing base classes!!!! \n"));

	if (!XAP_Frame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
				   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
				   AP_PREF_KEY_StringSet, AP_PREF_KEY_StringSet,
				   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
				   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet))
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	UT_DEBUGMSG(("AP_UnixFrame:: Creating Toplevel Window!!!! \n"));	
	pFrameImpl->_createWindow();

	return true;
}



/*****************************************************************/

// WL_REFACTOR: Put this in the helper
void AP_UnixFrame::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 /*yrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_UnixFrame * pUnixFrame = static_cast<AP_UnixFrame *>(pData);
	AV_View * pView = pUnixFrame->getCurrentView();
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(pUnixFrame->getFrameImpl());

	// we've been notified (via sendVerticalScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.
	
	gfloat yoffNew = yoff;
	gfloat yoffMax = gtk_adjustment_get_upper(pFrameImpl->m_pVadj) - 
		gtk_adjustment_get_page_size(pFrameImpl->m_pVadj);
	if (yoffMax <= 0)
		yoffNew = 0;
	else if (yoffNew > yoffMax)
		yoffNew = yoffMax;

	// we want to twiddle xoffNew such that actual scroll = anticipated scroll.
	// actual scroll is given by the formula xoffNew-pView->getXScrollOffset()

	// this is exactly the same computation that we do in the scrolling code.
	// I don't think we need as much precision anymore, now that we have
	// precise rounding, but it can't really be a bad thing.
	GR_Graphics * pG = static_cast<FV_View*>(pView)->getGraphics();

	UT_sint32 dy = static_cast<UT_sint32>
		(pG->tluD(static_cast<UT_sint32>(pG->tduD
			   (static_cast<UT_sint32>(pView->getYScrollOffset()-yoffNew)))));
	gfloat yoffDisc = static_cast<UT_sint32>(pView->getYScrollOffset()) - dy;

	// We need to block the signal this will send. The setYScrollOffset method
	// will do the scroll for us. Otherwise we'll scroll back here later!!
	
	g_signal_handler_block((gpointer)pFrameImpl->m_pVadj, pFrameImpl->m_iVScrollSignal);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(pFrameImpl->m_pVadj),yoffNew);
	g_signal_handler_unblock((gpointer)pFrameImpl->m_pVadj, pFrameImpl->m_iVScrollSignal);


	if (pG->tdu(static_cast<UT_sint32>(yoffDisc) - 
				pView->getYScrollOffset()) != 0)
	pView->setYScrollOffset(static_cast<UT_sint32>(yoffDisc));
}

// WL_REFACTOR: Put this in the helper
void AP_UnixFrame::_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 /*xrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_UnixFrame * pUnixFrame = static_cast<AP_UnixFrame *>(pData);
	AV_View * pView = pUnixFrame->getCurrentView();
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(pUnixFrame->getFrameImpl());

	// we've been notified (via sendScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.

	gfloat xoffNew = xoff;
	gfloat xoffMax = gtk_adjustment_get_upper(pFrameImpl->m_pHadj) - 
		gtk_adjustment_get_page_size(pFrameImpl->m_pHadj);
	if (xoffMax <= 0)
		xoffNew = 0;
	else if (xoffNew > xoffMax)
		xoffNew = xoffMax;

	// we want to twiddle xoffNew such that actual scroll = anticipated scroll.
	// actual scroll is given by the formula xoffNew-pView->getXScrollOffset()

	// this is exactly the same computation that we do in the scrolling code.
	// I don't think we need as much precision anymore, now that we have
	// precise rounding, but it can't really be a bad thing.
	GR_Graphics * pG = static_cast<FV_View*>(pView)->getGraphics();

	UT_sint32 dx = static_cast<UT_sint32>
		(pG->tluD(static_cast<UT_sint32>(pG->tduD
			   (static_cast<UT_sint32>(pView->getXScrollOffset()-xoffNew)))));
	gfloat xoffDisc = static_cast<UT_sint32>(pView->getXScrollOffset()) - dx;


	// We need to block the signal this will send. The setHScrollOffset method
	// will do the scroll for us. Otherwise we'll scroll back here later!!
	
	g_signal_handler_block((gpointer)pFrameImpl->m_pHadj, pFrameImpl->m_iHScrollSignal);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(pFrameImpl->m_pHadj),xoffDisc);
	g_signal_handler_unblock((gpointer)pFrameImpl->m_pHadj, pFrameImpl->m_iHScrollSignal);

	// (this is the calculation for dx again, post rounding)
	// This may not actually be helpful, because we could still lose if the
	// second round of rounding gives us the wrong number.  Leave it for now.
	if (pG->tdu(static_cast<UT_sint32>(xoffDisc) - 
				pView->getXScrollOffset()) != 0)
		pView->setXScrollOffset(static_cast<UT_sint32>(xoffDisc));
}

void AP_UnixFrame::translateDocumentToScreen(UT_sint32 & /*x*/, UT_sint32 & /*y*/)
{
	UT_ASSERT_NOT_REACHED();
}

void AP_UnixFrame::setStatusMessage(const char * szMsg)
{
#ifdef ENABLE_STATUSBAR
	if((getFrameMode() == XAP_NormalFrame) && (m_pData))
	{
		static_cast<AP_FrameData *>(m_pData)->m_pStatusBar->setStatusMessage(szMsg);
	}
#endif
}

void AP_UnixFrame::toggleTopRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(getFrameData());
	UT_ASSERT(pFrameData);
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
		
	AP_UnixTopRuler * pUnixTopRuler = NULL;

	UT_DEBUGMSG(("AP_UnixFrame::toggleTopRuler %d, %p\n", 
		     bRulerOn, pFrameData->m_pTopRuler));

	if ( bRulerOn )
	{
		if(pFrameData->m_pTopRuler)
		{
			if(pFrameImpl->m_topRuler && GTK_IS_WIDGET(pFrameImpl->m_topRuler))
			{
				gtk_widget_destroy( GTK_WIDGET(pFrameImpl->m_topRuler) );
			}			
			DELETEP(pFrameData->m_pTopRuler);
		}
		FV_View * pView = static_cast<FV_View *>(m_pView);
		UT_uint32 iZoom = pView->getGraphics()->getZoomPercentage();
		pUnixTopRuler = new AP_UnixTopRuler(this);
		UT_ASSERT(pUnixTopRuler);
		pFrameData->m_pTopRuler = pUnixTopRuler;		
		pFrameImpl->m_topRuler = pUnixTopRuler->createWidget();

		// attach everything
		gtk_grid_attach(GTK_GRID(pFrameImpl->m_innergrid), 
				 pFrameImpl->m_topRuler, 0, 0, 2, 1);

		static_cast<AP_TopRuler *>(pUnixTopRuler)->setView(m_pView,iZoom);

		// get the width from the left ruler and stuff it into the 
		// top ruler.
		if (static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler)
			pUnixTopRuler->setOffsetLeftRuler(static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->getWidth());
		else
			pUnixTopRuler->setOffsetLeftRuler(0);
	}
	else
	{
		// delete the actual widgets
		if(pFrameImpl->m_topRuler && GTK_IS_WIDGET(pFrameImpl->m_topRuler))
		{
			gtk_widget_destroy( GTK_WIDGET(pFrameImpl->m_topRuler) );
		}
		DELETEP(pFrameData->m_pTopRuler);
		pFrameImpl->m_topRuler = NULL;
		static_cast<FV_View *>(m_pView)->setTopRuler(NULL);
	}
}

void AP_UnixFrame::toggleLeftRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(getFrameData());
	UT_ASSERT(pFrameData);
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());

	UT_DEBUGMSG(("AP_UnixFrame::toggleLeftRuler %d, %p\n", 
		     bRulerOn, pFrameData->m_pLeftRuler));

	if (bRulerOn)
	{
//
// if there is an old ruler then delete that first.
//
		if(pFrameData->m_pLeftRuler)
		{
			if (pFrameImpl->m_leftRuler && GTK_IS_WIDGET(pFrameImpl->m_leftRuler))
			{
				gtk_widget_destroy(GTK_WIDGET(pFrameImpl->m_leftRuler) );
			}		
			DELETEP(pFrameData->m_pLeftRuler);
		} 
		FV_View * pView = static_cast<FV_View *>(m_pView);		
		UT_uint32 iZoom = pView->getGraphics()->getZoomPercentage();
		AP_UnixLeftRuler * pUnixLeftRuler = new AP_UnixLeftRuler(this);
		UT_ASSERT(pUnixLeftRuler);
		pFrameData->m_pLeftRuler = pUnixLeftRuler;		
		pFrameImpl->m_leftRuler = pUnixLeftRuler->createWidget();

		gtk_grid_attach(GTK_GRID(pFrameImpl->m_innergrid), 
				 pFrameImpl->m_leftRuler, 0, 1, 1, 1);
		static_cast<AP_LeftRuler *>(pUnixLeftRuler)->setView(m_pView,iZoom);
		setYScrollRange();
	}
	else
	{
	    if (pFrameImpl->m_leftRuler && GTK_IS_WIDGET(pFrameImpl->m_leftRuler))
		{
			gtk_widget_destroy(GTK_WIDGET(pFrameImpl->m_leftRuler) );
		}
	    DELETEP(pFrameData->m_pLeftRuler);
	    pFrameImpl->m_leftRuler = NULL;
		static_cast<FV_View *>(m_pView)->setLeftRuler(NULL);
	}

}

void AP_UnixFrame::toggleRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(getFrameData());
	UT_ASSERT(pFrameData);

        toggleTopRuler(bRulerOn);
	toggleLeftRuler(bRulerOn && (pFrameData->m_pViewMode == VIEW_PRINT));
}

void AP_UnixFrame::toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_UnixFrame::toggleBar %d, %d\n", iBarNb, bBarOn));	

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);
	
	if (bBarOn)
		pFrameData->m_pToolbar[iBarNb]->show();
	else	// turning toolbar off
		pFrameData->m_pToolbar[iBarNb]->hide();
}

void AP_UnixFrame::toggleStatusBar(bool bStatusBarOn)
{
	UT_DEBUGMSG(("AP_UnixFrame::toggleStatusBar %d\n", bStatusBarOn));	

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_return_if_fail(pFrameData && pFrameData->m_pStatusBar);
	
	if (bStatusBarOn)
		pFrameData->m_pStatusBar->show();
	else	// turning status bar off
		pFrameData->m_pStatusBar->hide();
}

bool AP_UnixFrame::_createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom)
{
	//WL: experimentally hiding this
	//gtk_widget_show(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_dArea);
	AP_UnixFrameImpl * pImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
	UT_ASSERT(pImpl);
	UT_DEBUGMSG(("Got FrameImpl %p area %p \n",pImpl,pImpl->m_dArea));
	GR_UnixCairoAllocInfo ai(pImpl->m_dArea);
	pG = (GR_CairoGraphics*) XAP_App::getApp()->newGraphics(ai);

	GtkWidget *widget = GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(getFrameImpl())->m_dArea);
	GR_UnixCairoGraphics *pUnixGraphics = static_cast<GR_UnixCairoGraphics *>(pG);
	GtkWidget * w = gtk_entry_new();
	pUnixGraphics->init3dColors(w);
	gtk_widget_destroy(w);
	pUnixGraphics->initWidget(widget);

	ENSUREP_RF(pG);
	pG->setZoomPercentage(iZoom);

	return true;
}

void AP_UnixFrame::_setViewFocus(AV_View *pView)
{
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
	bool bFocus=GPOINTER_TO_INT(g_object_get_data(G_OBJECT(pFrameImpl->getTopLevelWindow()),
						 "toplevelWindowFocus"));
	pView->setFocus(bFocus && (gtk_grab_get_current()==NULL || gtk_grab_get_current()==pFrameImpl->getTopLevelWindow()) ? AV_FOCUS_HERE : !bFocus && gtk_grab_get_current()!=NULL && isTransientWindow(GTK_WINDOW(gtk_grab_get_current()),GTK_WINDOW(pFrameImpl->getTopLevelWindow())) ?  AV_FOCUS_NEARBY : AV_FOCUS_NONE);
}

void AP_UnixFrame::_bindToolbars(AV_View *pView)
{
	static_cast<AP_UnixFrameImpl *>(getFrameImpl())->_bindToolbars(pView);
}

bool AP_UnixFrame::_createScrollBarListeners(AV_View * pView, AV_ScrollObj *& pScrollObj, 
					     ap_ViewListener *& pViewListener, ap_Scrollbar_ViewListener *& pScrollbarViewListener,
					     AV_ListenerId &lid, AV_ListenerId &lidScrollbarViewListener)
{
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
	ENSUREP_RF(pScrollObj);

	pViewListener = new ap_UnixViewListener(this);
	ENSUREP_RF(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	ENSUREP_RF(pScrollbarViewListener);
	
	if (!pView->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		return false;
	if (!pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		return false;

	return true;
}


UT_sint32 AP_UnixFrame::_getDocumentAreaWidth()
{
	GtkAllocation allocation;
	gtk_widget_get_allocation(GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(getFrameImpl())->m_dArea), &allocation);
	return static_cast<UT_sint32>(allocation.width);
}

UT_sint32 AP_UnixFrame::_getDocumentAreaHeight()
{
	GtkAllocation allocation;
	gtk_widget_get_allocation(GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(getFrameImpl())->m_dArea), &allocation);
	return static_cast<UT_sint32>(allocation.height);
}


UT_sint32 AP_UnixFrame::getDocumentAreaXoff()
{
	GtkAllocation allocation;
	gtk_widget_get_allocation(GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(getFrameImpl())->m_dArea), &allocation);
	return static_cast<UT_sint32>(allocation.x);
}

UT_sint32 AP_UnixFrame::getDocumentAreaYoff()
{
	GtkAllocation allocation;
	gtk_widget_get_allocation(GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(getFrameImpl())->m_dArea), &allocation);
	return static_cast<UT_sint32>(allocation.y);
}
