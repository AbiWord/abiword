/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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
	AP_CocoaLeftRuler* _xap;
}
- (void)setXAPOwner:(AP_CocoaLeftRuler*)owner;
- (void)viewDidResize:(NSNotification*)notif;
@end

/*****************************************************************/

AP_CocoaLeftRuler::AP_CocoaLeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame),
		m_wLeftRuler(nil),
		m_rootWindow(nil)
{
	m_wLeftRuler = [(AP_CocoaFrameController *)(static_cast<AP_CocoaFrameImpl*>(m_pFrame->getFrameImpl())->_getController()) getVRuler];
}

AP_CocoaLeftRuler::~AP_CocoaLeftRuler(void)
{
	while(m_pG->isSpawnedRedraw())
	{
		UT_usleep(100);
	}
	static_cast<GR_CocoaGraphics *>(m_pG)->_setUpdateCallback (NULL, NULL);
	DELETEP(m_pG);
	if (m_delegate) {
		[[NSNotificationCenter defaultCenter] removeObserver:m_delegate];
		[m_wLeftRuler setEventDelegate:nil];
		[m_delegate release];
	}
}

XAP_CocoaNSView * AP_CocoaLeftRuler::createWidget(void)
{
	return m_wLeftRuler;
}

void AP_CocoaLeftRuler::setView(AV_View * pView)
{
	AP_LeftRuler::setView(pView);
	
	DELETEP(m_pG);

	GR_CocoaAllocInfo ai(m_wLeftRuler, m_pFrame->getApp());
	GR_CocoaGraphics * pG = static_cast<GR_CocoaGraphics*>(XAP_App::getApp()->newGraphics(ai));
	m_pG = pG;
	UT_ASSERT(m_pG);
	m_delegate = [[AP_CocoaLeftRulerDelegate alloc] init];
	[m_wLeftRuler setEventDelegate:m_delegate];
	[m_delegate setXAPOwner:this];
	[[NSNotificationCenter defaultCenter] addObserver:m_delegate
			selector:@selector(viewDidResize:) 
			name:NSViewFrameDidChangeNotification object:m_wLeftRuler];
	static_cast<GR_CocoaGraphics *>(m_pG)->_setUpdateCallback (&_graphicsUpdateCB, (void *)this);
	NSRect bounds = [m_wLeftRuler bounds];
	setWidth(lrintf(bounds.size.width));
	setHeight(lrintf(bounds.size.height));
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


@implementation AP_CocoaLeftRulerDelegate

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}


- (void)setXAPOwner:(AP_CocoaLeftRuler*)owner
{
	_xap = owner;
}

- (void)viewDidResize:(NSNotification*)notif
{
	NSRect bounds = [[notif object] bounds];
	_xap->setWidth(lrintf(bounds.size.width));
	_xap->setHeight(lrintf(bounds.size.height));
	_xap->draw(NULL);
}

- (void)mouseDown:(NSEvent *)theEvent from:(id)sender
{
	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;
	bool rightBtn = false;

	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags], rightBtn);
	emb = EV_CocoaMouse::_convertMouseButton([theEvent buttonNumber], rightBtn);

	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	GR_CocoaGraphics* pGr = dynamic_cast<GR_CocoaGraphics*>(_xap->getGraphics());
	if (!pGr->_isFlipped()) {
		pt.y = [sender bounds].size.height - pt.y;
	}
	_xap->mousePress(ems, emb, (UT_uint32)pGr->tluD(pt.x), (UT_uint32)pGr->tluD(pt.y));
}


- (void)mouseDragged:(NSEvent *)theEvent from:(id)sender
{
	EV_EditModifierState ems = 0;
	bool rightBtn = false;
	
	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags], rightBtn);

	// Map the mouse into coordinates relative to our window.
	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	GR_CocoaGraphics* pGr = dynamic_cast<GR_CocoaGraphics*>(_xap->getGraphics());
	if (!pGr->_isFlipped()) {
		pt.y = [sender bounds].size.height - pt.y;
	}
	_xap->mouseMotion(ems,(UT_sint32)pGr->tluD(pt.x), (UT_sint32)pGr->tluD(pt.y));
}


- (void)mouseUp:(NSEvent *)theEvent from:(id)sender
{
	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;
	bool rightBtn = false;

	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags], rightBtn);
	emb = EV_CocoaMouse::_convertMouseButton([theEvent buttonNumber], rightBtn);

	// Map the mouse into coordinates relative to our window.
	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	GR_CocoaGraphics* pGr = dynamic_cast<GR_CocoaGraphics*>(_xap->getGraphics());
	if (!pGr->_isFlipped()) {
		pt.y = [sender bounds].size.height - pt.y;
	}
	_xap->mouseRelease(ems, emb, (UT_sint32)pGr->tluD(pt.x), (UT_sint32)pGr->tluD(pt.y));
}
@end
