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

#include "gr_CocoaGraphics.h"
#include "ap_CocoaTopRuler.h"

#import "ap_CocoaFrameImpl.h"

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

@interface AP_CocoaTopRulerDelegate : NSObject <XAP_MouseEventDelegate>
{
	AP_CocoaTopRuler* _xap;
}
- (void)setXAPOwner:(AP_CocoaTopRuler*)owner;
- (void)viewDidResize:(NSNotification*)notif;
@end


AP_CocoaTopRuler::AP_CocoaTopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_rootWindow = NULL;
	m_wTopRuler = NULL;
	m_pG = NULL;

	m_wTopRuler = [(AP_CocoaFrameController *)(static_cast<XAP_CocoaFrameImpl *>(m_pFrame->getFrameImpl())->_getController()) getHRuler];
}

AP_CocoaTopRuler::~AP_CocoaTopRuler(void)
{
	DELETEP(m_pG);
}



XAP_CocoaNSView * AP_CocoaTopRuler::createWidget(void)
{
	//UT_DEBUGMSG(("AP_CocoaTopRuler::createWidget - [w=%p] [this=%p]\n", m_wTopRuler,this));
	return m_wTopRuler;
}

void AP_CocoaTopRuler::setView(AV_View * pView)
{
	AP_CocoaTopRulerDelegate* delegate;
	AP_TopRuler::setView(pView);

	DELETEP(m_pG);

	GR_CocoaGraphics * pG = new GR_CocoaGraphics(m_wTopRuler, m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);
	delegate = [[AP_CocoaTopRulerDelegate alloc] init];
	[m_wTopRuler setEventDelegate:delegate];
	[delegate setXAPOwner:this];
	[[NSNotificationCenter defaultCenter] addObserver:delegate
			selector:@selector(viewDidResize:) 
			name:NSViewFrameDidChangeNotification object:m_wTopRuler];
	[delegate release];
//	static_cast<GR_CocoaGraphics *>(m_pG)->_setUpdateCallback (&_graphicsUpdateCB, (void *)this);
	NSRect bounds = [m_wTopRuler bounds];
	setWidth(lrintf(bounds.size.width));
	setHeight(lrintf(bounds.size.height));
}

void AP_CocoaTopRuler::getWidgetPosition(int * x, int * y)
{
	UT_ASSERT(x && y);
	
	NSRect theBounds = [m_wTopRuler bounds];
	*x = (int)theBounds.size.width;
	*y = (int)theBounds.size.height;
}

NSWindow * AP_CocoaTopRuler::getRootWindow(void)
{
	// TODO move this function somewhere more logical, like
	// TODO the XAP_Frame level, since that's where the
	// TODO root window is common to all descendants.
	if (m_rootWindow)
		return m_rootWindow;

	m_rootWindow  = [m_wTopRuler window];
	return m_rootWindow;
}


bool AP_CocoaTopRuler::_graphicsUpdateCB(NSRect * aRect, GR_CocoaGraphics *pG, void* param)
{
	// a static function
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)param;
	if (!pCocoaTopRuler)
		return false;

	UT_Rect rClip;
	rClip.left = (UT_sint32)aRect->origin.x;
	rClip.top = (UT_sint32)aRect->origin.y;
	rClip.width = (UT_sint32)aRect->size.width;
	rClip.height = (UT_sint32)aRect->size.height;
	xxx_UT_DEBUGMSG(("Cocoa in topruler expose painting area:  left=%d, top=%d, width=%d, height=%d\n", rClip.left, rClip.top, rClip.width, rClip.height));
	if(pG != NULL)
	{
//		pCocoaTopRuler->getGraphics()->doRepaint(&rClip);
		pCocoaTopRuler->draw(&rClip);
	}
	else {
		return false;
	}
	return true;
}
		
/*****************************************************************/


@implementation AP_CocoaTopRulerDelegate

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

- (void)setXAPOwner:(AP_CocoaTopRuler*)owner
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

	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags]);
	emb = EV_CocoaMouse::_convertMouseButton([theEvent buttonNumber]);

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
	
	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags]);

	// Map the mouse into coordinates relative to our window.
	NSPoint pt = [theEvent locationInWindow];
	pt = [sender convertPoint:pt fromView:nil];
	GR_CocoaGraphics* pGr = dynamic_cast<GR_CocoaGraphics*>(_xap->getGraphics());
	if (!pGr->_isFlipped()) {
		pt.y = [sender bounds].size.height - pt.y;
	}
	_xap->mouseMotion(ems, (UT_sint32)pGr->tluD(pt.x), (UT_sint32)pGr->tluD(pt.y));
	_xap->isMouseOverTab((UT_uint32)pGr->tluD(pt.x),(UT_uint32)pGr->tluD(pt.y));
}


- (void)mouseUp:(NSEvent *)theEvent from:(id)sender
{
	EV_EditModifierState ems = 0;
	EV_EditMouseButton emb = 0;

	ems = EV_CocoaMouse::_convertModifierState([theEvent modifierFlags]);
	emb = EV_CocoaMouse::_convertMouseButton([theEvent buttonNumber]);

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
