/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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


#import <Cocoa/Cocoa.h>

#import "ut_types.h"
#import "ut_debugmsg.h"
#import "ut_assert.h"
#import "gr_CocoaGraphics.h"
#import "ev_CocoaToolbar.h"
#import "xav_View.h"
#import "xap_CocoaApp.h"
#import "ap_FrameData.h"
#import "ap_CocoaFrame.h"
#import "ap_CocoaFrameImpl.h"
#import "ap_CocoaTopRuler.h"
#import "ap_CocoaLeftRuler.h"
#import "ap_CocoaStatusBar.h"

/*****************************************************************/
AP_CocoaFrameImpl::AP_CocoaFrameImpl(AP_CocoaFrame *pCocoaFrame, XAP_CocoaApp *pCocoaApp)
	: XAP_CocoaFrameImpl (pCocoaFrame, pCocoaApp)
{
	m_hScrollbar = NULL;
	m_vScrollbar = NULL;
	m_docAreaGRView = NULL;
}


void AP_CocoaFrameImpl::_createDocView(GR_CocoaGraphics* &pG)
{
	XAP_Frame*	pFrame = getFrame();
	NSView*		docArea = [_getController() getMainView];
	NSArray*	docAreaSubviews;
	
	docAreaSubviews = [docArea subviews];
	if ([docAreaSubviews count] != 0) {
		NSEnumerator *enumerator = [docAreaSubviews objectEnumerator];
		NSView* aSubview;
	
		while (aSubview = [enumerator nextObject]) {
			[aSubview removeFromSuperviewWithoutNeedingDisplay];
		}
		
		m_hScrollbar = NULL;
		m_vScrollbar = NULL;
		m_docAreaGRView = NULL;
	}
		NSRect frame = [docArea bounds];
	NSRect controlFrame;
	
	/* vertical scrollbar */
	controlFrame.origin.y = [NSScroller scrollerWidth];
	controlFrame.size.width = [NSScroller scrollerWidth];
	controlFrame.size.height = frame.size.height - controlFrame.origin.y;
	controlFrame.origin.x = frame.size.width - controlFrame.size.width;
	m_vScrollbar = [[NSScroller alloc] initWithFrame:controlFrame];
	[docArea addSubview:m_vScrollbar];
	[m_vScrollbar setAutoresizingMask:(NSViewMinXMargin |  NSViewHeightSizable)];
	[m_vScrollbar release];
	
	/* horizontal scrollbar */
	controlFrame.origin.x = 0;
	controlFrame.origin.y = 0;
	controlFrame.size.height = [NSScroller scrollerWidth];
	controlFrame.size.width = frame.size.width - controlFrame.size.height;
	m_hScrollbar = [[NSScroller alloc] initWithFrame:controlFrame];
	[docArea addSubview:m_hScrollbar];
	[m_hScrollbar setAutoresizingMask:(NSViewMaxYMargin |  NSViewWidthSizable)];
	[m_hScrollbar release];

	/* doc view */
	controlFrame.origin.x = 0;
	controlFrame.origin.y = [NSScroller scrollerWidth];
	controlFrame.size.height = frame.size.height - controlFrame.origin.y;
	controlFrame.size.width = frame.size.width - [NSScroller scrollerWidth];
	m_docAreaGRView = [[XAP_CocoaNSView alloc] initWith:pFrame andFrame:controlFrame];
	[docArea addSubview:m_docAreaGRView];
	[m_docAreaGRView setAutoresizingMask:(NSViewHeightSizable | NSViewWidthSizable)];
	[m_docAreaGRView release];

	
	pG = new GR_CocoaGraphics(m_docAreaGRView, /*fontManager,*/ pFrame->getApp());
	static_cast<GR_CocoaGraphics *>(pG)->_setUpdateCallback (&_graphicsUpdateCB, (void *)this);
	

}


