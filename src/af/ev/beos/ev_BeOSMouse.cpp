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
 


#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_BeOSMouse.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "xav_View.h"

#include <MessageFilter.h>
//uint32		modifiers();

class MouseFilter: public BMessageFilter {
	public:
		MouseFilter(XAP_BeOSApp * pBeOSApp, XAP_BeOSFrame * pBeOSFrame, 
				    EV_Mouse *pEVMouse);
		filter_result Filter(BMessage *message, BHandler **target);
	private:
		XAP_BeOSApp 	*m_pBeOSApp;
		XAP_BeOSFrame 	*m_pBeOSFrame;
		EV_Mouse	*m_pEVMouse;
};
		
MouseFilter::MouseFilter(XAP_BeOSApp * pBeOSApp, XAP_BeOSFrame * pBeOSFrame, 
				       EV_Mouse *pEVMouse)
          : BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE) {
	m_pBeOSApp = pBeOSApp;
	m_pBeOSFrame = pBeOSFrame;
	m_pEVMouse = pEVMouse;
}					   

filter_result MouseFilter::Filter(BMessage *message, BHandler **target) { 
	//if (message->what != B_KEY_DOWN && message->what != B_KEY_UP) {
	switch(message->what) {
	case B_MOUSE_DOWN:
		((ev_BeOSMouse*)m_pEVMouse)->mouseClick(m_pBeOSFrame->getCurrentView(), message);
		break;
	case B_MOUSE_UP:
		((ev_BeOSMouse*)m_pEVMouse)->mouseUp(m_pBeOSFrame->getCurrentView(), message);
		break;
	case B_MOUSE_MOVED:
		((ev_BeOSMouse*)m_pEVMouse)->mouseMotion(m_pBeOSFrame->getCurrentView(), message);
		break;
	default:
		return(B_DISPATCH_MESSAGE);
	}
	//pView->draw();
	return(B_SKIP_MESSAGE);			
}
/**************************************************************/


ev_BeOSMouse::ev_BeOSMouse(EV_EditEventMapper * pEEM) : EV_Mouse(pEEM)
{
	; //Do nothing yet ...
}

UT_Bool ev_BeOSMouse::synthesize(XAP_BeOSApp * pBeOSApp, 
				 XAP_BeOSFrame * pBeOSFrame) {
	UT_ASSERT(pBeOSFrame); 

	be_Window *pBWin = (be_Window *)pBeOSFrame->getTopLevelWindow();
	UT_ASSERT(pBWin);

	pBWin->Lock();
	pBWin->m_pbe_DocView->AddFilter(new MouseFilter(pBeOSApp, pBeOSFrame, this));
	pBWin->Unlock();
	return UT_TRUE;
}


void ev_BeOSMouse::mouseUp(AV_View* pView, BMessage *msg)
{
	return;

	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop;
        EV_EditMouseContext emc = 0; 

	BPoint 	pt;
	int32	clicks, mod, buttons;
	msg->FindInt32("clicks", &clicks);
	msg->FindInt32("buttons", &buttons);
	msg->FindInt32("modifiers", &mod);
	msg->FindPoint("where", &pt);

	//printf("MOUSE: Up \n");

	if (mod & B_SHIFT_KEY)
		ems |= EV_EMS_SHIFT;
	if (mod & B_SHIFT_KEY)
		ems |= EV_EMS_CONTROL;
	if (mod & B_OPTION_KEY)
		ems |= EV_EMS_ALT;

	if (buttons & B_PRIMARY_MOUSE_BUTTON)
		emb = EV_EMB_BUTTON1;
	else if (buttons & B_PRIMARY_MOUSE_BUTTON)
		emb = EV_EMB_BUTTON2;
	else if (buttons & B_PRIMARY_MOUSE_BUTTON)
		emb = EV_EMB_BUTTON3;
	
	//This seems to only crash when I do this detection ...
	mop = EV_EMO_RELEASE;
        if (m_clickState == EV_EMO_DOUBLECLICK)
                mop = EV_EMO_DOUBLERELEASE;
	m_clickState = 0;
	emc = m_contextState;

	// report movements under the mouse button that we did the capture on
	UT_DEBUGMSG(("onButtonMove: %p [b=%d m=%d]\n",EV_EMO_DRAG|ems, emb, ems));
	
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
		UT_ASSERT(0);
		return;
	}
}

