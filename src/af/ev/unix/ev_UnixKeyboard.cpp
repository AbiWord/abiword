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
 

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "ev_NamedVirtualKey.h"
#include "ev_UnixKeyboard.h"
#include "ev_UnixKeysym2ucs.cpp"

#include"ut_mbtowc.h"

//////////////////////////////////////////////////////////////////

static EV_EditBits s_mapVirtualKeyCodeToNVK(gint keyval);
static bool s_isVirtualKeyCode(gint keyval);
static GdkModifierType s_getAltMask(void);

//////////////////////////////////////////////////////////////////
// This is used to remember the modifier mask (GDK_MODx_MASK) that
// is bound to the Alt keys.  We load this once per session.
//
// TODO Figure out if GTK/GDK can send us a MappingNotify event.
// TODO If so, recompute this value.
//
static GdkModifierType s_alt_mask = GDK_MODIFIER_MASK;	// bogus value

GdkModifierType ev_UnixKeyboard::getAltModifierMask(void)
{
	return s_alt_mask;
}

//////////////////////////////////////////////////////////////////

ev_UnixKeyboard::ev_UnixKeyboard(EV_EditEventMapper* pEEM)
	: EV_Keyboard(pEEM)
{
	if (s_alt_mask == GDK_MODIFIER_MASK)
		s_alt_mask = s_getAltMask();
}

ev_UnixKeyboard::~ev_UnixKeyboard(void)
{
}

bool ev_UnixKeyboard::keyPressEvent(AV_View* pView,
									   GdkEventKey* e)
{
	EV_EditBits state = 0;
	EV_EditEventMapperResult result;
	EV_EditMethod * pEM;
	
	if (e->state & GDK_SHIFT_MASK)
		state |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		state |= EV_EMS_CONTROL;
	if (e->state & (s_alt_mask))
		state |= EV_EMS_ALT;

	//UT_DEBUGMSG(("KeyPressEvent: keyval=%x state=%x\n",e->keyval,state));
	
	if (s_isVirtualKeyCode(e->keyval))
	{
		EV_EditBits nvk = s_mapVirtualKeyCodeToNVK(e->keyval);

		switch (nvk)
		{
		case EV_NVK__IGNORE__:
			return false;
		default:

			result = m_pEEM->Keystroke((UT_uint32)EV_EKP_PRESS|state|nvk,&pEM);

			switch (result)
			{
			case EV_EEMR_BOGUS_START:
				// If it is a bogus key and we don't have a sequence in
				// progress, we should let the system handle it
				// (this lets things like ALT-F4 work).
				return false;
				
			case EV_EEMR_BOGUS_CONT:
				// If it is a bogus key but in the middle of a sequence,
				// we should silently eat it (this is to prevent things
				// like Control-X ALT-F4 from killing us -- if they want
				// to kill us, fine, but they shouldn't be in the middle
				// of a sequence).
				return true;
				
			case EV_EEMR_COMPLETE:
				UT_ASSERT(pEM);
				invokeKeyboardMethod(pView,pEM,0,0); // no char data to offer
				return true;
				
			case EV_EEMR_INCOMPLETE:
				return true;
				
			default:
				UT_ASSERT(0);
				return true;
			}
		}
	}
	else
	{
		UT_uint16 charData = e->keyval;
		//UT_DEBUGMSG(("UnixKeyboard::pressKeyEvent: key value %x\n", charData));
		UT_Mbtowc m;
		wchar_t wc;
	  	UT_UCSChar *ucs = NULL;
	  	char *mbs=e->string;
	  	int mLength=strlen(mbs);
	  	int uLength;

		if(charData>0xff)
		  result = m_pEEM->Keystroke(EV_EKP_PRESS|state|'a',&pEM);
		else
		  result = m_pEEM->Keystroke(EV_EKP_PRESS|state|charData,&pEM);

		switch (result)
		{
		case EV_EEMR_BOGUS_START:
			// If it is a bogus key and we don't have a sequence in
			// progress, we should let the system handle it
			// (this lets things like ALT-F4 work).
			return false;
			
		case EV_EEMR_BOGUS_CONT:
			// If it is a bogus key but in the middle of a sequence,
			// we should silently eat it (this is to prevent things
			// like Control-X ALT-F4 from killing us -- if they want
			// to kill us, fine, but they shouldn't be in the middle
			// of a sequence).
			return true;
			
		case EV_EEMR_COMPLETE:
			UT_ASSERT(pEM);

			uLength=0;
			/*
				if gdk fails to translate, then we will try to do this
				ourselves by calling kesym2ucs
			*/
			
			if(mLength == 0)
			{
				UT_sint32 u = keysym2ucs(e->keyval);
				
				if(u == -1 || u > 0xFFFF) //conversion failed, or more than 16 bit requied
				{
					mLength = 0;
				}
				else
				{
					mLength = 1;
					ucs = new UT_UCSChar[1];
					ucs[0] = u;
				}
				
				uLength = mLength;
				//UT_DEBUGMSG(("#TF: keyval=%x, ucs=%x\n", ucs[0]));
			}
			else
			{
				ucs=new UT_UCSChar[mLength];
				for(int i=0;i<mLength;++i)
			  	{
					if(m.mbtowc(wc,mbs[i]))
				  	ucs[uLength++]=wc;
			  	}					
			 }
			invokeKeyboardMethod(pView,pEM,ucs,uLength); // no char data to offer
			delete[] ucs;
 			return true;
	
			
		case EV_EEMR_INCOMPLETE:
			return true;
			
		default:
			UT_ASSERT(0);
			return true;
		}
	}

	return false;
}