void AP_CocoaFrameImpl::_bindToolbars(AV_View * pView)
{
	int nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (int k = 0; k < nrToolbars; k++)
	{
		// TODO Toolbars are a frame-level item, but a view-listener is
		// TODO a view-level item.  I've bound the toolbar-view-listeners
		// TODO to the current view within this frame and have code in the
		// TODO toolbar to allow the view-listener to be rebound to a different
		// TODO view.  in the future, when we have support for multiple views
		// TODO in the frame (think splitter windows), we will need to have
		// TODO a loop like this to help change the focus when the current
		// TODO view changes.
		
		EV_CocoaToolbar * pCocoaToolbar = (EV_CocoaToolbar *)m_vecToolbars.getNthItem(k);
		pCocoaToolbar->bindListenerToView(pView);
	}
}

// Does the initial show/hide of statusbar (based on the user prefs).
// Idem.
void AP_CocoaFrameImpl::_showOrHideStatusbar()
{
	XAP_Frame*	pFrame = getFrame();
	bool bShowStatusBar = static_cast<AP_FrameData*> (pFrame->getFrameData())->m_bShowStatusBar && false;
	static_cast<AP_CocoaFrame *>(pFrame)->toggleStatusBar(bShowStatusBar);
}

// Does the initial show/hide of toolbars (based on the user prefs).
// This is needed because toggleBar is called only when the user
// (un)checks the show {Stantandard,Format,Extra} toolbar checkbox,
// and thus we have to manually call this function at startup.
void AP_CocoaFrameImpl::_showOrHideToolbars()
{
	XAP_Frame*	pFrame = getFrame();
	bool *bShowBar = static_cast<AP_FrameData*> (pFrame->getFrameData())->m_bShowBar;
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	for (UT_uint32 i = 0; i < cnt; i++)
	{
		// TODO: The two next lines are here to bind the EV_Toolbar to the
		// AP_FrameData, but their correct place are next to the toolbar creation (JCA)
		EV_CocoaToolbar * pCocoaToolbar = static_cast<EV_CocoaToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*> (pFrame->getFrameData())->m_pToolbar[i] = pCocoaToolbar;
		static_cast<AP_CocoaFrame *>(pFrame)->toggleBar(i, bShowBar[i]);
	}
}


/*!
 * Refills the framedata class with pointers to the current toolbars. We 
 * need to do this after a toolbar icon and been dragged and dropped.
 */
void AP_CocoaFrameImpl::_refillToolbarsInFrameData(void)
{
	XAP_Frame*	pFrame = getFrame();
	UT_uint32 cnt = m_vecToolbarLayoutNames.getItemCount();

	for (UT_uint32 i = 0; i < cnt; i++)
	{
		EV_CocoaToolbar * pCocoaToolbar = static_cast<EV_CocoaToolbar *> (m_vecToolbars.getNthItem(i));
		static_cast<AP_FrameData*> (pFrame->getFrameData())->m_pToolbar[i] = pCocoaToolbar;
	}
}

