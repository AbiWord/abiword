/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#import "ev_CocoaMouse.h"
#include "xap_Frame.h"
#include "xap_CocoaFrame.h"
#import "ap_FrameData.h"
#include "ap_CocoaFrame.h"
#include "ap_CocoaLeftRuler.h"
#include "gr_CocoaGraphics.h"
#include "ut_sleep.h"

#import "ap_CocoaFrameImpl.h"

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

@interface AP_CocoaLeftRulerDelegate : NSObject <XAP_MouseEventDelegate>
{
}
@end

/*****************************************************************/

AP_CocoaLeftRuler::AP_CocoaLeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame)
{
	m_rootWindow = nil;
	m_wLeftRuler = nil;
	m_pG = NULL;
    // change ruler color on theme change
	m_wLeftRuler = [(AP_CocoaFrameController *)(static_cast<AP_CocoaFrameImpl*>(m_pFrame->getFrameImpl())->_getController()) getVRuler];
}

AP_CocoaLeftRuler::~AP_CocoaLeftRuler(void)
{
	while(m_pG->isSpawnedRedraw())
	{
		UT_usleep(100);
	}
	DELETEP(m_pG);
}

XAP_CocoaNSView * AP_CocoaLeftRuler::createWidget(void)
{
#if 0
	g_signal_connect(G_OBJECT(m_wLeftRuler), "configure_event",
					   G_CALLBACK(_fe::configure_event), NULL);
	if( m_iBackgroundRedrawID == 0)
	{
//
// Start background repaint
//
		m_iBackgroundRedrawID = gtk_timeout_add(200,(GtkFunction) _fe::abi_expose_repaint, (gpointer) this);
	}
	else
    {
		gtk_timeout_remove(m_iBackgroundRedrawID);
		m_iBackgroundRedrawID = gtk_timeout_add(200,(GtkFunction) _fe::abi_expose_repaint, (gpointer) this);
	}
#endif

	return m_wLeftRuler;
}

void AP_CocoaLeftRuler::setView(AV_View * pView)
{
	AP_LeftRuler::setView(pView);

	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wLeftRuler->window)
	// is not created until the frame's top-level window is
	// shown.
	
	DELETEP(m_pG);

	GR_CocoaGraphics * pG = new GR_CocoaGraphics(m_wLeftRuler, m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);
	[m_wLeftRuler setEventDelegate:[[[AP_CocoaLeftRulerDelegate alloc] init] autorelease]];
	static_cast<GR_CocoaGraphics *>(m_pG)->_setUpdateCallback (&_graphicsUpdateCB, (void *)this);

//	GtkWidget * ruler = gtk_vruler_new ();
// TODO	pG->init3dColors(get_ensured_style (ruler));
}

void AP_CocoaLeftRuler::getWidgetPosition(int * x, int * y)
{
	UT_ASSERT(x && y);

	NSRect theBounds = [m_wLeftRuler bounds];
	*x = (int)theBounds.size.width;
	*y = (int)theBounds.size.height;
}

NSWindow * AP_CocoaLeftRuler::getRootWindow(void)
{
	// TODO move this function somewhere more logical, like
	// TODO the XAP_Frame level, since that's where the
	// TODO root window is common to all descendants.
	if (m_rootWindow)
		return m_rootWindow;

	m_rootWindow  = [m_wLeftRuler window];
	return m_rootWindow;
}


