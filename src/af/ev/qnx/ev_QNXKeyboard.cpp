/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include "xav_View.h"
#include "ev_NamedVirtualKey.h"
#include "ev_QNXKeyboard.h"
#include "ev_EditBits.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////

static EV_EditBits s_mapVirtualKeyCodeToNVK(PhKeyEvent_t *keyevent);
static UT_Bool s_isVirtualKeyCode(PhKeyEvent_t *keyevent);
static int s_getKeyEventValue(PhKeyEvent_t *keyevent);

//////////////////////////////////////////////////////////////////

ev_QNXKeyboard::ev_QNXKeyboard(EV_EditEventMapper* pEEM)
	: EV_Keyboard(pEEM)
{
}

ev_QNXKeyboard::~ev_QNXKeyboard(void)
{
}

/*
 This will get called by the static callback handler which 
 is attached ... elsewhere in the Frame
*/
UT_Bool ev_QNXKeyboard::keyPressEvent(AV_View* pView,
					PtCallbackInfo_t* e)
{
	PhKeyEvent_t *keyevent;

	keyevent = (PhKeyEvent_t *)PhGetData(e->event);
	//We only want to catch the down and repeat keys, ignore all others
	if (!keyevent || 
        !(keyevent->key_flags & (Pk_KF_Key_Down | Pk_KF_Key_Repeat)))  {
		return(UT_FALSE);
	}

	EV_EditBits state = 0;
	EV_EditEventMapperResult result;
	EV_EditMethod * pEM;

	if (keyevent->key_mods & Pk_KM_Shift) 
		state |= EV_EMS_SHIFT;
	if (keyevent->key_mods & Pk_KM_Ctrl) 
		state |= EV_EMS_CONTROL;
	if (keyevent->key_mods & Pk_KM_Alt) 
		state |= EV_EMS_ALT;

	if (s_isVirtualKeyCode(keyevent))
	{
		EV_EditBits nvk = s_mapVirtualKeyCodeToNVK(keyevent);

		//printf("Virtual 0x%x (ignore is 0x%x) \n", nvk, EV_NVK__IGNORE__);
		switch (nvk)
		{
		case EV_NVK__IGNORE__:
			return UT_FALSE;
		default:
			result = m_pEEM->Keystroke((UT_uint32)EV_EKP_PRESS|state|nvk,&pEM);

			switch (result)
			{
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
				invokeKeyboardMethod(pView,pEM,0,0); // no char data to offer
				return UT_TRUE;
				
			case EV_EEMR_INCOMPLETE:
				return UT_TRUE;
				
			default:
				UT_ASSERT(0);
				return UT_TRUE;
			}
		}
	}
	else
	{
		UT_uint16 charData;
		charData = s_getKeyEventValue(keyevent);

		//printf("Real key value [%c] \n", charData);
		result = m_pEEM->Keystroke(EV_EKP_PRESS|state|charData,&pEM);

		switch (result)
		{
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
			invokeKeyboardMethod(pView,pEM,&charData,1); // no char data to offer
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

#if 0
// pulled in from gdk/gdkkeysyms.h
static EV_EditBits s_Table_NVK_0xff[] =
{
	EV_NVK_BACKSPACE,    // GDK_Backspace 0xFF08
	EV_NVK_TAB,          // GDK_Tab 0xFF09
	EV_NVK__IGNORE__,    // GDK_Linefeed 0xFF0A
	EV_NVK__IGNORE__,    // GDK_Clear 0xFF0B
 	EV_NVK_RETURN,       // GDK_Return 0xFF0D
	EV_NVK_ESCAPE,       // GDK_Escape 0xFF1B
	EV_NVK_HOME,         // GDK_Home 0xFF50
	EV_NVK_LEFT,         // GDK_Left 0xFF51
	EV_NVK_UP,           // GDK_Up 0xFF52
	EV_NVK_RIGHT,        // GDK_Right 0xFF53
	EV_NVK_DOWN,         // GDK_Down 0xFF54
	EV_NVK_PAGEUP,       // GDK_Page_Up 0xFF55
	EV_NVK_PAGEDOWN,     // GDK_Page_Down 0xFF56
	EV_NVK_END,          // GDK_End 0xFF57
	EV_NVK_INSERT,       // GDK_Insert 0xFF63
	EV_NVK_TAB,          // GDK_KP_Tab 0xFF89
	EV_NVK_RETURN,       // GDK_KP_Enter 0xFF8D
	EV_NVK_F1,           // GDK_KP_F1 0xFF91
	EV_NVK_F2,           // GDK_KP_F2 0xFF92
	EV_NVK_F3,           // GDK_KP_F3 0xFF93
	EV_NVK_F4,           // GDK_KP_F4 0xFF94
	EV_NVK_HOME,         // GDK_KP_Home 0xFF95
	EV_NVK_LEFT,         // GDK_KP_Left 0xFF96
	EV_NVK_UP,           // GDK_KP_Up 0xFF97
	EV_NVK_RIGHT,        // GDK_KP_Right 0xFF98
	EV_NVK_DOWN,         // GDK_KP_Down 0xFF99
	EV_NVK_PAGEUP,       // GDK_KP_Prior 0xFF9A
	EV_NVK_PAGEDOWN,     // GDK_KP_Next 0xFF9B
	EV_NVK_END,          // GDK_KP_End 0xFF9C
	EV_NVK_HOME,         // GDK_KP_Begin 0xFF9D
	EV_NVK_INSERT,       // GDK_KP_Insert 0xFF9E
	EV_NVK_DELETE,       // GDK_KP_Delete 0xFF9F
	EV_NVK_F1,           // GDK_F1 0xFFBE
	EV_NVK_F2,           // GDK_F2 0xFFBF
	EV_NVK_F3,           // GDK_F3 0xFFC0
	EV_NVK_F4,           // GDK_F4 0xFFC1
	EV_NVK_F5,           // GDK_F5 0xFFC2
	EV_NVK_F6,           // GDK_F6 0xFFC3
	EV_NVK_F7,           // GDK_F7 0xFFC4
	EV_NVK_F8,           // GDK_F8 0xFFC5
	EV_NVK_F9,           // GDK_F9 0xFFC6
	EV_NVK_F10,          // GDK_F10 0xFFC7
	EV_NVK_F11,          // GDK_F11 0xFFC8
	EV_NVK_F12,          // GDK_F12 0xFFC9
	EV_NVK_F13,          // GDK_F13 0xFFCA
	EV_NVK_F14,          // GDK_F14 0xFFCB
	EV_NVK_F15,          // GDK_F15 0xFFCC
	EV_NVK_F16,          // GDK_F16 0xFFCD
	EV_NVK_F17,          // GDK_F17 0xFFCE
	EV_NVK_F18,          // GDK_F18 0xFFCF
	EV_NVK_F19,          // GDK_F19 0xFFD0
	EV_NVK_F20,          // GDK_F20 0xFFD1
	EV_NVK_F21,          // GDK_F21 0xFFD2
	EV_NVK_F22,          // GDK_F22 0xFFD3
	EV_NVK_F23,          // GDK_F23 0xFFD4
	EV_NVK_F24,          // GDK_F24 0xFFD5
	EV_NVK_F25,          // GDK_F25 0xFFD6
	EV_NVK_F26,          // GDK_F26 0xFFD7
	EV_NVK_F27,          // GDK_F27 0xFFD8
	EV_NVK_F28,          // GDK_F28 0xFFD9
	EV_NVK_F29,          // GDK_F29 0xFFDA
	EV_NVK_F30,          // GDK_F30 0xFFDB
	EV_NVK_F31,          // GDK_F31 0xFFDC
	EV_NVK_F32,          // GDK_F32 0xFFDD
	EV_NVK_F33,          // GDK_F33 0xFFDE
	EV_NVK_F34,          // GDK_F34 0xFFDF
	EV_NVK_F35,          // GDK_F35 0xFFE0
	EV_NVK_MENU_SHORTCUT,    // GDK_Hyper_R 0xFFEE
	EV_NVK_DELETE,       // GDK_Delete 0xFFFF
};

#endif

/*
 These three functions all work hand in hand to get the
key events ... 
*/
static UT_Bool s_isVirtualKeyCode(PhKeyEvent_t *keyevent)
{
	int key;
/*
	if (keyevent->key_sym <= 0xff) {
		printf("Key Symbol is 0x%x \n", keyevent->key_sym);
		return(UT_FALSE);
	}
*/
	if ((key = PhTo8859_1(keyevent)) == -1) {
		return(UT_TRUE);
	}

	switch (keyevent->key_cap) {
	case ' ':
	case Pk_BackSpace:
	case Pk_Tab:
 	case Pk_Return:
	case Pk_Escape:
	case Pk_Home:
	case Pk_Left:
	case Pk_Right:
	case Pk_Down:
	case Pk_Up:
	case Pk_Pg_Up:
	case Pk_Pg_Down:
	case Pk_End:
	case Pk_Insert:
	case Pk_Delete:
	case Pk_Menu:
	case Pk_F1:
	case Pk_F2:
	case Pk_F3:
	case Pk_F4:
	case Pk_F5:
	case Pk_F6:
	case Pk_F7:
	case Pk_F8:
	case Pk_F9:
	case Pk_F10:
	case Pk_F11:
	case Pk_F12:
	case Pk_F13:
	case Pk_F14:
	case Pk_F15:
	case Pk_F16:
	case Pk_F17:
	case Pk_F18:
	case Pk_F19:
	case Pk_F20:
	case Pk_F21:
	case Pk_F22:
	case Pk_F23:
	case Pk_F24:
	case Pk_F25:
	case Pk_F26:
	case Pk_F27:
	case Pk_F28:
	case Pk_F29:
	case Pk_F30:
	case Pk_F31:
	case Pk_F32:
	case Pk_F33:
	case Pk_F34:
	case Pk_F35:
		return UT_TRUE;	
	default:
		break;
	}

	return UT_FALSE;
}

static EV_EditBits s_mapVirtualKeyCodeToNVK(PhKeyEvent_t *keyevent)
{
	
	//Ignore keys with symbols (key press?)
#if 0
	printf("-- Key Symbol: 0x%x \n", keyevent->key_sym);
	printf("Key Cap:    0x%x \n", keyevent->key_cap);
	printf("Key Flags:  0x%x \n", keyevent->key_flags);
	printf("Key Mods:   0x%x \n", keyevent->key_mods);
#endif

	switch (keyevent->key_cap) {
	case ' ':
		return EV_NVK_SPACE;
	case Pk_BackSpace:
		return EV_NVK_BACKSPACE;
	case Pk_Tab:
		return EV_NVK_TAB;
 	case Pk_Return:
		return EV_NVK_RETURN;
	case Pk_Escape:
		return EV_NVK_ESCAPE;
	case Pk_Home:
		return EV_NVK_HOME;
	case Pk_Left:
		return EV_NVK_LEFT;
	case Pk_Up:
		return EV_NVK_UP;
	case Pk_Right:
		return EV_NVK_RIGHT;
	case Pk_Down:
		return EV_NVK_DOWN;
	case Pk_Pg_Up:
		return EV_NVK_PAGEUP;
	case Pk_Pg_Down:
		return EV_NVK_PAGEDOWN;
	case Pk_End:
		return EV_NVK_END;
	case Pk_Insert:
		return EV_NVK_INSERT;
	case Pk_Delete:
		return EV_NVK_DELETE;
	case Pk_Menu:
		return EV_NVK_MENU_SHORTCUT;
	case Pk_F1:
		return EV_NVK_F1;
	case Pk_F2:
		return EV_NVK_F2;
	case Pk_F3:
		return EV_NVK_F3;
	case Pk_F4:
		return EV_NVK_F4;
	case Pk_F5:
		return EV_NVK_F5;
	case Pk_F6:
		return EV_NVK_F6;
	case Pk_F7:
		return EV_NVK_F7;
	case Pk_F8:
		return EV_NVK_F8;
	case Pk_F9:
		return EV_NVK_F9;
	case Pk_F10:
		return EV_NVK_F10;
	case Pk_F11:
		return EV_NVK_F11;
	case Pk_F12:
		return EV_NVK_F12;
	case Pk_F13:
		return EV_NVK_F13;
	case Pk_F14:
		return EV_NVK_F14;
	case Pk_F15:
		return EV_NVK_F15;
	case Pk_F16:
		return EV_NVK_F16;
	case Pk_F17:
		return EV_NVK_F17;
	case Pk_F18:
		return EV_NVK_F18;
	case Pk_F19:
		return EV_NVK_F19;
	case Pk_F20:
		return EV_NVK_F20;
	case Pk_F21:
		return EV_NVK_F21;
	case Pk_F22:
		return EV_NVK_F22;
	case Pk_F23:
		return EV_NVK_F23;
	case Pk_F24:
		return EV_NVK_F24;
	case Pk_F25:
		return EV_NVK_F25;
	case Pk_F26:
		return EV_NVK_F26;
	case Pk_F27:
		return EV_NVK_F27;
	case Pk_F28:
		return EV_NVK_F28;
	case Pk_F29:
		return EV_NVK_F29;
	case Pk_F30:
		return EV_NVK_F30;
	case Pk_F31:
		return EV_NVK_F31;
	case Pk_F32:
		return EV_NVK_F32;
	case Pk_F33:
		return EV_NVK_F33;
	case Pk_F34:
		return EV_NVK_F34;
	case Pk_F35:
		return EV_NVK_F35;
	default:
		break;
	}


	return EV_NVK__IGNORE__;
}

static int s_getKeyEventValue(PhKeyEvent_t *keyevent) {
	int key;

	//TODO: Make this more international!
	if ((key = PhTo8859_1(keyevent)) == -1) {
		return(EV_NVK__IGNORE__);
	}
	return(key);
}

