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

#include <gtk/gtk.h>

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
#include "gr_UnixGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_UnixFrame.h"
#include "ap_UnixFrameImpl.h"
#include "xap_UnixApp.h"
#include "ap_UnixTopRuler.h"
#include "ap_UnixLeftRuler.h"
#include "xap_UnixFontManager.h"
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

void AP_UnixFrame::setZoomPercentage(UT_uint32 iZoom)
{
	XAP_Frame::setZoomPercentage(iZoom);
	_showDocument(iZoom);
}

UT_uint32 AP_UnixFrame::getZoomPercentage(void)
{
	return static_cast<AP_FrameData*>(m_pData)->m_pG->getZoomPercentage();
}

void AP_UnixFrame::setXScrollRange(void)
{
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
	GR_Graphics * pGr = pFrameImpl->getFrame ()->getCurrentView ()->getGraphics ();

	int width = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getWidth();
	int windowWidth = static_cast<int>(pGr->tluD (GTK_WIDGET(pFrameImpl->m_dArea)->allocation.width));
	
	int newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	int newmax = width - windowWidth; /* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;
	
	bool bDifferentPosition = (newvalue != pFrameImpl->m_pHadj->value);
	bool bDifferentLimits = ((width-windowWidth) != pFrameImpl->m_pHadj->upper-
							                        pFrameImpl->m_pHadj->page_size);
		
	pFrameImpl->_setScrollRange(apufi_scrollX, newvalue, static_cast<gfloat>(width), static_cast<gfloat>(windowWidth));
	
	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendHorizontalScrollEvent(newvalue, 
										   static_cast<UT_sint32>
												(pFrameImpl->m_pHadj->upper-
												 pFrameImpl->m_pHadj->page_size));
}

void AP_UnixFrame::setYScrollRange(void)
{
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
	GR_Graphics * pGr = pFrameImpl->getFrame ()->getCurrentView ()->getGraphics ();

	int height = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getHeight();
	int windowHeight = static_cast<int>(pGr->tluD (GTK_WIDGET(pFrameImpl->m_dArea)->allocation.height));

	int newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	int newmax = height - windowHeight;	/* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	bool bDifferentPosition = (newvalue != pGr->tluD (pFrameImpl->m_pVadj->value));
	bool bDifferentLimits ((height-windowHeight) != pFrameImpl->m_pVadj->upper-
													pFrameImpl->m_pVadj->page_size);
	
	pFrameImpl->_setScrollRange(apufi_scrollY, newvalue, static_cast<gfloat>(height), static_cast<gfloat>(windowHeight));
	
	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendVerticalScrollEvent(newvalue, 
										 static_cast<UT_sint32>
											   (pFrameImpl->m_pVadj->upper -
												pFrameImpl->m_pVadj->page_size));
}


AP_UnixFrame::AP_UnixFrame(XAP_UnixApp * pApp)
	: AP_Frame(new AP_UnixFrameImpl(this, pApp), pApp)
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
		static_cast<XAP_App *>(m_pApp)->forgetFrame(pClone);
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
		UT_DEBUGMSG(("AP_UnixFrame::initialize!!!! NormalFrame this %x \n",this));
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
	gfloat yoffMax = pFrameImpl->m_pVadj->upper - 
		pFrameImpl->m_pVadj->page_size;
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

	gtk_adjustment_set_value(GTK_ADJUSTMENT(pFrameImpl->m_pVadj),yoffNew);

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
	gfloat xoffMax = pFrameImpl->m_pHadj->upper - 
		pFrameImpl->m_pHadj->page_size;
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

	gtk_adjustment_set_value(GTK_ADJUSTMENT(pFrameImpl->m_pHadj),xoffDisc);

	// (this is the calculation for dx again, post rounding)
	// This may not actually be helpful, because we could still lose if the
	// second round of rounding gives us the wrong number.  Leave it for now.
	if (pG->tdu(static_cast<UT_sint32>(xoffDisc) - 
				pView->getXScrollOffset()) != 0)
		pView->setXScrollOffset(static_cast<UT_sint32>(xoffDisc));
}

void AP_UnixFrame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{
	UT_ASSERT_NOT_REACHED();
}

void AP_UnixFrame::setStatusMessage(const char * szMsg)
{
	if(getFrameMode() == XAP_NormalFrame)
	{
		static_cast<AP_FrameData *>(m_pData)->m_pStatusBar->setStatusMessage(szMsg);
	}
}

void AP_UnixFrame::toggleTopRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(getFrameData());
	UT_ASSERT(pFrameData);
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
		
	AP_UnixTopRuler * pUnixTopRuler = NULL;

	UT_DEBUGMSG(("AP_UnixFrame::toggleTopRuler %d, %d\n", 
		     bRulerOn, pFrameData->m_pTopRuler));

	if ( bRulerOn )
	{
		AP_TopRuler * pTop = pFrameData->m_pTopRuler;
		if(pTop)
		{
			delete pTop;
		}
		pUnixTopRuler = new AP_UnixTopRuler(this);
		UT_ASSERT(pUnixTopRuler);
		pFrameImpl->m_topRuler = pUnixTopRuler->createWidget();

		// attach everything	
		gtk_table_attach(GTK_TABLE(pFrameImpl->m_innertable), 
				 pFrameImpl->m_topRuler, 0, 2, 0, 1, 
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL),
				 0, 0);
		FV_View * pView = static_cast<FV_View *>(m_pView);
		UT_uint32 iZoom = pView->getGraphics()->getZoomPercentage();
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
		  AP_TopRuler * pTop = pFrameData->m_pTopRuler;
		  if(pTop && !pTop->isHidden())
		  {
			  gtk_object_destroy( GTK_OBJECT(pFrameImpl->m_topRuler) );
		  }
		  DELETEP(static_cast<AP_FrameData*>(m_pData)->m_pTopRuler);
		  pFrameImpl->m_topRuler = NULL;
		  static_cast<FV_View *>(m_pView)->setTopRuler(NULL);
	  }

	static_cast<AP_FrameData*>(m_pData)->m_pTopRuler = pUnixTopRuler;
}

void AP_UnixFrame::toggleLeftRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(getFrameData());
	UT_ASSERT(pFrameData);
	AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());

	AP_UnixLeftRuler * pUnixLeftRuler = NULL;

	UT_DEBUGMSG(("AP_UnixFrame::toggleLeftRuler %d, %d\n", 
		     bRulerOn, pFrameData->m_pLeftRuler));

	if (bRulerOn)
	{
//
// if there is an old ruler just return.
//
		AP_LeftRuler * pLeft = pFrameData->m_pLeftRuler;
		if(pLeft)
		{
			delete pLeft;
		}
		pUnixLeftRuler = new AP_UnixLeftRuler(this);
		UT_ASSERT(pUnixLeftRuler);
		pFrameImpl->m_leftRuler = pUnixLeftRuler->createWidget();

		gtk_table_attach(GTK_TABLE(pFrameImpl->m_innertable), 
				 pFrameImpl->m_leftRuler, 0, 1, 1, 2,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 0,0);
		FV_View * pView = static_cast<FV_View *>(m_pView);
		UT_uint32 iZoom = pView->getGraphics()->getZoomPercentage();
		static_cast<AP_LeftRuler *>(pUnixLeftRuler)->setView(m_pView,iZoom);
		setYScrollRange();
	}
	else
	{
	    if (pFrameImpl->m_leftRuler && GTK_IS_OBJECT(pFrameImpl->m_leftRuler))
			gtk_object_destroy(GTK_OBJECT(pFrameImpl->m_leftRuler) );
	    
	    DELETEP(static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler);
	    pFrameImpl->m_leftRuler = NULL;
		static_cast<FV_View *>(m_pView)->setLeftRuler(NULL);
	}

	static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler = pUnixLeftRuler;
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
	UT_ASSERT(pFrameData);
	
	if (bStatusBarOn)
		pFrameData->m_pStatusBar->show();
	else	// turning status bar off
		pFrameData->m_pStatusBar->hide();
}

bool AP_UnixFrame::_createViewGraphics(GR_Graphics *& pG, UT_uint32 iZoom)
{
	XAP_UnixFontManager * fontManager = (static_cast<XAP_UnixApp *>(getApp())->getFontManager());
	//WL: experimentally hiding this
	//gtk_widget_show(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_dArea);
	AP_UnixFrameImpl * pImpl = static_cast<AP_UnixFrameImpl *>(getFrameImpl());
	UT_ASSERT(pImpl);
	UT_DEBUGMSG(("Got FrameImpl %x area %x \n",pImpl,pImpl->m_dArea));
	pG = new GR_UnixGraphics(pImpl->m_dArea->window, fontManager, getApp());
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
	return static_cast<UT_sint32>(GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(getFrameImpl())->m_dArea)->allocation.width);
}

UT_sint32 AP_UnixFrame::_getDocumentAreaHeight()
{
	return static_cast<UT_sint32>(GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(getFrameImpl())->m_dArea)->allocation.height);
}