bool AP_CocoaLeftRuler::_graphicsUpdateCB(NSRect * aRect, GR_CocoaGraphics *pG, void* param)
{
	// a static function
	AP_CocoaLeftRuler * pCocoaLeftRuler = (AP_CocoaLeftRuler *)param;
	if (!pCocoaLeftRuler)
		return false;

	UT_Rect rClip;
	rClip.left = (UT_sint32)aRect->origin.x;
	rClip.top = (UT_sint32)aRect->origin.y;
	rClip.width = (UT_sint32)aRect->size.width;
	rClip.height = (UT_sint32)aRect->size.height;
	xxx_UT_DEBUGMSG(("Cocoa in leftruler expose painting area:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	if(pG != NULL)
	{
//		pCocoaLeftRuler->getGraphics()->doRepaint(&rClip);
		pCocoaLeftRuler->draw(&rClip);
	}
	else {
		return false;
	}
#if 0
	else
	{
		UT_DEBUGMSG(("No graphics Context. Doing fallback. \n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		pCocoaLeftRuler->draw(&rClip);
	}
#endif
	return true;
}

/*****************************************************************/

#if 0
/*!
 * Background abi repaint function.
\param XAP_CocoaFrame * p pointer to the Frame that initiated this background
       repainter.
 */
gint AP_CocoaLeftRuler::_fe::abi_expose_repaint( gpointer p)
{
//
// Grab our pointer so we can do useful stuff.
//
	UT_Rect localCopy;
	AP_CocoaLeftRuler * pR = static_cast<AP_CocoaLeftRuler *>(p);
	GR_Graphics * pG = pR->getGraphics();
	if(pG == NULL || pG->isDontRedraw())
	{
//
// Come back later
//
		return TRUE;
	}
	pG->setSpawnedRedraw(true);
	if(pG->isExposePending())
	{
		while(pG->isExposedAreaAccessed())
		{
			UT_usleep(10); // 10 microseconds
		}
		pG->setExposedAreaAccessed(true);
		localCopy.set(pG->getPendingRect()->left,pG->getPendingRect()->top,
					  pG->getPendingRect()->width,pG->getPendingRect()->height);
//
// Clear out this set of expose info
//
		pG->setExposePending(false);
		pG->setExposedAreaAccessed(false);
		xxx_UT_DEBUGMSG(("Painting area on Left ruler:  left=%d, top=%d, width=%d, height=%d\n", localCopy.left, localCopy.top, localCopy.width, localCopy.height));
		xxx_UT_DEBUGMSG(("SEVIOR: Repaint now \n"));
		pR->draw(&localCopy);
	}
//
// OK we've finshed. Wait for the next signal
//
	pG->setSpawnedRedraw(false);
	return TRUE;
}
#endif

@implementation AP_CocoaLeftRulerDelegate

- (void)mouseDown:(NSEvent *)theEvent from:(id)sender
{
	XAP_Frame* pFrame = [(XAP_CocoaNSView*)sender xapFrame];
	AP_FrameData * pFrameData = (AP_FrameData *)pFrame->getFrameData();
	AP_CocoaLeftRuler * pCocoaLeftRuler = (AP_CocoaLeftRuler *)pFrameData->m_pLeftRuler;

	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;

	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags]);
	emb = EV_CocoaMouse::_convertMouseButton([theEvent buttonNumber]);

	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	pCocoaLeftRuler->mousePress(ems, emb, (UT_uint32) pt.x, (UT_uint32) pt.y);
}


- (void)mouseDragged:(NSEvent *)theEvent from:(id)sender
{
	XAP_Frame* pFrame = [(XAP_CocoaNSView*)sender xapFrame];
	AP_FrameData * pFrameData = (AP_FrameData *)pFrame->getFrameData();
	AP_CocoaLeftRuler * pCocoaLeftRuler = (AP_CocoaLeftRuler *)pFrameData->m_pLeftRuler;

	EV_EditModifierState ems = 0;
	
	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags]);

	// Map the mouse into coordinates relative to our window.
	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	pCocoaLeftRuler->mouseMotion(ems,(UT_sint32)pt.x, (UT_sint32)pt.y);
}


- (void)mouseUp:(NSEvent *)theEvent from:(id)sender
{
	XAP_Frame* pFrame = [(XAP_CocoaNSView*)sender xapFrame];
	AP_FrameData * pFrameData = (AP_FrameData *)pFrame->getFrameData();
	AP_CocoaLeftRuler * pCocoaLeftRuler = (AP_CocoaLeftRuler *)pFrameData->m_pLeftRuler;

	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;

	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags]);
	emb = EV_CocoaMouse::_convertMouseButton([theEvent buttonNumber]);

	// Map the mouse into coordinates relative to our window.
	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	pCocoaLeftRuler->mouseRelease(ems, emb, (UT_sint32)pt.x, (UT_sint32)pt.y);
}
@end
