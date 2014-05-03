/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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

// TODO see if we need to do the GTK absolute-to-relative coordinate
// TODO trick like we did in the top ruler.

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ut_units.h"
#include "ev_Mouse.h"
#include "ev_UnixMouse.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "xav_View.h"
#include "gr_Graphics.h"

EV_UnixMouse::EV_UnixMouse(EV_EditEventMapper * pEEM)
	: EV_Mouse(pEEM)
{
}

void EV_UnixMouse::mouseUp(AV_View* pView, GdkEventButton* e)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop;
	EV_EditMouseContext emc = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (e->state & GDK_BUTTON1_MASK)
		emb = EV_EMB_BUTTON1;
	else if (e->state & GDK_BUTTON2_MASK)
		emb = EV_EMB_BUTTON2;
	else if (e->state & GDK_BUTTON3_MASK)
		emb = EV_EMB_BUTTON3;
	// these are often used for X scrolling mice, 4 is down, 5 is up
	else if (e->state & GDK_BUTTON4_MASK)
		emb = EV_EMB_BUTTON4;
	else if (e->state & GDK_BUTTON5_MASK)
		emb = EV_EMB_BUTTON5;
	else
	{
		// TODO decide something better to do here....
		UT_DEBUGMSG(("EV_UnixMouse::mouseUp: unknown button %d\n",e->button));
		return;
	}

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
		UT_ASSERT(pEM);
		invokeMouseMethod(pView, pEM, static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)), static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
		signal(emc|mop|emb|ems, static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)), static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
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

void EV_UnixMouse::mouseClick(AV_View* pView, GdkEventButton* e)
{
	EV_EditMethod * pEM;
	EV_EditModifierState state = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop = 0;
	EV_EditMouseContext emc = 0;
#if GTK_CHECK_VERSION(3,0,0)
	GdkDevice *device;

	device = gdk_event_get_source_device((GdkEvent *) e);
#endif
	if (e->button == 1)
		emb = EV_EMB_BUTTON1;
	else if (e->button == 2)
		emb = EV_EMB_BUTTON2;
	else if (e->button == 3)
		emb = EV_EMB_BUTTON3;
	// these are often used for X scrolling mice, 4 is down, 5 is up
	else if (e->button == 4)
		emb = EV_EMB_BUTTON4;
	else if (e->button == 5)
	        emb = EV_EMB_BUTTON5;
	else
	{
		// TODO decide something better to do here....
		UT_DEBUGMSG(("EV_UnixMouse::mouseClick: unknown button %d\n",e->button));
		return;
	}
	
	if (e->state & GDK_SHIFT_MASK)
		state |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		state |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		state |= EV_EMS_ALT;

	if (e->type == GDK_BUTTON_PRESS)
		mop = EV_EMO_SINGLECLICK;
	else if (e->type == GDK_2BUTTON_PRESS)
		mop = EV_EMO_DOUBLECLICK;
	else
	{
		// TODO decide something better to do here....
		return;
	}

	emc = pView->getMouseContext(static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)),static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
	
	m_clickState = mop;					// remember which type of click
	m_contextState = emc;				// remember context of click
	
	result = m_pEEM->Mouse(emc|mop|emb|state, &pEM);
	
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)),static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
		signal(emc|mop|emb|state, static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)),static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));

#if GTK_CHECK_VERSION(3,0,0)
		if (gdk_device_get_source (device) == GDK_SOURCE_TOUCHSCREEN || getenv ("ABI_TEST_TOUCH")) {
			pView->setVisualSelectionEnabled(true);
		} else
#endif
		{
			pView->setVisualSelectionEnabled(false);
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

void EV_UnixMouse::mouseMotion(AV_View* pView, GdkEventMotion *e)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop;
	EV_EditMouseContext emc = 0;

	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (e->state & GDK_BUTTON1_MASK)
		emb = EV_EMB_BUTTON1;
	else if (e->state & GDK_BUTTON2_MASK)
		emb = EV_EMB_BUTTON2;
	else if (e->state & GDK_BUTTON3_MASK)
		emb = EV_EMB_BUTTON3;
	else
		emb = EV_EMB_BUTTON0;

	// TODO confirm that we report movements under the
	// TODO mouse button that we did the capture on.

	if (m_clickState == 0)
	{
		mop = EV_EMO_DRAG;
		emc = pView->getMouseContext(static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)),static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
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
		emc = m_contextState;
		return;
	}

	result = m_pEEM->Mouse(emc|mop|emb|ems, &pEM);
	
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)),static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
		signal(emc|mop|emb|ems, static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)),static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
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

void EV_UnixMouse::mouseScroll(AV_View* pView, GdkEventScroll *e)
{
	EV_EditMethod * pEM;
	EV_EditModifierState state = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop = 0;
	EV_EditMouseContext emc = 0;

	// map the scrolling type generated from mouse buttons 4 to 7 onto mousebuttons
	if (e->direction == GDK_SCROLL_UP)
	{
	  emb = EV_EMB_BUTTON4;
	  xxx_UT_DEBUGMSG(("Scroll up detected \n"));
	}
	else if (e->direction == GDK_SCROLL_DOWN)
	{
	  emb = EV_EMB_BUTTON5;
	  xxx_UT_DEBUGMSG(("Scroll down detected \n"));
	}
	/*else if (e->direction == GDK_SCROLL_LEFT) // we don't handle buttons 6 and 7
	        emb = EV_EMB_BUTTON6;
	else if (e->direction == GDK_SCROLL_RIGHT)
	        emb = EV_EMB_BUTTON7;*/
	else
	{
		// TODO decide something better to do here....
		UT_DEBUGMSG(("EV_UnixMouse::mouseScroll: unhandled scroll action\n"));
		return;
	}

	if (e->state & GDK_SHIFT_MASK)
		state |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		state |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		state |= EV_EMS_ALT;

	// map the scrolling event onto a single mouse click
	if (e->type == GDK_SCROLL)
		mop = EV_EMO_SINGLECLICK;
	else
	{
		// TODO this shouldn't really happen at all
	}

	emc = pView->getMouseContext(static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)),static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
	
	m_clickState = 0;					// do NOT remember which type of click, see #13635
	m_contextState = emc;				// remember context of click
	
	result = m_pEEM->Mouse(emc|mop|emb|state, &pEM);
	
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)),static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
		signal(emc|mop|emb|state, static_cast<UT_sint32>(pView->getGraphics()->tluD(e->x)),static_cast<UT_sint32>(pView->getGraphics()->tluD(e->y)));
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
