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

#include "gr_CocoaCairoGraphics.h"
#include "gr_Painter.h"

#include "ev_CocoaMouse.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_CocoaFrame.h"

#include "ap_CocoaFrame.h"
#include "ap_CocoaFrameImpl.h"
#include "ap_CocoaTopRuler.h"
#include "ap_FrameData.h"

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
	: AP_TopRuler(pFrame),
		m_wTopRuler(nil),
		m_rootWindow(nil),
		m_delegate(nil)
{
	m_wTopRuler = [(AP_CocoaFrameController *)(static_cast<XAP_CocoaFrameImpl *>(m_pFrame->getFrameImpl())->_getController()) getHRuler];
}

AP_CocoaTopRuler::~AP_CocoaTopRuler(void)
{
	static_cast<GR_CocoaCairoGraphics *>(m_pG)->_setUpdateCallback (NULL, NULL);
	DELETEP(m_pG);
	if (m_delegate) {
		[[NSNotificationCenter defaultCenter] removeObserver:m_delegate];
		[m_wTopRuler setEventDelegate:nil];
		[m_delegate release];
	}
}

void AP_CocoaTopRuler::setView(AV_View * pView)
{
	AP_TopRuler::setView(pView);

	if (m_delegate)
		return;

	DELETEP(m_pG);

	GR_CocoaCairoAllocInfo ai(m_wTopRuler);
	GR_CocoaCairoGraphics * pG = (GR_CocoaCairoGraphics *) XAP_App::getApp()->newGraphics(ai);
	UT_ASSERT(pG);
	m_pG = pG;

	m_delegate = [[AP_CocoaTopRulerDelegate alloc] init];
	[m_wTopRuler setEventDelegate:m_delegate];
	[m_delegate setXAPOwner:this];
	[[NSNotificationCenter defaultCenter] addObserver:m_delegate
			selector:@selector(viewDidResize:) 
			name:NSViewFrameDidChangeNotification object:m_wTopRuler];

	pG->_setUpdateCallback(&_graphicsUpdateCB, (void *)this);
	pG->setGrabCursor(GR_Graphics::GR_CURSOR_LEFTRIGHT);

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

#if 0
void AP_CocoaTopRuler::_drawMarginProperties(const UT_Rect * /*pClipRect*/, AP_TopRulerInfo * pInfo, GR_Graphics::GR_Color3D /*clr*/)
{
	if (!m_pG)
		return;

	UT_Rect rLeft;
	UT_Rect rRight;

	_getMarginMarkerRects(pInfo, rLeft, rRight);

	GR_Painter painter(m_pG);

	GR_CocoaCairoGraphics * pG = static_cast<GR_CocoaCairoGraphics *>(m_pG);

	UT_sint32 l = rLeft.left;
	UT_sint32 t = rLeft.top;

	UT_Point control[4];

	control[0].x = l + 0;
	control[0].y = t + 0;
	control[1].x = l + 0;
	control[1].y = t + 6;
	control[2].x = l + 6;
	control[2].y = t + 6;
	control[3].x = l + 6;
	control[3].y = t + 0;

	UT_RGBColor lineColor;
	GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor knobColor], lineColor);

	UT_RGBColor fillColor;
	fillColor = pG->HBlue();

	pG->polygon(fillColor, control, 4);
	pG->setColor(lineColor);
	pG->polyLine(control, 4);

	l = rRight.left;
	t = rRight.top;

	pG->polygon(fillColor, control, 4);
	pG->polyLine(control, 4);
}

void AP_CocoaTopRuler::_drawLeftIndentMarker(UT_Rect & rect, bool bFilled)
{
	if (!m_pG)
		return;

	UT_sint32 l = rect.left;
	UT_sint32 t = rect.top;

	fl_BlockLayout * pBlock = (static_cast<FV_View *>(m_pView))->getCurrentBlock();
	
	bool bRTL = pBlock ? (pBlock->getDominantDirection() == UT_BIDI_RTL) : false;

	GR_Painter painter(m_pG);

	GR_CocoaCairoGraphics * pG = static_cast<GR_CocoaCairoGraphics *>(m_pG);

	UT_Point control[5];

	control[0].x = l + 10;
	control[0].y = t + 8;
	control[1].x = l + 10;
	control[1].y = t + 5;
	control[2].x = l + 5;
	control[2].y = t + 0;
	control[3].x = l + 0;
	control[3].y = t + 5;
	control[4].x = l + 0;
	control[4].y = t + 8;

	UT_RGBColor lineColor;
	GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor knobColor], lineColor);

	UT_RGBColor fillColor;

        if (bFilled)
                fillColor = pG->HBlue();
        else
                GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor whiteColor], fillColor);

	pG->polygon(fillColor, control, 5);
	pG->setColor(lineColor);
	pG->polyLine(control, 5);

	if (!bRTL)
	{
		control[0].x = l + 10;
		control[0].y = t + 9;
		control[1].x = l + 0;
		control[1].y = t + 9;
		control[2].x = l + 0;
		control[2].y = t + 14;
		control[3].x = l + 10;
		control[3].y = t + 14;

		pG->polygon(fillColor, control, 4);
		pG->setColor(lineColor);
		pG->polyLine(control, 4);
	}
}

