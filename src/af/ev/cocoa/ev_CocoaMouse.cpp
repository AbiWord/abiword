/* AbiSource Program Utilities
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

#include <Cocoa/Cocoa.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_CocoaMouse.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "gr_CocoaCairoGraphics.h"
#include "xav_View.h"

EV_CocoaMouse::EV_CocoaMouse(EV_EditEventMapper * pEEM)
	: EV_Mouse(pEEM)
{
}

void EV_CocoaMouse::mouseUp(AV_View* pView, NSEvent* e, NSView* hitView)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop;
	EV_EditMouseContext emc = 0;
	bool rightBtn = false;
	
	xxx_UT_DEBUGMSG (("Received mouse up...\n"));
	
	ems = _convertModifierState ([e modifierFlags], rightBtn);
	emb = _convertMouseButton ([e buttonNumber], rightBtn);

	// TODO confirm that we report release under the
	// TODO mouse button that we did the capture on.

	mop = EV_EMO_RELEASE;
	if (m_clickState == EV_EMO_DOUBLECLICK)
		mop = EV_EMO_DOUBLERELEASE;
	m_clickState = 0;

	emc = m_contextState;
	
	result = m_pEEM->Mouse(emc|mop|emb|ems, &pEM);

	switch (result)
	{
	case EV_EEMR_COMPLETE:
		{
			UT_ASSERT(pEM);
			NSPoint pt = [e locationInWindow];
			pt = [hitView convertPoint:pt fromView:nil];
			GR_CocoaCairoGraphics* pG = dynamic_cast<GR_CocoaCairoGraphics*>(pView->getGraphics());
			if (!pG->_isFlipped()) {
				pt.y = [hitView bounds].size.height - pt.y;
			}
			invokeMouseMethod(pView, pEM, static_cast<UT_sint32>(pG->tluD(pt.x)), 
										static_cast<UT_sint32>(pG->tluD(pt.y)));
		}
		return;
	case EV_EEMR_INCOMPLETE:
		// I'm not sure this makes any sense, but we allow it.
		return;
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:
		// TODO What to do ?? Should we beep at them or just be quiet ??
		return;
	default:
		UT_ASSERT(0);
		return;
	}
}

void EV_CocoaMouse::mouseClick(AV_View* pView, NSEvent* e, NSView *hitView)
{
	EV_EditMethod * pEM;
	EV_EditModifierState state = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop = 0;
	EV_EditMouseContext emc = 0;
	NSPoint pt;
	bool rightBtn = false;
	
	xxx_UT_DEBUGMSG (("Received mouse click...\n"));
	state = _convertModifierState ([e modifierFlags], rightBtn);
	emb = _convertMouseButton ([e buttonNumber], rightBtn);

	NSEventType evtType = [e type];
	switch (evtType) {
	case NSLeftMouseDown:
	case NSRightMouseDown:
	case NSOtherMouseDown:
		switch ([e clickCount]) {
		case 1:
			mop = EV_EMO_SINGLECLICK;
			break;
		default:	/* in case of triple click */
			mop = EV_EMO_DOUBLECLICK;
			break;
		}
		break;
	default:
		// TODO decide something better to do here....
		return;
	}

	pt = [e locationInWindow];
	pt = [hitView convertPoint:pt fromView:nil];
	GR_CocoaCairoGraphics* pG = dynamic_cast<GR_CocoaCairoGraphics*>(pView->getGraphics());
	if (!pG->_isFlipped()) {
		pt.y = [hitView bounds].size.height - pt.y;
	}
	UT_DEBUGMSG(("Mouse click at x=%f y=%f\n", pt.x, pt.y));
	emc = pView->getMouseContext(static_cast<UT_sint32>(pG->tluD(pt.x)), 
										static_cast<UT_sint32>(pG->tluD(pt.y)));
	
	m_clickState = mop;					// remember which type of click
	m_contextState = emc;				// remember context of click
	
	result = m_pEEM->Mouse(emc|mop|emb|state, &pEM);
	
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		{
			UT_ASSERT(pEM);
			invokeMouseMethod(pView, pEM, static_cast<UT_sint32>(pG->tluD(pt.x)), 
										static_cast<UT_sint32>(pG->tluD(pt.y)));
		}
		return;
	case EV_EEMR_INCOMPLETE:
		// I'm not sure this makes any sense, but we allow it.
		return;
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:
		// TODO What to do ?? Should we beep at them or just be quiet ??
		return;
	default:
		UT_ASSERT(0);
		return;
	}
}


