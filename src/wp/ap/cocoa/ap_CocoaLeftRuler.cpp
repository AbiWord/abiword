/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_sleep.h"

#include "gr_CocoaCairoGraphics.h"
#include "gr_Painter.h"

#include "ev_CocoaMouse.h"

#include "xap_App.h"
#include "xap_CocoaFrame.h"
#include "xap_Frame.h"

#include "ap_CocoaFrame.h"
#include "ap_CocoaFrameImpl.h"
#include "ap_CocoaLeftRuler.h"
#include "ap_FrameData.h"

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
		m_rootWindow(nil),
		m_delegate(nil)
{
	m_wLeftRuler = [(AP_CocoaFrameController *)(static_cast<AP_CocoaFrameImpl*>(m_pFrame->getFrameImpl())->_getController()) getVRuler];
}

AP_CocoaLeftRuler::~AP_CocoaLeftRuler(void)
{
	static_cast<GR_CocoaCairoGraphics *>(m_pG)->_setUpdateCallback (NULL, NULL);
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

	if (m_delegate)
		return;

	DELETEP(m_pG);

	GR_CocoaCairoAllocInfo ai(m_wLeftRuler);
	GR_CocoaCairoGraphics * pG = static_cast<GR_CocoaCairoGraphics*>(XAP_App::getApp()->newGraphics(ai));
	UT_ASSERT(pG);
	m_pG = pG;

	m_delegate = [[AP_CocoaLeftRulerDelegate alloc] init];
	[m_wLeftRuler setEventDelegate:m_delegate];
	[m_delegate setXAPOwner:this];
	[[NSNotificationCenter defaultCenter] addObserver:m_delegate
			selector:@selector(viewDidResize:) 
			name:NSViewFrameDidChangeNotification object:m_wLeftRuler];

	pG->_setUpdateCallback(&_graphicsUpdateCB, (void *)this);
	pG->setGrabCursor(GR_Graphics::GR_CURSOR_UPDOWN);

	NSRect bounds = [m_wLeftRuler bounds];
	setWidth(lrintf(bounds.size.width));
	setHeight(lrintf(bounds.size.height));
}

void AP_CocoaLeftRuler::getWidgetPosition(int * x, int * y)
{
	UT_ASSERT(x && y);

	NSRect theBounds = [m_wLeftRuler bounds]; // FIXME ?? [... frame], or just remove method? also TopRuler
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

#if 0
void AP_CocoaLeftRuler::_drawMarginProperties(const UT_Rect * /* pClipRect */, AP_LeftRulerInfo * pInfo, 
                                              GR_Graphics::GR_Color3D /*clr*/)
{
	if (!m_pG)
		return;

	UT_Rect rTop;
	UT_Rect rBottom;

	_getMarginMarkerRects(pInfo, rTop, rBottom);
	
	GR_Painter painter(m_pG);

	GR_CocoaCairoGraphics * pG = static_cast<GR_CocoaCairoGraphics *>(m_pG);

	UT_Point control[4];
	UT_Point control2[4];

	control[0].x = 0;
	control[0].y = 0;
	control[1].x = 0;
	control[1].y = 6;
	control[2].x = 6;
	control[2].y = 6;
	control[3].x = 6;
	control[3].y = 0;

	UT_RGBColor lineColor;
	GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor knobColor], lineColor);
	UT_RGBColor fillColor;
	fillColor = pG->VBlue();

	UT_sint32 l = rTop.left;
	UT_sint32 t = rTop.top;

	for(int i = 0; i < 4; i++) {
		control2[i].x = control[i].x + l;
		control2[i].y = control[i].y + t;
	}

	pG->polygon(fillColor, control2, 4);
	pG->setColor(lineColor);
	pG->polyLine(control2, 4);

	l = rBottom.left;
	t = rBottom.top;

	for(int i = 0; i < 4; i++) {
		control2[i].x = control[i].x + l;
		control2[i].y = control[i].y + t;
	}

	pG->polygon(fillColor, control2, 4);
	pG->polyLine(control2, 4);
}

void AP_CocoaLeftRuler::_drawCellMark(UT_Rect *prDrag, bool bUp)
{
	if (!m_pG || !prDrag)
		return;

	UT_sint32 l = prDrag->left;
	UT_sint32 t = prDrag->top;

	UT_sint32 w = m_pG->tdu(prDrag->width);
	UT_sint32 h = m_pG->tdu(prDrag->height);

	GR_Painter painter(m_pG);

	GR_CocoaCairoGraphics * pG = static_cast<GR_CocoaCairoGraphics *>(m_pG);

	UT_Point control[4];

	control[0].x = l + 1;
	control[0].y = t + 1;
	control[1].x = l + 1;
	control[1].y = t + h - 1;
	control[2].x = l + w - 1;
	control[2].y = t + h - 1;
	control[3].x = l + w - 1;
	control[3].y = t + 1;

	UT_RGBColor lineColor;
	GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor knobColor], lineColor);

	UT_RGBColor fillColor;
	if (bUp)
		fillColor = pG->VGrey();
	else
		GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor whiteColor], fillColor);

	pG->polygon(fillColor, control, 4);
	pG->setColor(lineColor);
	pG->polyLine(control, 4);
}
#endif

bool AP_CocoaLeftRuler::_graphicsUpdateCB(NSRect * aRect, GR_CocoaCairoGraphics *pG, void* param)
{
	// a static function
	AP_CocoaLeftRuler * pCocoaLeftRuler = (AP_CocoaLeftRuler *)param;
	if (!pCocoaLeftRuler)
		return false;

	UT_Rect rClip;
	rClip.left   = aRect->origin.x;
	rClip.top    = aRect->origin.y;
	rClip.width  = aRect->size.width;
	rClip.height = aRect->size.height;
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
	// _xap->draw(NULL);
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
	GR_CocoaCairoGraphics* pGr = dynamic_cast<GR_CocoaCairoGraphics*>(_xap->getGraphics());
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
	GR_CocoaCairoGraphics* pGr = dynamic_cast<GR_CocoaCairoGraphics*>(_xap->getGraphics());
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
	GR_CocoaCairoGraphics* pGr = dynamic_cast<GR_CocoaCairoGraphics*>(_xap->getGraphics());
	if (!pGr->_isFlipped()) {
		pt.y = [sender bounds].size.height - pt.y;
	}
	_xap->mouseRelease(ems, emb, (UT_sint32)pGr->tluD(pt.x), (UT_sint32)pGr->tluD(pt.y));
}
@end