// pulled in from gdk/gdkkeysyms.h
static EV_EditBits s_Table_NVK_0xff[] =
{	0, // 00
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
	EV_NVK__IGNORE__,    // GDK_Kanji 0xFF21
	EV_NVK__IGNORE__,    // GDK_Muhenkan 0xFF22
	EV_NVK__IGNORE__,    // GDK_Henkan_Mode 0xFF23
	EV_NVK__IGNORE__,    // GDK_Henkan 0xFF23
	EV_NVK__IGNORE__,    // GDK_Romaji 0xFF24
	EV_NVK__IGNORE__,    // GDK_Hiragana 0xFF25
	EV_NVK__IGNORE__,    // GDK_Katakana 0xFF26
	EV_NVK__IGNORE__,    // GDK_Hiragana_Katakana 0xFF27
	EV_NVK__IGNORE__,    // GDK_Zenkaku 0xFF28
	EV_NVK__IGNORE__,    // GDK_Hankaku 0xFF29
	EV_NVK__IGNORE__,    // GDK_Zenkaku_Hankaku 0xFF2A
	EV_NVK__IGNORE__,    // GDK_Touroku 0xFF2B
	EV_NVK__IGNORE__,    // GDK_Massyo 0xFF2C
	EV_NVK__IGNORE__,    // GDK_Kana_Lock 0xFF2D
	EV_NVK__IGNORE__,    // GDK_Kana_Shift 0xFF2E
	EV_NVK__IGNORE__,    // GDK_Eisu_Shift 0xFF2F
	EV_NVK__IGNORE__,    // GDK_Eisu_toggle 0xFF30
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	EV_NVK_HOME,         // GDK_Home 0xFF50
	EV_NVK_LEFT,         // GDK_Left 0xFF51
	EV_NVK_UP,           // GDK_Up 0xFF52
	EV_NVK_RIGHT,        // GDK_Right 0xFF53
	EV_NVK_DOWN,         // GDK_Down 0xFF54
	EV_NVK_PAGEUP,       // GDK_Page_Up 0xFF55
	EV_NVK_PAGEDOWN,     // GDK_Page_Down 0xFF56
	EV_NVK_END,          // GDK_End 0xFF57
	EV_NVK__IGNORE__,    // GDK_Begin 0xFF58
	0, 0, 0, 0, 0, 0, 0,
	EV_NVK__IGNORE__,    // GDK_Select 0xFF60
	EV_NVK__IGNORE__,    // GDK_Print 0xFF61
	EV_NVK__IGNORE__,    // GDK_Execute 0xFF62
	EV_NVK_INSERT,       // GDK_Insert 0xFF63
	0, 
	EV_NVK__IGNORE__,    // GDK_Undo 0xFF65
	EV_NVK__IGNORE__,    // GDK_Redo 0xFF66
	EV_NVK__IGNORE__,    // GDK_Menu 0xFF67
	EV_NVK__IGNORE__,    // GDK_Find 0xFF68
	EV_NVK__IGNORE__,    // GDK_Cancel 0xFF69
	EV_NVK__IGNORE__,    // GDK_Help 0xFF6A
	EV_NVK__IGNORE__,    // GDK_Break 0xFF6B
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	EV_NVK__IGNORE__,    // GDK_script_switch 0xFF7E
	EV_NVK__IGNORE__,    // GDK_Num_Lock 0xFF7F
	EV_NVK__IGNORE__,    // GDK_KP_Space 0xFF80
	0, 0, 0, 0, 0, 0, 0, 0,
	EV_NVK_TAB,          // GDK_KP_Tab 0xFF89
	0, 0, 0,
	EV_NVK_RETURN,       // GDK_KP_Enter 0xFF8D
	0, 0, 0,
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
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0,                   // GDK_KP_Multiply 0xFFAA
	0,                   // GDK_KP_Add 0xFFAB
	0,                   // GDK_KP_Separator 0xFFAC
	0,                   // GDK_KP_Subtract 0xFFAD
	0,                   // GDK_KP_Decimal 0xFFAE
	0,                   // GDK_KP_Divide 0xFFAF
	0,                   // GDK_KP_0 0xFFB0
	0,                   // GDK_KP_1 0xFFB1
	0,                   // GDK_KP_2 0xFFB2
	0,                   // GDK_KP_3 0xFFB3
	0,                   // GDK_KP_4 0xFFB4
	0,                   // GDK_KP_5 0xFFB5
	0,                   // GDK_KP_6 0xFFB6
	0,                   // GDK_KP_7 0xFFB7
	0,                   // GDK_KP_8 0xFFB8
	0,                   // GDK_KP_9 0xFFB9
	0, 0, 0,
	0,                   // GDK_KP_Equal 0xFFBD
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
	EV_NVK_MENU_SHORTCUT,    // GDK_Hyper_R 0xFFEE
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	EV_NVK_DELETE,       // GDK_Delete 0xFFFF
};

