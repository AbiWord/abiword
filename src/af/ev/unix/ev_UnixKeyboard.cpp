 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include <gdk/gdk.h>

#include "ut_types.h"
#include "ut_assert.h"

#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_EditMethod.h"
#include "fv_View.h"
#include "ev_NamedVirtualKey.h"
#include "ev_UnixKeyboard.h"

static EV_EditBits s_mapVirtualKeyCodeToNVK(gint nVirtKey);

ev_UnixKeyboard::ev_UnixKeyboard(EV_EditEventMapper* pEEM) : EV_Keyboard(pEEM)
{
	m_pEEM = pEEM;
}

UT_Bool ev_UnixKeyboard::keyPressEvent(FV_View* pView,
									   GdkEventKey* e)
{
	EV_EditBits state = 0;
	UT_uint32 iPrefix;
	EV_EditEventMapperResult result;
	EV_EditMethod * pEM;
	
	if (e->state & GDK_SHIFT_MASK)
		state |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		state |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		state |= EV_EMS_ALT;
		
	if (e->keyval > 0xff00)  // virt key
	{
		EV_EditBits nvk = s_mapVirtualKeyCodeToNVK(e->keyval - 0xff00);
		switch (nvk)
		{
		case EV_NVK__IGNORE__:
			return UT_FALSE;
		default:
			result = m_pEEM->Keystroke((UT_uint32)EV_EKP_PRESS|state|nvk,&pEM,&iPrefix);

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
				invokeKeyboardMethod(pView,pEM,iPrefix,0,0); // no char data to offer
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
		UT_uint16 charData = e->keyval;
		result = m_pEEM->Keystroke(EV_EKP_PRESS|state|charData,&pEM,&iPrefix);

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
			invokeKeyboardMethod(pView,pEM,iPrefix,&charData,1); // no char data to offer
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

// pulled in from gdk/gdkkeysyms.h
static EV_EditBits s_Table_NVK[] =
{
	0, // 00
	0, // 01
	0, // 02
	0, // 03
	0, // 04
	0, // 05
	0, // 06
	0, // 07
	EV_NVK_BACKSPACE,    // GDK_Backspace 0xFF08
	EV_NVK_TAB,          // GDK_Tab 0xFF09
	EV_NVK__IGNORE__,    // GDK_Linefeed 0xFF0A
	EV_NVK__IGNORE__,    // GDK_Clear 0xFF0B
	0,                   // 0C
	EV_NVK_RETURN,       // GDK_Return 0xFF0D
	0,
	0,
	0,
	0,
	0,
	EV_NVK__IGNORE__,    // GDK_Pause 0xFF13
    EV_NVK__IGNORE__,    // GDK_Scroll_Lock 0xFF14
	EV_NVK__IGNORE__,    // GDK_Sys_Req 0xFF15
	0,
	0,
	0,
	0,
	0,
	EV_NVK_ESCAPE,       // GDK_Escape 0xFF1B
	0, 0, 0, 0,
	EV_NVK__IGNORE__,    // GDK_Multi_key 0xFF20
	EV_NVK__IGNORE__,    //  GDK_Kanji 0xFF21
	EV_NVK__IGNORE__,    // GDK_Muhenkan 0xFF22
	EV_NVK__IGNORE__,    // GDK_Henkan_Mode 0xFF23
	EV_NVK__IGNORE__,    // GDK_Henkan 0xFF23
	EV_NVK__IGNORE__,    // GDK_Romaji 0xFF24
	EV_NVK__IGNORE__,    //  GDK_Hiragana 0xFF25
	EV_NVK__IGNORE__,    //  GDK_Katakana 0xFF26
	EV_NVK__IGNORE__,    // GDK_Hiragana_Katakana 0xFF27
	EV_NVK__IGNORE__,    // GDK_Zenkaku 0xFF28
	EV_NVK__IGNORE__,    // GDK_Hankaku 0xFF29
	EV_NVK__IGNORE__,    // GDK_Zenkaku_Hankaku 0xFF2A
	EV_NVK__IGNORE__,    // GDK_Touroku 0xFF2B
	EV_NVK__IGNORE__,    //  GDK_Massyo 0xFF2C
	EV_NVK__IGNORE__,    // GDK_Kana_Lock 0xFF2D
	EV_NVK__IGNORE__,    // GDK_Kana_Shift 0xFF2E
	EV_NVK__IGNORE__,    // GDK_Eisu_Shift 0xFF2F
	EV_NVK__IGNORE__,    // GDK_Eisu_toggle 0xFF30
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	EV_NVK_HOME,    // GDK_Home 0xFF50
	EV_NVK_LEFT,    // GDK_Left 0xFF51
	EV_NVK_UP,    // GDK_Up 0xFF52
	EV_NVK_RIGHT,    // GDK_Right 0xFF53
	EV_NVK_DOWN,    // GDK_Down 0xFF54
	EV_NVK_PAGEUP,    // GDK_Page_Up 0xFF55
	EV_NVK_PAGEDOWN,    // GDK_Page_Down 0xFF56
	EV_NVK_END,    // GDK_End 0xFF57
	EV_NVK__IGNORE__,    // GDK_Begin 0xFF58
	0, 0, 0, 0, 0, 0, 0,
	EV_NVK__IGNORE__,    // GDK_Select 0xFF60
	EV_NVK__IGNORE__,    // GDK_Print 0xFF61
	EV_NVK__IGNORE__,    // GDK_Execute 0xFF62
	EV_NVK_INSERT,    // GDK_Insert 0xFF63
	0, 
	EV_NVK__IGNORE__,    // GDK_Undo 0xFF65
	EV_NVK__IGNORE__,    // GDK_Redo 0xFF66
	EV_NVK__IGNORE__,    //  GDK_Menu 0xFF67
	EV_NVK__IGNORE__,    // GDK_Find 0xFF68
	EV_NVK__IGNORE__,    // GDK_Cancel 0xFF69
	EV_NVK__IGNORE__,    // GDK_Help 0xFF6A
	EV_NVK__IGNORE__,    // GDK_Break 0xFF6B
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	EV_NVK__IGNORE__,    // GDK_script_switch 0xFF7E
	EV_NVK__IGNORE__,    // GDK_Num_Lock 0xFF7F
	EV_NVK__IGNORE__,    // GDK_KP_Space 0xFF80
	0, 0, 0, 0, 0, 0, 0, 0,
	EV_NVK_TAB,    // GDK_KP_Tab 0xFF89
	0, 0, 0,
	EV_NVK_RETURN,    // GDK_KP_Enter 0xFF8D
	0, 0, 0,
	EV_NVK_F1,    // GDK_KP_F1 0xFF91
	EV_NVK_F2,    // GDK_KP_F2 0xFF92
	EV_NVK_F3,    // GDK_KP_F3 0xFF93
	EV_NVK_F4,    // GDK_KP_F4 0xFF94
	EV_NVK_HOME,    // GDK_KP_Home 0xFF95
	EV_NVK_LEFT,    // GDK_KP_Left 0xFF96
	EV_NVK_UP,    // GDK_KP_Up 0xFF97
	EV_NVK_RIGHT,    // GDK_KP_Right 0xFF98
	EV_NVK_DOWN,    // GDK_KP_Down 0xFF99
	EV_NVK_PAGEUP,    // GDK_KP_Prior 0xFF9A
	EV_NVK_PAGEDOWN,    // GDK_KP_Next 0xFF9B
	EV_NVK_END,    // GDK_KP_End 0xFF9C
	EV_NVK_HOME,    // GDK_KP_Begin 0xFF9D
	EV_NVK_INSERT,    // GDK_KP_Insert 0xFF9E
	EV_NVK_DELETE,    // GDK_KP_Delete 0xFF9F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0,    // GDK_KP_Multiply 0xFFAA
	0,    // GDK_KP_Add 0xFFAB
	0,    // GDK_KP_Separator 0xFFAC
	0,    // GDK_KP_Subtract 0xFFAD
	0,    // GDK_KP_Decimal 0xFFAE
	0,    // GDK_KP_Divide 0xFFAF
	0,    // GDK_KP_0 0xFFB0
	0,    // GDK_KP_1 0xFFB1
	0,    // GDK_KP_2 0xFFB2
	0,    // GDK_KP_3 0xFFB3
	0,    // GDK_KP_4 0xFFB4
	0,    // GDK_KP_5 0xFFB5
	0,    // GDK_KP_6 0xFFB6
	0,    // GDK_KP_7 0xFFB7
	0,    // GDK_KP_8 0xFFB8
	0,    // GDK_KP_9 0xFFB9
	0, 0, 0,
	0,    // GDK_KP_Equal 0xFFBD
	EV_NVK_F1,    // GDK_F1 0xFFBE
	EV_NVK_F2,    // GDK_F2 0xFFBF
	EV_NVK_F3,    // GDK_F3 0xFFC0
	EV_NVK_F4,    // GDK_F4 0xFFC1
	EV_NVK_F5,    // GDK_F5 0xFFC2
	EV_NVK_F6,    // GDK_F6 0xFFC3
	EV_NVK_F7,    // GDK_F7 0xFFC4
	EV_NVK_F8,    // GDK_F8 0xFFC5
	EV_NVK_F9,    // GDK_F9 0xFFC6
	EV_NVK_F10,    // GDK_F10 0xFFC7
	EV_NVK_F11,    // GDK_F11 0xFFC8
	EV_NVK_F12,    // GDK_F12 0xFFC9
	EV_NVK_F13,    // GDK_F13 0xFFCA
	EV_NVK_F14,    // GDK_F14 0xFFCB
	EV_NVK_F15,    // GDK_F15 0xFFCC
	EV_NVK_F16,    // GDK_F16 0xFFCD
	EV_NVK_F17,    // GDK_F17 0xFFCE
	EV_NVK_F18,    // GDK_F18 0xFFCF
	EV_NVK_F19,    // GDK_F19 0xFFD0
	EV_NVK_F20,    // GDK_F20 0xFFD1
	EV_NVK_F21,    // GDK_F21 0xFFD2
	EV_NVK_F22,    // GDK_F22 0xFFD3
	EV_NVK_F23,    // GDK_F23 0xFFD4
	EV_NVK_F24,    // GDK_F24 0xFFD5
	EV_NVK_F25,    // GDK_F25 0xFFD6
	EV_NVK_F26,    // GDK_F26 0xFFD7
	EV_NVK_F27,    // GDK_F27 0xFFD8
	EV_NVK_F28,    // GDK_F28 0xFFD9
	EV_NVK_F29,    // GDK_F29 0xFFDA
	EV_NVK_F30,    // GDK_F30 0xFFDB
	EV_NVK_F31,    // GDK_F31 0xFFDC
	EV_NVK_F32,    // GDK_F32 0xFFDD
	EV_NVK_F33,    // GDK_F33 0xFFDE
	EV_NVK_F34,    // GDK_F34 0xFFDF
	EV_NVK_F35,    // GDK_F35 0xFFE0
	EV_NVK__IGNORE__,    // GDK_Shift_L 0xFFE1
	EV_NVK__IGNORE__,    // GDK_Shift_R 0xFFE2
	EV_NVK__IGNORE__,    // GDK_Control_L 0xFFE3
	EV_NVK__IGNORE__,    // GDK_Control_R 0xFFE4
	EV_NVK__IGNORE__,    // GDK_Caps_Lock 0xFFE5
	EV_NVK__IGNORE__,    // GDK_Shift_Lock 0xFFE6
	EV_NVK__IGNORE__,    // GDK_Meta_L 0xFFE7
	EV_NVK__IGNORE__,    // GDK_Meta_R 0xFFE8
	EV_NVK__IGNORE__,    // GDK_Alt_L 0xFFE9
	EV_NVK__IGNORE__,    // GDK_Alt_R 0xFFEA
	EV_NVK__IGNORE__,    // GDK_Super_L 0xFFEB
	EV_NVK__IGNORE__,    // GDK_Super_R 0xFFEC
	EV_NVK__IGNORE__,    // GDK_Hyper_L 0xFFED
	EV_NVK__IGNORE__,    // GDK_Hyper_R 0xFFEE
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	EV_NVK_DELETE,       // GDK_Delete 0xFFFF
};



#define NrElements(a)	((sizeof(a) / sizeof(a[0])))

static EV_EditBits s_mapVirtualKeyCodeToNVK(gint nVirtKey)
{
	// map the given virtual key into a "named virtual key".
	// these are referenced by NVK_ symbol so that the cross
	// platform code can properly refer to them.
	
	UT_ASSERT(nVirtKey <= NrElements(s_Table_NVK));

	return s_Table_NVK[nVirtKey];
}


