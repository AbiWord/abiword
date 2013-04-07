/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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
 

#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "ev_NamedVirtualKey.h"
#include "ev_CocoaKeyboard.h"
#include "xap_EncodingManager.h"

#include "ut_mbtowc.h"

//////////////////////////////////////////////////////////////////

static EV_EditBits s_mapVirtualKeyCodeToNVK(UT_uint32 keyval);
static bool s_isVirtualKeyCode(UT_uint32 keyval);

//////////////////////////////////////////////////////////////////

ev_CocoaKeyboard::ev_CocoaKeyboard(EV_EditEventMapper* pEEM)
	: EV_Keyboard(pEEM)
{
}

ev_CocoaKeyboard::~ev_CocoaKeyboard(void)
{
}


bool ev_CocoaKeyboard::_dispatchKey(AV_View * pView, UT_uint32 charData, EV_EditBits state)
{
	EV_EditMethod * pEM;
	EV_EditEventMapperResult result;
	bool retval = false;
	if (s_isVirtualKeyCode(charData))
	{
		EV_EditBits nvk = s_mapVirtualKeyCodeToNVK(charData);

		xxx_UT_DEBUGMSG(("Virtual 0x%x (ignore is 0x%x) \n", nvk, EV_NVK__IGNORE__));
		switch (nvk)
		{
		case EV_NVK__IGNORE__:
			retval = false;
			break;
		default:
			result = m_pEEM->Keystroke((UT_uint32)EV_EKP_PRESS|state|nvk,&pEM);

			switch (result)
			{
			case EV_EEMR_BOGUS_START:
				// If it is a bogus key and we don't have a sequence in
				// progress, we should let the system handle it
				// (this lets things like ALT-F4 work).
				retval = false;
				break;
			case EV_EEMR_BOGUS_CONT:
				// If it is a bogus key but in the middle of a sequence,
				// we should silently eat it (this is to prevent things
				// like Control-X ALT-F4 from killing us -- if they want
				// to kill us, fine, but they shouldn't be in the middle
				// of a sequence).
				retval = true;
				break;
			case EV_EEMR_COMPLETE:
				UT_ASSERT(pEM);
				invokeKeyboardMethod(pView,pEM,0,0); // no char data to offer
				retval = true;
				break;
			case EV_EEMR_INCOMPLETE:
				retval = true;
				break;
			default:
				UT_ASSERT(0);
				retval = true;
			}
		}
	}
	else
	{
		//printf("Real key value [%c] \n", charData);
		result = m_pEEM->Keystroke(EV_EKP_PRESS|state|charData,&pEM);

		switch (result)
		{
		case EV_EEMR_BOGUS_START:
			// If it is a bogus key and we don't have a sequence in
			// progress, we should let the system handle it
			// (this lets things like ALT-F4 work).
			retval = false;
			break;
		case EV_EEMR_BOGUS_CONT:
			// If it is a bogus key but in the middle of a sequence,
			// we should silently eat it (this is to prevent things
			// like Control-X ALT-F4 from killing us -- if they want
			// to kill us, fine, but they shouldn't be in the middle
			// of a sequence).
			retval = true;
			break;
		case EV_EEMR_COMPLETE:
			UT_ASSERT(pEM);
			invokeKeyboardMethod(pView,pEM,(UT_UCS4Char*)&charData,1); // no char data to offer
			retval = true;
			break;
		case EV_EEMR_INCOMPLETE:
			retval = true;
			break;
		default:
			UT_ASSERT(0);
			retval = true;
		}
	}
	return retval;
}


void ev_CocoaKeyboard::insertTextEvent(AV_View * pView, NSString* s)
{
	bool retval = false;
	EV_EditBits state = 0;

	int uLength = [s length];
	unichar* buffer = (unichar*)g_try_malloc(sizeof(unichar)*uLength);

	xxx_UT_DEBUGMSG(("insertTextEvent()\n"));
	[s getCharacters:buffer];
	for (int ind = 0; ind < uLength; ind++) {
		UT_uint32 charData = buffer[ind];
		retval = _dispatchKey(pView, charData, state);
	}
	FREEP(buffer);
}