void ev_BeOSMouse::mouseClick(AV_View* pView, BMessage *msg)
{
	EV_EditMethod * pEM;
	EV_EditModifierState state = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop = 0;
        EV_EditMouseContext emc = 0;

	BPoint 	pt;
	int32	clicks, mod, buttons;
	msg->FindInt32("clicks", &clicks);
	msg->FindInt32("buttons", &buttons);
	msg->FindInt32("modifiers", &mod);
	msg->FindPoint("where", &pt);

	//printf("MOUSE: Click \n");

	if (buttons & B_PRIMARY_MOUSE_BUTTON)
		emb = EV_EMB_BUTTON1;
	else if (buttons & B_PRIMARY_MOUSE_BUTTON)
		emb = EV_EMB_BUTTON2;
	else if (buttons & B_PRIMARY_MOUSE_BUTTON)
		emb = EV_EMB_BUTTON3;

	if (mod & B_SHIFT_KEY)
		state |= EV_EMS_SHIFT;
	if (mod & B_SHIFT_KEY)
		state |= EV_EMS_CONTROL;
	if (mod & B_OPTION_KEY)
		state |= EV_EMS_ALT;

	if (clicks == 1)
		mop = EV_EMO_SINGLECLICK;
	else if (clicks == 2)
		mop = EV_EMO_DOUBLECLICK;
/*
	else if (msg->what == B_MOUSE_UP)
		mop = EV_EMO_RELEASE;
*/
	
	emc = pView->getMouseContext((UT_sint32)pt.x,(UT_sint32)pt.y);
	m_clickState = mop;		//Remember the click type
	m_contextState = emc;

	result = m_pEEM->Mouse(emc|mop|emb|state, &pEM);
	
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
		UT_ASSERT(0);
		return;
	}
}

void ev_BeOSMouse::mouseMotion(AV_View* pView, BMessage *msg)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems = 0;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
        EV_EditMouseOp mop;
        EV_EditMouseContext emc = 0;
	
	BPoint 	pt;
	int32	clicks, mod, buttons;
	msg->FindInt32("clicks", &clicks);
	msg->FindInt32("buttons", &buttons);
	msg->FindInt32("modifiers", &mod);
	msg->FindPoint("where", &pt);

	//printf("MOUSE: Move \n");
	
	if (buttons & B_PRIMARY_MOUSE_BUTTON)
		emb = EV_EMB_BUTTON1;
	else if (buttons & B_PRIMARY_MOUSE_BUTTON)
		emb = EV_EMB_BUTTON2;
	else if (buttons & B_PRIMARY_MOUSE_BUTTON)
		emb = EV_EMB_BUTTON3;
	else {
		//Bail because I don't think  the code expect non button events
		return;
	}
		
	if (mod & B_SHIFT_KEY)
		ems |= EV_EMS_SHIFT;
	if (mod & B_SHIFT_KEY)
		ems |= EV_EMS_CONTROL;
	if (mod & B_OPTION_KEY)
		ems |= EV_EMS_ALT;

	// report movements under the mouse button that we did the capture on
	if (m_clickState == 0) {
                mop = EV_EMO_DRAG;
                emc = pView->getMouseContext((UT_sint32)pt.x,(UT_sint32)pt.y);
        }
        else if (m_clickState == EV_EMO_SINGLECLICK) {
                mop = EV_EMO_DRAG;
                emc = m_contextState;
        }
        else if (m_clickState == EV_EMO_DOUBLECLICK) {
                mop = EV_EMO_DOUBLEDRAG;
                emc = m_contextState;
        }
        else {
                UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
                return;
        }

                                                       
	UT_DEBUGMSG(("onButtonMove: %p [b=%d m=%d]\n",EV_EMO_DRAG|ems, emb, ems));
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