void AP_CocoaFrameImpl::_createDocumentWindow()
{
	XAP_Frame*	pFrame = getFrame();
	AP_FrameData* pData = static_cast<AP_FrameData*> (pFrame->getFrameData());
	bool bShowRulers = pData->m_bShowRuler;

	// create the rulers
	AP_CocoaTopRuler * pCocoaTopRuler = NULL;
	AP_CocoaLeftRuler * pCocoaLeftRuler = NULL;

	if ( bShowRulers )
	{
		pCocoaTopRuler = new AP_CocoaTopRuler(pFrame);
		UT_ASSERT(pCocoaTopRuler);
		pCocoaTopRuler->createWidget();
		
		if (pData->m_pViewMode == VIEW_PRINT) {
		    pCocoaLeftRuler = new AP_CocoaLeftRuler(pFrame);
		    UT_ASSERT(pCocoaLeftRuler);
		    pCocoaLeftRuler->createWidget();

		    // get the width from the left ruler and stuff it into the top ruler.
		    pCocoaTopRuler->setOffsetLeftRuler(pCocoaLeftRuler->getWidth());
		}
		else {
//		    m_leftRuler = NULL;
		    pCocoaTopRuler->setOffsetLeftRuler(0);
		}
	}

	pData->m_pTopRuler = pCocoaTopRuler;
	pData->m_pLeftRuler = pCocoaLeftRuler;

	// TODO check what really remains to be done.
//	UT_ASSERT (UT_TODO);
#if 0
//	g_signal_connect(G_OBJECT(m_pHadj), "value_changed", G_CALLBACK(_fe::hScrollChanged), NULL);
//	g_signal_connect(G_OBJECT(m_pVadj), "value_changed", G_CALLBACK(_fe::vScrollChanged), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "expose_event",
					   G_CALLBACK(_fe::expose), NULL);
  
	g_signal_connect(G_OBJECT(m_dArea), "button_press_event",
					   G_CALLBACK(_fe::button_press_event), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "button_release_event",
					   G_CALLBACK(_fe::button_release_event), NULL);

	g_signal_connect(G_OBJECT(m_dArea), "motion_notify_event",
					   G_CALLBACK(_fe::motion_notify_event), NULL);
  
	g_signal_connect(G_OBJECT(m_dArea), "configure_event",
					   G_CALLBACK(_fe::configure_event), NULL);
#endif
}

void AP_CocoaFrameImpl::_createStatusBarWindow(XAP_CocoaNSStatusBar * statusBar)
{
	XAP_Frame*	pFrame = getFrame();
	
	UT_DEBUGMSG (("AP_CocoaFrame::_createStatusBarWindow ()\n"));
	// TODO: pass the NSView instead of the whole frame
	AP_CocoaStatusBar * pCocoaStatusBar = new AP_CocoaStatusBar(pFrame);
	UT_ASSERT(pCocoaStatusBar);

	((AP_FrameData *)pFrame->getFrameData())->m_pStatusBar = pCocoaStatusBar;
}

void AP_CocoaFrameImpl::_setWindowIcon()
{
	// this is NOT needed. Just need to the the title.
}

NSString *	AP_CocoaFrameImpl::_getNibName ()
{
	return @"ap_CocoaFrame";
}

/*!
	Create and intialize the controller.
 */
XAP_CocoaFrameController *AP_CocoaFrameImpl::_createController()
{
	UT_DEBUGMSG (("AP_CocoaFrame::_createController()\n"));
	return [[AP_CocoaFrameController createFrom:this] retain];
}



bool AP_CocoaFrameImpl::_graphicsUpdateCB(NSRect * aRect, GR_CocoaGraphics *pG, void* param)
{
	// a static function
	AP_CocoaFrame * pCocoaFrame = (AP_CocoaFrame *)param;
	if (!pCocoaFrame)
		return false;

	UT_Rect rClip;
	rClip.left = aRect->origin.x;
	rClip.top = aRect->origin.y;
	rClip.width = aRect->size.width;
	rClip.height = aRect->size.height;
	xxx_UT_DEBUGMSG(("Cocoa in frame expose painting area:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	if(pG != NULL)
		pG->doRepaint(&rClip);
	else
		return false;
	return true;
}

/* Objective-C section */

@implementation AP_CocoaFrameController
+ (XAP_CocoaFrameController*)createFrom:(XAP_CocoaFrameImpl *)frame
{
	UT_DEBUGMSG (("Cocoa: @AP_CocoaFrameController createFrom:frame\n"));
	AP_CocoaFrameController *obj = [[AP_CocoaFrameController alloc] initWith:frame];
	return [obj autorelease];
}

- (id)initWith:(XAP_CocoaFrameImpl *)frame
{
	UT_DEBUGMSG (("Cocoa: @AP_CocoaFrameController initWith:frame\n"));
	return [super initWith:frame];
}


- (IBAction)rulerClick:(id)sender
{
}

- (XAP_CocoaNSView *)getVRuler
{
	return vRuler;
}

- (XAP_CocoaNSView *)getHRuler
{
	return hRuler;
}


@end