static EV_EditBits s_Table_NVK_0xfe[] =		// ISO 9995 Function and Modifier Keys
{											// see /usr/include/X11/keysymdef.h
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfe00 - 0xfe0f
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfe10 - 0xfe1f
	EV_NVK_TAB,								// 0xfe20 left_tab
	  0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfe21 - 0xfe2f
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfe30 - 0xfe3f
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfe40 - 0xfe4f
	EV_NVK_DEAD_GRAVE,						// 0xfe50 dead_grave
	EV_NVK_DEAD_ACUTE,						// 0xfe51 dead_acute
	EV_NVK_DEAD_CIRCUMFLEX,					// 0xfe52 dead_circumfex
	EV_NVK_DEAD_TILDE,						// 0xfe53 dead_tilda
	EV_NVK_DEAD_MACRON,						// 0xfe54 dead_macron
	EV_NVK_DEAD_BREVE,						// 0xfe55 dead_breve
	EV_NVK_DEAD_ABOVEDOT,					// 0xfe56 dead_abovedot
	EV_NVK_DEAD_DIAERESIS,					// 0xfe57 dead_diaeresis
	                  0,					// 0xfe58 dead_abovering (not used)
	EV_NVK_DEAD_DOUBLEACUTE,				// 0xfe59 dead_doubleacute
	EV_NVK_DEAD_CARON,						// 0xfe5a dead_caron
	EV_NVK_DEAD_CEDILLA,					// 0xfe5b dead_cedilla
	EV_NVK_DEAD_OGONEK,						// 0xfe5c dead_ogonek
	EV_NVK_DEAD_IOTA,						// 0xfe5d dead_iota
	                               0,0,		// 0xfe5e - 0xfe5f
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfe60 - 0xfe6f
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfe70 - 0xfe7f
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfe80 - 0xfe8f
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfe90 - 0xfe9f
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfea0 - 0xfeaf
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfeb0 - 0xfebf
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfec0 - 0xfecf
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfed0 - 0xfedf
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfee0 - 0xfeef
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,		// 0xfef0 - 0xfeff
};


static bool s_isVirtualKeyCode(gint keyval)
{
	// X11 has several segregated sets of keys
	// 0xff00 - 0xffff has most function keys and non-letter keys
	// 0xfe00 - 0xfeff has additional function keys
	// 0xfd00 - 0xfdff has 3270 terminal keys
	// 0x0000 - 0x00ff latin1
	// 0x0100 - 0x01ff latin2
	// ... and so on ...
	//
	// there is also a set of Hardware Vendor ranges defined.
	// these appear to have stuff in the high word.  (see {ap_,DEC,HP,Sun}keysym.h)
	// for now, we will ignore these.

	if (keyval > 0x0000FFFF)
		return false;//was true before CJK patch
//
// Causes immediate on keypress segfault??
//	if (keyval >= GDK_KP_Space && keyval <= GDK_KP_9) // number pad keys
//		return false;
	
	if (keyval > 0xFF00)				// see the above table
		return true;
	if (keyval > 0xFE00)				// see the above table
		return true;

	UT_ASSERT(keyval <= 0xFD00);		// we don't what to do with 3270 keys
	
	if (keyval == 0x0020)				// special handling for ASCII-Space
		return true;

	return false;
}

