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
 
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_EditMethod.h"
#include "ev_NamedVirtualKey.h"
#include "ev_BeOSKeyboard.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#define DPRINTF(x)

EV_EditBits s_mapVirtualKeyCodeToNVK(int keyval, int modifiers, int charval);

/**************************************************************/
/*
 EVENT FILTERING
 The user hit some key ... do the right thing
*/
class KeybdFilter: public BMessageFilter {
	public:
		KeybdFilter(XAP_BeOSApp * pBeOSApp, XAP_BeOSFrame * pBeOSFrame, 
			    EV_Keyboard *pEVKeyboard);
		filter_result Filter(BMessage *message, BHandler **target);
	private:
		XAP_BeOSApp 	*m_pBeOSApp;
		XAP_BeOSFrame 	*m_pBeOSFrame;
		EV_Keyboard	*m_pEVKeyboard;
};
		
KeybdFilter::KeybdFilter(XAP_BeOSApp * pBeOSApp, XAP_BeOSFrame * pBeOSFrame, 
			 EV_Keyboard *pEVKeyboard)
          : BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE) {
	m_pBeOSApp = pBeOSApp;
	m_pBeOSFrame = pBeOSFrame;
	m_pEVKeyboard = pEVKeyboard;
}					   

filter_result KeybdFilter::Filter(BMessage *message, BHandler **target) { 
	//if (message->what != B_KEY_DOWN && message->what != B_KEY_UP) {
	if (message->what != B_KEY_DOWN) {
		return(B_DISPATCH_MESSAGE);
	}
	((ev_BeOSKeyboard*)m_pEVKeyboard)->keyPressEvent(m_pBeOSFrame->getCurrentView(), message);
	//pView->draw();
	return(B_SKIP_MESSAGE);			
}

/**************************************************************/

ev_BeOSKeyboard::ev_BeOSKeyboard(EV_EditEventMapper* pEEM) : EV_Keyboard(pEEM)
{
}

UT_Bool ev_BeOSKeyboard::synthesize(XAP_BeOSApp * pBeOSApp, 
				    XAP_BeOSFrame * pBeOSFrame) {
	UT_ASSERT(pBeOSFrame); 
	
	be_Window *pBeWin;
	pBeWin = (be_Window*)pBeOSFrame->getTopLevelWindow();
	UT_ASSERT(pBeWin);

	pBeWin->Lock();
	pBeWin->m_pbe_DocView->AddFilter(new KeybdFilter(pBeOSApp, 
					  	         pBeOSFrame, 
						         this));
	pBeWin->Unlock();
	return UT_TRUE;
}

//Handle mapping
UT_Bool ev_BeOSKeyboard::keyPressEvent(AV_View* pView, BMessage *msg)
{
	EV_EditBits state = 0;
	EV_EditEventMapperResult result;
	EV_EditMethod * pEM;
	
	key_map *keymap;
	char 	*chars;
	get_key_map(&keymap, &chars);

/* A typical keyboard message looks like:
BMessage: what = _KYD (0x5f4b5944, or 1598773572)
    entry           when, type=LLNG, c=1, size= 8, data[0]: 
    entry      modifiers, type=LONG, c=1, size= 4, data[0]: 0x0 
    entry            key, type=LONG, c=1, size= 4, data[0]: 0x41
    entry         states, type=UBYT, c=1, size=16,
    entry       raw_char, type=LONG, c=1, size= 4, data[0]: 0x68 
    entry           byte, type=BYTE, c=1, size= 1,
    entry          bytes, type=CSTR, c=1, size= 2, data[0]: 
*/
	int32	modifier;
	int32 	keychar;
	int32	rawchar;
	int32   charindex;
		
	msg->FindInt32("modifiers", &modifier);
	msg->FindInt32("key", &keychar);
	msg->FindInt32("raw_char", &rawchar);
	
	DPRINTF(printf("Modifiers 0x%x keychar %c (0x%x) rawchar %c (0x%x)\n", 
					modifier, keychar, keychar, rawchar, rawchar));

	if (modifier & B_SHIFT_KEY) {
		state |= EV_EMS_SHIFT;
	}
	if (modifier & B_CONTROL_KEY) {
		state |= EV_EMS_CONTROL;
	}
	if (modifier & B_OPTION_KEY) {
		state |= EV_EMS_ALT;
	}
	
	//UT_ASSERT(keyval <= 0x00FF) //Latin ASCII type characters
	//if (s_isVirtualKeyCode(e->keyval))
	if ((rawchar >= 0x00FF) || 
	    s_mapVirtualKeyCodeToNVK(keychar, modifier, rawchar) != EV_NVK__IGNORE__) {
	    DPRINTF(printf("Special Key Code \n"));
		EV_EditBits nvk = s_mapVirtualKeyCodeToNVK(keychar, modifier, rawchar);
		switch (nvk) {
		case EV_NVK__IGNORE__:
			return UT_FALSE;
		default:
			result = m_pEEM->Keystroke((UT_uint32)EV_EKP_PRESS|state|nvk,&pEM);

			switch (result)	{
			case EV_EEMR_BOGUS_START:
				// If it is a bogus key and we don't have a sequence in
				// progress, we should let the system handle it
				// (this lets things like ALT-F4 work).
				return UT_FALSE;
				
			case EV_EEMR_BOGUS_CONT:
				// If it is a bogus key but in the middle of a sequence,
				// we should silently eat it (this is to prevent things
				// like Control-X ALT-F4 from killing us -- if they want
				// to kill us, fine, but they shouldn't be in the middle
				// of a sequence).
				return UT_TRUE;
				
			case EV_EEMR_COMPLETE:
				UT_ASSERT(pEM);
				// no char data to offer
				invokeKeyboardMethod(pView,pEM,0,0); 
				return UT_TRUE;
				
			case EV_EEMR_INCOMPLETE:
				return UT_TRUE;
				
			default:
				UT_ASSERT(0);
				return UT_TRUE;
			}
		}
	}
	else {
		DPRINTF(printf("Ascii Key Code \n"));
		UT_uint16 charData = (UT_uint16)rawchar;
		
		if (modifier & B_SHIFT_KEY) {
			charindex = keymap->shift_map[(char)keychar];
			charData = chars[charindex + 1];		//Only 1 byte chars right now
		}
		if (modifier & B_CONTROL_KEY) {
		}
		if (modifier & B_OPTION_KEY) {
		}
		
		result = m_pEEM->Keystroke(EV_EKP_PRESS|state|charData,&pEM);
		switch (result) {
		case EV_EEMR_BOGUS_START:
			// If it is a bogus key and we don't have a sequence in
			// progress, we should let the system handle it
			// (this lets things like ALT-F4 work).
			return UT_FALSE;
			
		case EV_EEMR_BOGUS_CONT:
			// If it is a bogus key but in the middle of a sequence,
			// we should silently eat it (this is to prevent things
			// like Control-X ALT-F4 from killing us -- if they want
			// to kill us, fine, but they shouldn't be in the middle
			// of a sequence).
			return UT_TRUE;
			
		case EV_EEMR_COMPLETE:
			UT_ASSERT(pEM);
			// no char data to offer
			invokeKeyboardMethod(pView,pEM,&charData,1); 
			return UT_TRUE;
			
		case EV_EEMR_INCOMPLETE:
			return UT_TRUE;
			
		default:
			UT_ASSERT(0);
			return UT_TRUE;
		}
	}
	return UT_FALSE;
}