bool ev_CocoaKeyboard::keyPressEvent(AV_View* pView, NSEvent* e)
{
	bool retval = false;
	EV_EditBits state = 0;

        // note that we don't usually want keyCode, but instead [e characters].
        xxx_UT_DEBUGMSG(("KeyPressEvent: keyval=%x characters=%s state=%x\n",[e keyCode],
                     [[e characters] UTF8String], state));

	unsigned int modifierFlags = [e modifierFlags];
	if (modifierFlags & NSShiftKeyMask)
		state |= EV_EMS_SHIFT;
	if (modifierFlags & NSControlKeyMask)
		state |= EV_EMS_CONTROL;
	if (modifierFlags & NSAlternateKeyMask)
		state |= EV_EMS_ALT;

	NSString *characters = [e characters];
	int uLength = [characters length];
	UT_DEBUGMSG(("num of chars %d\n", uLength));
	unichar* buffer = (unichar*)g_try_malloc(sizeof(unichar)*uLength);
	[characters getCharacters:buffer];
	for (int ind = 0; ind < uLength; ind++) {
		UT_uint32 charData = buffer[ind];
		retval = _dispatchKey (pView, charData, state);
	}
	FREEP(buffer);
	return retval;
}

/*
 These three functions all work hand in hand to get the
key events ... 
*/
static bool s_isVirtualKeyCode(UT_uint32 keyCode)
{
	// Problem: We don't get an event for tab!
	if (keyCode == 0x20 || keyCode == 0x0d || keyCode == 0x7f)
		return true;

	return (keyCode >= 0xf700 && keyCode <= 0xf8ff);
}

static EV_EditBits s_mapVirtualKeyCodeToNVK(UT_uint32 keyCode)
{
	switch (keyCode)
	{
	case 0x20:
		return EV_NVK_SPACE;
	case 0x0d:
		return EV_NVK_RETURN;
	case '\t':
		return EV_NVK_TAB;
	case 0x7f:
		return EV_NVK_BACKSPACE;
	case NSUpArrowFunctionKey:
		return EV_NVK_UP;
	case NSDownArrowFunctionKey:
		return EV_NVK_DOWN;
	case NSLeftArrowFunctionKey:
		return EV_NVK_LEFT;
	case NSRightArrowFunctionKey:
		return EV_NVK_RIGHT;
	case NSInsertFunctionKey:
		return EV_NVK_INSERT;
	case NSDeleteFunctionKey:
		return EV_NVK_DELETE;
	case NSHomeFunctionKey:
		return EV_NVK_HOME;
	case NSEndFunctionKey:
		return EV_NVK_END;
	case NSPageUpFunctionKey:
		return EV_NVK_PAGEUP;
	case NSPageDownFunctionKey:
		return EV_NVK_PAGEDOWN;
	default:
		break;
	}
	return EV_NVK__IGNORE__;
}

/*
PROBLEM: Tab doesn't work.

Thomas Schnitzer wrote:
> the default cell editing behaviour of the AppKit's table view classes is to
> start editing the next row in the table if the user finishs the current one
> by pressing return or the tab key. I just wanted to change this behaviour
> to the one used by the Finder or in the bookmark table of OmniWeb: if the
> user presses return, the editing ends and the current row stays selected. I
> wonder whether it is possible to achieve this goal with relatively simple
> intervention of the delegate (although I did not find such a simple way
> yet) or if I have to subclass the NSTableView class and override the
> -textDidEndEditing: method.

A while ago ran into the very same problem. It turned out that neither the
delegate nor the NSTableView subclass will help you in that situation.

But I found a reliable solution by subclassing NSWindow:

1. Override sendEvent to check for return/enter/tab keyDown events
2. Check if any NSTableView cell editing is currently in progress (the
   firstResponder isKindOfClass:NSText, the firstResponder's delegate
   isKindOfClass:NSTableView)
3. Force endEditing &quot;from outside&quot; by invoking
   [self makeFirstResponder:theTableView]


You may also find it useful to end the table-cell-editing when the window
resigns its keyWindow status (see the code below).


Hope this helps, Norbert

From: http://wodeveloper.com/omniLists/macosx-dev/2000/August/msg00199.html
*/
