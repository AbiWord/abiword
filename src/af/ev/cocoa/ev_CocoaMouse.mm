/* AbiSource Program Utilities
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

#include <Cocoa/Cocoa.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_CocoaMouse.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "xav_View.h"

EV_CocoaMouse::EV_CocoaMouse(EV_EditEventMapper * pEEM)
	: EV_Mouse(pEEM)
{
	m_clickState = 0;					// no click
	m_contextState = 0;
}

void EV_CocoaMouse::mouseUp(AV_View* pView, NSEvent* e, NSView* hitView)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop;
	EV_EditMouseContext emc = 0;
	
	UT_DEBUGMSG (("Received mouse up...\n"));
	
	ems = _convertModifierState ([e modifierFlags]);
	emb = _convertMouseButton ([e buttonNumber]);

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
			invokeMouseMethod(pView, pEM, (UT_sint32) pt.x, (UT_sint32) pt.y);
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
	
	xxx_UT_DEBUGMSG (("Received mouse click...\n"));
	state = _convertModifierState ([e modifierFlags]);
	emb = _convertMouseButton ([e buttonNumber]);

	NSEventType evtType = [e type];
	switch (evtType) {
	case NSLeftMouseDown:
	case NSRightMouseDown:
	case NSOtherMouseDown:
		mop = EV_EMO_SINGLECLICK;
		//detect double clicks
		// TODO
		//mop = EV_EMO_DOUBLECLICK;
		break;
	default:
		// TODO decide something better to do here....
		return;
	}

	pt = [e locationInWindow];
	pt = [hitView convertPoint:pt fromView:nil];
	pt.y = [hitView bounds].size.width - pt.y;
	emc = pView->getMouseContext((UT_sint32)pt.x, (UT_sint32)pt.y);
	
	m_clickState = mop;					// remember which type of click
	m_contextState = emc;				// remember context of click
	
	result = m_pEEM->Mouse(emc|mop|emb|state, &pEM);
	
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,(UT_sint32)pt.x, (UT_sint32)pt.y);
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
	
	xxx_UT_DEBUGMSG (("Received mouse motion...\n"));
	ems = _convertModifierState ([e modifierFlags]);
	emb = _convertMouseButton ([e buttonNumber]);

	// TODO confirm that we report movements under the
	// TODO mouse button that we did the capture on.
	pt = [e locationInWindow];
	pt = [hitView convertPoint:pt fromView:nil];

	if (m_clickState == 0)
	{
		mop = EV_EMO_DRAG;
		emc = pView->getMouseContext((UT_sint32)pt.x,(UT_sint32)pt.y);
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
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,(UT_sint32)pt.x,(UT_sint32)pt.y);
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


EV_EditMouseButton EV_CocoaMouse::_convertMouseButton(int btn)
{
	EV_EditMouseButton emb = 0;
	switch (btn) {
	case 0:
		emb = EV_EMB_BUTTON1; // left
		break;
	case 1:
		emb = EV_EMB_BUTTON2; // right 
		break;
	case 2:
		emb = EV_EMB_BUTTON3; // middle
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


EV_EditModifierState EV_CocoaMouse::_convertModifierState(unsigned int modifiers)
{
	EV_EditModifierState ems = 0;
	if (modifiers & NSShiftKeyMask)
		ems |= EV_EMS_SHIFT;
	if (modifiers & NSControlKeyMask)
		ems |= EV_EMS_CONTROL;
	if (modifiers & NSAlternateKeyMask)
		ems |= EV_EMS_ALT;
	return ems;
}