static EV_EditBits s_mapVirtualKeyCodeToNVK(gint keyval)
{
	// map the given virtual key into a "named virtual key".
	// these are referenced by NVK_ symbol so that the cross
	// platform code can properly refer to them.

	// there is also a set of Hardware Vendor ranges defined.
	// these appear to have stuff in the high word.  (see {ap_,DEC,HP,Sun}keysym.h)
	// for now, we will ignore these.

	if (keyval >0x0000FFFF)
		return EV_NVK__IGNORE__;
	
	if (keyval > 0xFF00)
		return s_Table_NVK_0xff[keyval - 0xFF00];
	if (keyval > 0xFE00)
		return s_Table_NVK_0xfe[keyval - 0xFE00];
	
	if (keyval == 0x0020)
		return EV_NVK_SPACE;

	UT_ASSERT(0);
	return EV_NVK__IGNORE__;
}

//////////////////////////////////////////////////////////////////
// deal with keyboard mapping oddities
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/keysym.h>

static GdkModifierType s_getAltMask(void)
{
	//////////////////////////////////////////////////////////////////
	// find out what modifier mask XL_Alt_{L,R} are bound to.
	//////////////////////////////////////////////////////////////////
	
	int alt_mask = 0;

	Display * display = GDK_DISPLAY();

	KeyCode kcAltL = XKeysymToKeycode(display,XK_Alt_L);
	KeyCode kcAltR = XKeysymToKeycode(display,XK_Alt_R);

	XModifierKeymap * pModMap = XGetModifierMapping(display);
	int mkpm = pModMap->max_keypermod;
	int k,m;
	int mAltL=-1;
	int mAltR=-1;
	
	for (m=0; m<8; m++)
	{
		for (k=0; k<mkpm; k++)
		{
			KeyCode code = pModMap->modifiermap[m*mkpm + k];
			if (kcAltL && (code == kcAltL))
				mAltL = m;
			if (kcAltR && (code == kcAltR))
				mAltR = m;
		}
	}

	switch (mAltL)
	{
	default:							// Should not happen...
	case -1:							// Alt_L is not a modifier key ??
	case 0:								// Alt_L is mapped to SHIFT ??
	case 1:								// Alt_L is mapped to (Caps)LOCK ??
	case 2:								// Alt_L is mapped to CONTROL ??
		break;							// ... ignore this key.
		
	case 3: alt_mask |= GDK_MOD1_MASK; break;
	case 4: alt_mask |= GDK_MOD2_MASK; break;
	case 5: alt_mask |= GDK_MOD3_MASK; break;
	case 6: alt_mask |= GDK_MOD4_MASK; break;
	case 7: alt_mask |= GDK_MOD5_MASK; break;
	}

	switch (mAltR)
	{
	default:							// Should not happen...
	case -1:							// Alt_R is not a modifier key ??
	case 0:								// Alt_R is mapped to SHIFT ??
	case 1:								// Alt_R is mapped to (Caps)LOCK ??
	case 2:								// Alt_R is mapped to CONTROL ??
		break;							// ... ignore this key.
		
	case 3: alt_mask |= GDK_MOD1_MASK; break;
	case 4: alt_mask |= GDK_MOD2_MASK; break;
	case 5: alt_mask |= GDK_MOD3_MASK; break;
	case 6: alt_mask |= GDK_MOD4_MASK; break;
	case 7: alt_mask |= GDK_MOD5_MASK; break;
	}

	XFreeModifiermap(pModMap);

	if (!alt_mask)						// if nothing set, fall back to MOD1
		alt_mask = GDK_MOD1_MASK;
	
	//UT_DEBUGMSG(("Keycodes for alt [l 0x%x][r 0x%x] using modifiers [%d %d] yields [0x%x]\n",kcAltL,kcAltR,mAltL-2,mAltR-2,alt_mask));

	return (GdkModifierType)alt_mask;
}