void EV_CocoaMouse::mouseMotion(AV_View* pView, NSEvent *e, NSView *hitView)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop;
	EV_EditMouseContext emc = 0;
	NSPoint pt;
	bool rightBtn = false;
	
	xxx_UT_DEBUGMSG (("Received mouse motion...\n"));
	ems = _convertModifierState ([e modifierFlags], rightBtn);
	emb = _convertMouseButton ([e buttonNumber], rightBtn);

	// TODO confirm that we report movements under the
	// TODO mouse button that we did the capture on.
	pt = [e locationInWindow];
	pt = [hitView convertPoint:pt fromView:nil];
	GR_CocoaCairoGraphics* pG = dynamic_cast<GR_CocoaCairoGraphics*>(pView->getGraphics());
	if (!pG->_isFlipped()) {
		pt.y = [hitView bounds].size.height - pt.y;
	}

	if (m_clickState == 0)
	{
		mop = EV_EMO_DRAG;
		emc = pView->getMouseContext(static_cast<UT_sint32>(pG->tluD(pt.x)), 
										static_cast<UT_sint32>(pG->tluD(pt.y)));
	}
	else if (m_clickState == EV_EMO_SINGLECLICK)
	{
		mop = EV_EMO_DRAG;
		emc = m_contextState;
	}
	else if (m_clickState == EV_EMO_DOUBLECLICK)
	{
		mop = EV_EMO_DOUBLEDRAG;
		emc = m_contextState;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	result = m_pEEM->Mouse(emc|mop|emb|ems, &pEM);
	
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		{
			UT_ASSERT(pEM);
			invokeMouseMethod(pView, pEM, static_cast<UT_sint32>(pG->tluD(pt.x)), 
										static_cast<UT_sint32>(pG->tluD(pt.y)));
		}
		return;
	case EV_EEMR_INCOMPLETE:
		// I'm not sure this makes any sense, but we allow it.
		return;
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:
		// TODO What to do ?? Should we beep at them or just be quiet ??
		return;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
}


EV_EditMouseButton EV_CocoaMouse::_convertMouseButton(int btn, bool rightBtn)
{
	EV_EditMouseButton emb = 0;
	switch (btn) {
	case 0:
		if (rightBtn) {
			emb = EV_EMB_BUTTON3; // right 
		}
		else {
			emb = EV_EMB_BUTTON1; // left
		}
		break;
	case 1:
		emb = EV_EMB_BUTTON3; // right
		break;
	// mac mouse with scroll wheel and 2 buttons, numbered 0 and 1 - don't know about other mice

	case 2:
		emb = EV_EMB_BUTTON2; // middle 
		break;
	// these are often used for X scrolling mice, 4 is down, 5 is up
	case 3:
		emb = EV_EMB_BUTTON4; // scroll down
		break;
	case 4:
		emb = EV_EMB_BUTTON5; // scroll up
		break;
	default:
		// TODO decide something better to do here....
		UT_DEBUGMSG(("EV_CocoaMouse::_convertMouseButton: unknown button %d\n", btn));
		return 0xffffffff;
	}
	return emb;
}


EV_EditModifierState EV_CocoaMouse::_convertModifierState(unsigned int modifiers, bool &rightBtn)
{
	EV_EditModifierState ems = 0;
	if (modifiers & NSShiftKeyMask)
		ems |= EV_EMS_SHIFT;
	if (modifiers & NSCommandKeyMask)
		ems |= EV_EMS_ALT;
	if (modifiers & NSAlternateKeyMask)
		ems |= EV_EMS_CONTROL;
	if (modifiers & NSControlKeyMask)
		rightBtn = true;
	return ems;
}