void AP_CocoaTopRuler::_drawRightIndentMarker(UT_Rect & rect, bool bFilled)
{
	if (!m_pG)
		return;

	UT_sint32 l = rect.left;
	UT_sint32 t = rect.top;

	fl_BlockLayout * pBlock = (static_cast<FV_View *>(m_pView))->getCurrentBlock();
	
	bool bRTL = pBlock ? (pBlock->getDominantDirection() == UT_BIDI_RTL) : false;

	GR_Painter painter(m_pG);

	GR_CocoaCairoGraphics * pG = static_cast<GR_CocoaCairoGraphics *>(m_pG);

	UT_Point control[5];

	control[0].x = l + 10;
	control[0].y = t + 8;
	control[1].x = l + 10;
	control[1].y = t + 5;
	control[2].x = l + 5;
	control[2].y = t + 0;
	control[3].x = l + 0;
	control[3].y = t + 5;
	control[4].x = l + 0;
	control[4].y = t + 8;

	UT_RGBColor lineColor;
	GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor knobColor], lineColor);

	UT_RGBColor fillColor;

	if (bFilled)
		fillColor = pG->HBlue();
	else
		GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor whiteColor], fillColor);

	pG->polygon(fillColor, control, 5);
	pG->setColor(lineColor);
	pG->polyLine(control, 5);

	if (bRTL)
	{
		control[0].x = l + 10;
		control[0].y = t + 9;
		control[1].x = l + 0;
		control[1].y = t + 9;
		control[2].x = l + 0;
		control[2].y = t + 14;
		control[3].x = l + 10;
		control[3].y = t + 14;

		pG->polygon(fillColor, control, 4);
		pG->setColor(lineColor);
		pG->polyLine(control, 4);
	}
}

void AP_CocoaTopRuler::_drawFirstLineIndentMarker(UT_Rect & rect, bool bFilled)
{
	if (!m_pG)
		return;

	UT_sint32 l = rect.left;
	UT_sint32 t = rect.top;

	GR_Painter painter(m_pG);

	GR_CocoaCairoGraphics * pG = static_cast<GR_CocoaCairoGraphics *>(m_pG);

	UT_Point control[5];

	control[0].x = l + 0;
	control[0].y = t + 0;
	control[1].x = l + 0;
	control[1].y = t + 3;
	control[2].x = l + 5;
	control[2].y = t + 8;
	control[3].x = l + 10;
	control[3].y = t + 3;
	control[4].x = l + 10;
	control[4].y = t + 0;

	UT_RGBColor lineColor;
	GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor knobColor], lineColor);

	UT_RGBColor fillColor;

	if (bFilled)
		fillColor = pG->HBlue();
	else
		GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor whiteColor], fillColor);

	pG->polygon(fillColor, control, 5);
	pG->setColor(lineColor);
	pG->polyLine(control, 5);
}

void AP_CocoaTopRuler::_drawColumnGapMarker(UT_Rect & rect)
{
	if (!m_pG)
		return;

	UT_sint32 l = rect.left;
	UT_sint32 t = rect.top;

	UT_sint32 w = m_pG->tdu(rect.width);

	GR_Painter painter(m_pG);

	GR_CocoaCairoGraphics * pG = static_cast<GR_CocoaCairoGraphics *>(m_pG);

	UT_Point control[6];

	control[0].x = l + w - 1;
	control[0].y = t + 0;
	control[1].x = l + 1;
	control[1].y = t + 0;
	control[2].x = l + 1;
	control[2].y = t + 6;
	control[3].x = l + 4;
	control[3].y = t + 3;
	control[4].x = l + w - 4;
	control[4].y = t + 3;
	control[5].x = l + w - 1;
	control[5].y = t + 6;

	UT_RGBColor lineColor;
	GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor knobColor], lineColor);
	UT_RGBColor fillColor;
	fillColor = pG->HBlue();

	pG->polygon(fillColor, control, 6);
	pG->setColor(lineColor);
	pG->polyLine(control, 6);
}

void AP_CocoaTopRuler::_drawCellMark(UT_Rect * prDrag, bool bUp)
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
		fillColor = pG->HGrey();
	else
		GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor whiteColor], fillColor);

	pG->polygon(fillColor, control, 4);
	pG->setColor(lineColor);
	pG->polyLine(control, 4);
}
#endif

bool AP_CocoaTopRuler::_graphicsUpdateCB(NSRect * aRect, GR_CocoaCairoGraphics *pG, void* param)
{
	// a static function
	AP_CocoaTopRuler * pCocoaTopRuler = (AP_CocoaTopRuler *)param;
	if (!pCocoaTopRuler)
		return false;

	UT_Rect rClip;
	rClip.left   = aRect->origin.x;
	rClip.top    = aRect->origin.y;
	rClip.width  = aRect->size.width;
	rClip.height = aRect->size.height;
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
	_xap->mouseMotion(ems, (UT_sint32)pGr->tluD(pt.x), (UT_sint32)pGr->tluD(pt.y));
	_xap->isMouseOverTab((UT_uint32)pGr->tluD(pt.x),(UT_uint32)pGr->tluD(pt.y));
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
