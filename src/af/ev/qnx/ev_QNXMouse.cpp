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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// TODO see if we need to do the GTK absolute-to-relative coordinate
// TODO trick like we did in the top ruler.

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_QNXMouse.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "xav_View.h"
#include <stdio.h>

EV_QNXMouse::EV_QNXMouse(EV_EditEventMapper * pEEM)
	: EV_Mouse(pEEM)
{
	m_clickState = 0;					// no click
	m_contextState = 0;
}

void EV_QNXMouse::mouseUp(AV_View* pView, PtCallbackInfo_t * e)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop;
	EV_EditMouseContext emc = 0;

    PhPointerEvent_t *ptrevent;
	PhRect_t		 *rect;
	int 			 mx, my;
    ptrevent = (PhPointerEvent_t *)PhGetData(e->event);
	rect = PhGetRects(e->event);

	mx = rect->ul.x;
	my = rect->ul.y;

	if (e->event->subtype == Ph_EV_RELEASE_REAL) {
    	UT_DEBUGMSG(("Mouse Real Release! (%d,%d)\n", mx, my));
	}
	else if (e->event->subtype == Ph_EV_RELEASE_PHANTOM) {
    	UT_DEBUGMSG(("Ignoring Mouse Phantom Release! (%d,%d)\n", mx, my));
		return;
	}
	else if (e->event->subtype == Ph_EV_RELEASE_ENDCLICK) {
    	UT_DEBUGMSG(("Ignoring Mouse Endclick Release! (%d,%d)\n", mx, my));
		return;
	}
	else {
		UT_DEBUGMSG(("Ignoring Unknown release type 0x%x (%d,%d)\n",e->event->subtype, mx, my));
		return;
	}

	if (ptrevent->key_mods & Pk_KM_Shift)
		ems |= EV_EMS_SHIFT;
	if (ptrevent->key_mods & Pk_KM_Ctrl)
		ems |= EV_EMS_CONTROL;
	if (ptrevent->key_mods & Pk_KM_Alt)
		ems |= EV_EMS_ALT;

	//PHOTON refers to buttons 1,2,3 as the mouse buttons
	//from right to left (biased against right handers!)
	if (ptrevent->buttons & Ph_BUTTON_3)
		emb = EV_EMB_BUTTON1;
	else if (ptrevent->buttons & Ph_BUTTON_2)
		emb = EV_EMB_BUTTON2;
	else if (ptrevent->buttons & Ph_BUTTON_1)
		emb = EV_EMB_BUTTON3;

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
		invokeMouseMethod(pView,pEM,
						  (UT_sint32)mx, 
						  (UT_sint32)my);
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

void EV_QNXMouse::mouseClick(AV_View* pView, PtCallbackInfo_t * e)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop = 0;
	EV_EditMouseContext emc = 0;

    PhPointerEvent_t *ptrevent;
	PhRect_t		 *rect;
	int 			 mx, my;
    ptrevent = (PhPointerEvent_t *)PhGetData(e->event);
	rect = PhGetRects(e->event);

	mx = rect->ul.x;
	my = rect->ul.y;
    UT_DEBUGMSG(("Mouse Click! (%d,%d)\n", mx, my));

	if (ptrevent->key_mods & Pk_KM_Shift)
		ems |= EV_EMS_SHIFT;
	if (ptrevent->key_mods & Pk_KM_Ctrl)
		ems |= EV_EMS_CONTROL;
	if (ptrevent->key_mods & Pk_KM_Alt)
		ems |= EV_EMS_ALT;

	if (ptrevent->buttons & Ph_BUTTON_3)
		emb = EV_EMB_BUTTON1;
	else if (ptrevent->buttons & Ph_BUTTON_2)
		emb = EV_EMB_BUTTON2;
	else if (ptrevent->buttons & Ph_BUTTON_1)
		emb = EV_EMB_BUTTON3;
	else {
		// TODO decide something better to do here....
		UT_DEBUGMSG(("EV_QNXMouse::mouseClick: unknown button %d\n", ptrevent->buttons));
		return;
	}

	if (ptrevent->click_count == 1)
		mop = EV_EMO_SINGLECLICK;
	else if (ptrevent->click_count == 2)
		mop = EV_EMO_DOUBLECLICK;
	else
	{
		// TODO decide something better to do here....
		UT_DEBUGMSG(("EV_QNXMouse::mouseClick:: unknown type %d\n", ptrevent->click_count));
		return;
	}

	emc = pView->getMouseContext((UT_sint32)mx,
								 (UT_sint32)my);
	
	m_clickState = mop;					// remember which type of click
	m_contextState = emc;				// remember context of click
	
	result = m_pEEM->Mouse(emc|mop|emb|ems, &pEM);
	
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		printf("Invoke Mouse Method \n");
		invokeMouseMethod(pView,pEM,
						  (UT_sint32)mx, 
						  (UT_sint32)my);
		printf("Finished Mouse Method \n");
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

void EV_QNXMouse::mouseMotion(AV_View* pView, PtCallbackInfo_t *e)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop;
	EV_EditMouseContext emc = 0;

    PhPointerEvent_t *ptrevent;
	PhRect_t		 *rect;
	int 			 mx, my;
    ptrevent = (PhPointerEvent_t *)PhGetData(e->event);
	rect = PhGetRects(e->event);

	mx = rect->ul.x;
	my = rect->ul.y;
    UT_DEBUGMSG(("Mouse Move! (%d,%d)\n", mx, my));

	if (ptrevent->key_mods & Pk_KM_Shift)
		ems |= EV_EMS_SHIFT;
	if (ptrevent->key_mods & Pk_KM_Ctrl)
		ems |= EV_EMS_CONTROL;
	if (ptrevent->key_mods & Pk_KM_Alt)
		ems |= EV_EMS_ALT;

	if (ptrevent->buttons & Ph_BUTTON_3)
		emb = EV_EMB_BUTTON1;
	else if (ptrevent->buttons & Ph_BUTTON_2)
		emb = EV_EMB_BUTTON2;
	else if (ptrevent->buttons & Ph_BUTTON_1)
		emb = EV_EMB_BUTTON3;
	else
		emb = EV_EMB_BUTTON0;

	// TODO confirm that we report movements under the
	// TODO mouse button that we did the capture on.

	if (m_clickState == 0)
	{
		mop = EV_EMO_DRAG;
		emc = pView->getMouseContext((UT_sint32)mx,
									 (UT_sint32)my);
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
		invokeMouseMethod(pView,pEM,
						  (UT_sint32)mx, 
						  (UT_sint32)my);
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