EV_EditBits s_mapVirtualKeyCodeToNVK(int keyval, int modifiers, int charval) {
	//switch (keyval) {
	switch (charval) {
	case B_BACKSPACE:	//0x08 (same as '\\b')
		return(EV_NVK_BACKSPACE);
	//case B_ENTER: 		//0x0a (same as '\\n')
	case B_RETURN:		//0x0a (synonym for B_ENTER)
		return(EV_NVK_RETURN);
	case B_SPACE: 		//0x20 (same as ' ')
		return(EV_NVK_SPACE);
	case B_TAB:			//0x09 (same as '\\t')
		return(EV_NVK_TAB);
	case B_ESCAPE:		//0x1b
		return(EV_NVK_ESCAPE);
	case B_LEFT_ARROW:	//0x1c
		return(EV_NVK_LEFT);	
	case B_RIGHT_ARROW: //0x1d
		return(EV_NVK_RIGHT);
	case B_UP_ARROW:	//0x1e
		return(EV_NVK_UP);
	case B_DOWN_ARROW:	//0x1f
		return(EV_NVK_DOWN);
	case B_INSERT:		//0x05
		return(EV_NVK_INSERT);
	case B_DELETE:		//0x7f
		return(EV_NVK_DELETE);
	case B_HOME:		//0x01
		return(EV_NVK_HOME);
	case B_END:			//0x04
		return(EV_NVK_END);
	case B_PAGE_UP:		//0x0b
		return(EV_NVK_PAGEUP);
	case B_PAGE_DOWN:	//0x0c
		return(EV_NVK_PAGEDOWN);
	}

	if (charval == B_FUNCTION_KEY) { 	//0x10
		switch(keyval) {
		case B_F1_KEY:
			return(EV_NVK_F1);
		case B_F2_KEY:
			return(EV_NVK_F2);
		case B_F3_KEY:
			return(EV_NVK_F3);
		case B_F4_KEY:
			return(EV_NVK_F4);
		case B_F5_KEY:
			return(EV_NVK_F5);
		case B_F6_KEY:
			return(EV_NVK_F6);
		case B_F7_KEY:
			return(EV_NVK_F7);
		case B_F8_KEY:
			return(EV_NVK_F8);
		case B_F9_KEY:
			return(EV_NVK_F9);
		case B_F10_KEY:
			return(EV_NVK_F10);
		case B_F11_KEY:
			return(EV_NVK_F11);
		case B_F12_KEY:
			return(EV_NVK_F12);
		//Goes all the way to EV_NVK_F34
		}
	}

	//UT_ASSERT(0);
	return EV_NVK__IGNORE__;
}
