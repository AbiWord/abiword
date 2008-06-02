/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#include <stdio.h>
#include <string.h>
#include <X11/keysym.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "ev_NamedVirtualKey.h"
#include "ev_X11Keyboard.h"
#include "ut_mbtowc.h"
#include "ut_string_class.h"

/*!
 * deal with keyboard mapping oddities
 */
GdkModifierType EV_X11Keyboard::getAltModifierMask(void)
{
printf ("EV_X11Keyboard::getAltModifierMask()\n");
	return static_cast<GdkModifierType>(m_altMask);
}

EV_X11Keyboard::EV_X11Keyboard(EV_EditEventMapper* pEEM)
	: EV_UnixKeyboard(pEEM)
{
	//////////////////////////////////////////////////////////////////
	// find out what modifier mask XL_Alt_{L,R} are bound to.
	//////////////////////////////////////////////////////////////////
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

	case 3: m_altMask |= GDK_MOD1_MASK; break;
	case 4: m_altMask |= GDK_MOD2_MASK; break;
	case 5: m_altMask |= GDK_MOD3_MASK; break;
	case 6: m_altMask |= GDK_MOD4_MASK; break;
	case 7: m_altMask |= GDK_MOD5_MASK; break;
	}

	switch (mAltR)
	{
	default:							// Should not happen...
	case -1:							// Alt_R is not a modifier key ??
	case 0:								// Alt_R is mapped to SHIFT ??
	case 1:								// Alt_R is mapped to (Caps)LOCK ??
	case 2:								// Alt_R is mapped to CONTROL ??
		break;							// ... ignore this key.

	case 3: m_altMask |= GDK_MOD1_MASK; break;
	case 4: m_altMask |= GDK_MOD2_MASK; break;
	case 5: m_altMask |= GDK_MOD3_MASK; break;
	case 6: m_altMask |= GDK_MOD4_MASK; break;
	case 7: m_altMask |= GDK_MOD5_MASK; break;
	}

	XFreeModifiermap(pModMap);

	if (!m_altMask)						// if nothing set, fall back to MOD1
		m_altMask = GDK_MOD1_MASK;
}

EV_X11Keyboard::~EV_X11Keyboard(void)
{
}

bool EV_X11Keyboard::keyPressEvent(AV_View* pView, GdkEventKey* e)
{
	EV_EditBits state = 0;
	EV_EditEventMapperResult result;
	EV_EditMethod * pEM;

	UT_uint32 charData = e->keyval;

	if (e->state & GDK_SHIFT_MASK)
		state |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
	{
		printf("%d:%d: control mask\n", __FILE__, __LINE__);
		state |= EV_EMS_CONTROL;

		// Gdk does us the favour of working out a translated keyvalue for us,
		// but with the Ctrl keys, we do not want that -- see bug 9545
		Display * display = GDK_DISPLAY();
		KeySym sym = XKeycodeToKeysym(display,
									  e->hardware_keycode,
									  e->state & GDK_SHIFT_MASK ? 1 : 0);
		printf("EV_X11Keyboard::keyPressEvent: keyval %d, hardware_keycode %d\n"
					 "                                sym: 0x%x\n",
					 e->keyval, e->hardware_keycode, sym);

		charData = sym;
	}
	if (e->state & (m_altMask))
		state |= EV_EMS_ALT;

	if (isVirtualKeyCode(charData))
	{
		EV_EditBits nvk = mapVirtualKeyCodeToNVK(charData);

		switch (nvk)
		{
		case EV_NVK__IGNORE__:
			return false;
		default:

			result = m_pEEM->Keystroke(static_cast<UT_uint32>(EV_EKP_PRESS|state|nvk),&pEM);

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
	    // TODO: is this necessary?
	    charData = gdk_keyval_to_unicode (charData);
	    UT_UTF8String utf8 (static_cast<const UT_UCS4Char *>(&charData), 1);
	    return charDataEvent (pView, state, utf8.utf8_str(), utf8.byteLength());
	  }

	return false;
}

