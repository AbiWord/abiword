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

#include <windows.h>
#include <stdlib.h>
#include <ctype.h>

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ev_Keyboard.h"
#include "ev_NamedVirtualKey.h"
#include "ev_Win32Keyboard.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"

#ifdef UT_DEBUG
#define MSG(keydata,args)	do { if ( ! (keyData & 0x40000000)) UT_DEBUGMSG args ; } while (0)
#else
#define MSG(keydata,args)	do { } while (0)
#endif

#define HACK_FOR_MENU

/*****************************************************************
******************************************************************
** We handle the Win32 keyboard events in a slightly non-standard
** way.  First, we do not call neither TranslateMessage() nor
** TranslateAccelerator() in our main event loop.
**
**    We will use the keybinding mechanism here to provide
**    all accelerator functionality (in conjunction with
**    our dynamic menu mechanism).  (Which will, in turn,
**    help us keep the application free of windows
**    resources and be able to localize completely from
**    an external config file.)
**
**    We let the keyboard code, below, decide if the key
**    event (WM_KEYDOWN and WM_SYSKEYDOWN messages) should
**    be processed as is -- or if it should be translated
**    to a character before being processed.  This gives
**    us the flexibility that we want, but also lets the
**    system deal with upper/lowercase issues.  And deal
**    deal with the various character locale compose
**    conventions used to enter non-7bit characters.
**
** When a WM_KEYDOWN or WM_SYSKEYDOWN occurs, we first check to
** see if the key is a "named key that we have interest in".  A
** "named key" is something like BACKSPACE, HOME, END, F2, etc.
** The "have interest in" means that we have exposed that key
** to the keybinding mechanism and allow bindings for -- for
** example, the VK_LWIN and VK_RWIN keys (the left- and right-
** flag keys on new Win95-enabled keyboards) are "named keys",
** but we don't allow access to them.
** 
**    If it is a "named key of interest", we see
**    if it is bound (either as a terminal or
**    non-terminal node in an event sequence).
**
**        If it is bound, we advance the state
**        machine.  If it is the terminal node
**        in a sequence, we invoke the method.
**
**        If it is not bound, and in the middle
**        of a sequence, we abort the state
**        machine and ignore the key.
**
**        If it is not bound and we are not in
**        the middle of a sequence, we pass it
**        on the system for normal processing
**        (TranslateMessage() and DefWindowProc()).
**        This lets the system keys (like ALT-F4)
**        do their thing.
**
**    If it is not a "named key that we have an
**    interest in" (EV_NVK__IGNORE__) we pass it
**    on to the system for normal processing
**    (TranslateMessage() and DefWindowProc()).
**    This lets the system keys (like VK_LWIN)
**    do their thing.
**
**    If it is not a "named key" (nvk==0), we ask the
**    system to translate it to a character using
**    ToAsciiEx() and the current keyboard layout.
**    Depending upon the result we either process the
**    resulting character or strip off the modifiers
**    and retranslate the key and process that.  (more
**    details are given in the function.)
**
** In a nutshell, this algorithm provides the following:
**
** [] cannot bind to named keys we don't want rebound
**    (such as VK_LWIN) either at the start of sequence
**    or in the middle.
** [] by recognizing "named" keys (such as BACKSPACE),
**    we don't have to rely on the ASCII control code
**    scheme (ie, Control-h) (and are free to use them
**    differently).
** [] keys like ALT-F4 and ALT-F are processed by us,
**    if bound, and by the system, if not.
** [] an unbound system keys (like ALT-F4) will not
**    be sent to the system in the middle of a sequence.
**    (another way of saying this is that unbound system
**    keys will only receive system processing at the
**    start of a sequence.)
** [] we mark VK_SHIFT, VK_CONTROL, and VK_MENU (alt)
**    as "not of interest" -- to prevent them from being
**    a binding rather than a modifier.
**
** We did it this way because we want to be able to freely
** use the control, alt, and shift keys as modifiers.  The
** normal Windows method of calling TranslateMessage() just
** doesn't give us the control that we want -- sometimes it
** generates a WM_CHAR and sometimes it doesn't and it is
** not possible to tell (see the differences in the return
** value documented between Win95 and WinNT).  Furthermore,
** it doesn't generate in all the different combinations --
** 'a', 'shift a', 'control a' and maybe 'alt a' get sent,
** but 'control shift a', 'alt control a', 'alt shift a',
** and 'alt control shift a' do not.  This is just unacceptable :-)
**
** The traditional method was sufficient to get alt, control
** and shift modifiers on the NamedVirtualKeys (PageDown, etc),
** but was not for letter keys which do not have a corresponding
** ASCII control character, such as '2'.  on my keyboard, 'shift 2'
** is a '@' -- we can get '2', 'control 2', 'alt 2', 'alt control 2'
** and '@', 'control @', 'alt @', and 'alt control @' -- note
** that the shift is implied in the latter set.
**
** Note: If the user presses ALT and releases it, focus moves
** to the menu bar.  We don't have control of it.  This should
** be considered a feature because it lets the user have access
** to the menu bar from the keyboard even when most of the typical
** menu accelerators are bound -- as is the case in EMACS mode.
**
******************************************************************
*****************************************************************/

// Note: these variables are static and thus shared by all
// Note: instances of this class, but then so is the physical
// Note: keyboard.

static UT_Bool s_bMapped = UT_FALSE;
static HKL s_hKeyboardLayout = 0;

static EV_EditBits s_mapVirtualKeyCodeToNVK(WPARAM nVirtKey);

/*****************************************************************/
/*****************************************************************/

ev_Win32Keyboard::ev_Win32Keyboard(EV_EditEventMapper * pEEM)
	: EV_Keyboard(pEEM)
{
	if (!s_bMapped)
		remapKeyboard(GetKeyboardLayout(0));
	s_bMapped = UT_TRUE;
}

void ev_Win32Keyboard::remapKeyboard(HKL hKeyboardLayout)
{
	s_hKeyboardLayout = hKeyboardLayout;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool ev_Win32Keyboard::onKeyDown(AV_View * pView,
									HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData)
{
	// process the keydown message.
	// return true if we handled it.
	// return false if the DefWindowProc() should be called.

	// For now, we choose to ignore the repeat count.

	//MSG(keyData,(("%s: %p %p\n",((iMsg==WM_KEYDOWN)?"onKeyDown":"onSysKeyDown"),nVirtKey,keyData)));

	///////////////////////////////////////////////////////////////////
	//
	// Try to determine the nature of the character.
	//
	// The windows keyboard driver keeps track of mappings
	// from virtual-keys to scan code and characters.  It
	// also keeps track of which keys are dead-chars (like
	// accent, grave, etc) ***and*** the set of valid chars
	// than can follow a particular dead-char.  Unfortunately,
	// it doesn't tell us what they are until after the fact.
	// That is, the output of the ToAsciiEx() function depends
	// upon what dead-chars were previously typed.  The
	// key typed immediately after a dead-char is
	// either an allowed value (and we get the proper accented
	// character) or not allowed (and we get **2** characters,
	// the accent character by itself and the second character).
	// However (by experimentation I've found that) there are
	// times that it doesn't treat the second key this way --
	// for example, when it is F3 -- BUT IT DOES KEEP THE DEAD-CHAR
	// STATE AROUND AND EVENTUALLY DRAIN IT AS EITHER AN ACCENTED
	// CHARACTER OR A 2 CHARACTER SEQUENCE -- so yes,
	// <dead-grave> <F3> <a> will cause a <F3> <a-grave>, and
	// <dead-grave> <F3> <s> will cause a <F3> <grave> <s>
	// *** I don't like this -- a dead-char is either a prefix
	// *** or it isn't -- it shouldn't just lurk...
	//
	// If we let it lurk, it means that we can't immediately map
	// the keys via our machinery -- we process the keys as we see
	// them (advancing our state-machine into the prefix state
	// when we see the dead-char).
	//
	// There's also the issue of having multiple windows.  If
	// the user hits a dead-char in one window and then raises
	// another winodw and types a character, does the dead-char
	// eval get done in the new window or is the dead-char state
	// a per-window thing (or does the system just discard the
	// dead-char).  By experimentation, it looks like the new
	// window inherits the dead-char -- as if there is only one
	// keyboard state for the entire desktop.
	// *** Likewise, I don't like this -- a dead-char should
	// *** be constrained to the window in which it was received...
	//
	// There's also the issue of hitting multiple dead-chars.  If
	// the user hits one dead-char and then another, the first one
	// is silently suppressed (and the second one remains active
	// as described above).  If the second dead-char is the same
	// key as the first, the same thing happens -- there is no
	// notion of canceling the dead-char or actually entering the
	// non-dead-char.
	// *** Likewise, I don't like this.
	//
	///////////////////////////////////////////////////////////////////
	//
	// The implications of this are:
	//
	// [] we cannot do anything with the WM_KEYDOWN for a dead-char
	//    -- we may see the second character or another window or
	//       another application may see it.
	// [] symmetrically, another window or application could get
	//    the dead-char and we only get the second char, so we
	//    cannot assume that a dead-char did not occur.
	//
	// We modify our normal processing to return immediately
	// upon receiving a dead-char (and without affecting our state
	// machine).
	//
	// Since the set of dead-chars is dependent upon the keyboard
	// layout installed and we don't have a table of what they are
	// and the ToAsciiEx() call will correctly (??) do the mapping
	// to the proper character, we cannot use the dead-char prefix
	// maps (like the X11 version does).  Therefore we need bindings
	// in the base map for all of the accented characters (whereas
	// the X11 version uses the prefix maps).
	//
	// But, before we check for dead-chars, we route it thru the NVK
	// tables (since dead-chars come in as regular VK_ keysyms but
	// with hidden meaning (unlike on X11)).
	//
	///////////////////////////////////////////////////////////////////
	//
	// The windows keyboard layout mechanism also treats Control+Alt
	// as AltGr and Shift+Control+Alt as Shift+AltGr -- two special
	// sequence to get another range of chars.  Note, some layouts
	// only define AltGr while other define both.  (See the German
	// and the US-International keyboard layouts.)
	//
	// BUGBUG We have a slight discrepancy with other apps (like MSWord)
	// BUGBUG where a AltGr (Control+Alt) on a dead-char which is only
	// BUGBUG a dead-char in the non-AltGr state (see [`/~] on the ENUS
	// BUGBUG International layout) still acts like a dead-char.  That
	// BUGBUG is: "Control+Alt+grave a" yields "agrave" just like the
	// BUGBUG sequence "grave a".  Whereas, MSWord, you just get "a".
	// BUGBUG I don't care.
	//
	///////////////////////////////////////////////////////////////////
	
	EV_EditMethod * pEM;
	EV_EditEventMapperResult result;

	EV_EditModifierState ems = _getModifierState();
	EV_EditBits nvk = s_mapVirtualKeyCodeToNVK(nVirtKey);

	if (nvk == EV_NVK__IGNORE__)
	{
		// if a named-virtual-key that we don't have
		// an interest in (must be ignored), give it
		// the traditional processing.
		// note: translating it is probably unnecessary,
		// note: but we must run it thru DefWindowProc().

		//MSG(keyData,(("    NVK_Ignore: %p\n",nVirtKey)));
		_translateMessage(hWnd,iMsg,nVirtKey,keyData);
		return UT_FALSE;
	}

	if (nvk != 0)
	{
		// a named-virtual-key that we have an interest in.
		
		//MSG(keyData,(("    NVK: %s (%s %s %s)\n",EV_NamedVirtualKey::getName(nvk),
		//			  (ems&EV_EMS_SHIFT)?"shift":"",
		//			  (ems&EV_EMS_CONTROL)?"control":"",
		//			  (ems&EV_EMS_ALT)?"alt":"")));

		result = m_pEEM->Keystroke(EV_EKP_PRESS|ems|nvk,&pEM);
		switch (result)
		{
		case EV_EEMR_BOGUS_START:
			// If it is a bogus key and we don't have a sequence in
			// progress, we should let the system handle it
			// (this lets things like ALT-F4 work).
			_translateMessage(hWnd,iMsg,nVirtKey,keyData);
			return UT_FALSE;

		case EV_EEMR_BOGUS_CONT:
			// If it is a bogus key but in the middle of a sequence,
			// we should silently eat it (this is to prevent things
			// like Control-X ALT-F4 from killing us -- if they want
			// to kill us, fine, but they shouldn't be in the middle
			// of a sequence).
			return UT_TRUE;

		case EV_EEMR_COMPLETE:			// a terminal node in state machine
			UT_ASSERT(pEM);
			invokeKeyboardMethod(pView,pEM,0,0); // no char data to offer
			return UT_TRUE;

		case EV_EEMR_INCOMPLETE:		// a non-terminal node in state machine
			return UT_TRUE;

		default:
			UT_ASSERT(0);
			return UT_TRUE;
		}
	}

	// an unnamed-virtual-key -- a character key possibly with some sugar on it.

	BYTE keyState[256];
	BYTE buffer[2];

	unsigned int scancode = (keyData & 0x00ff0000)>>16;
	GetKeyboardState(keyState);
	int count = ToAsciiEx(nVirtKey,scancode,keyState,(WORD *)buffer,0,s_hKeyboardLayout);

	if (count == -1)
	{
		// a possible dead-char -- ignore it and wait for possible completed sequence.
		//UT_DEBUGMSG(("    Received possible dead-char: %x\n",nVirtKey));
		return UT_TRUE;
	}
		
	if (count == 0)
	{
		// this key combination (this vk and whatever modifiers
		// are already down) doesn't produce a character (not all
		// AltGr or Shift+AltGr key do).
		//
		// here we need some trickery: strip off Control+Alt and let
		// the system re-interpret the vk.  take that result and
		// use the Control+Alt settings in our tables.

		//UT_DEBUGMSG(("    Received non-character-producing key: %x\n",nVirtKey));
		EV_EditModifierState ems2 = ems & (EV_EMS_CONTROL|EV_EMS_ALT);

		keyState[VK_CONTROL] &= ~0x80;
		keyState[VK_MENU] &= ~0x80;
		keyState[VK_LCONTROL] &= ~0x80;
		keyState[VK_LMENU] &= ~0x80;
		keyState[VK_RCONTROL] &= ~0x80;
		keyState[VK_RMENU] &= ~0x80;

		count = ToAsciiEx(nVirtKey,scancode,keyState,(WORD *)buffer,0,s_hKeyboardLayout);

		if (count == 1)
		{
			//UT_DEBUGMSG(("        Remapped char to: %c (%s %s)\n",
			//			 buffer[0],
			//			 ((ems&EV_EMS_CONTROL)?"control":""),
			//			 ((ems&EV_EMS_ALT)?"alt":"")));

			_emitChar(pView,hWnd,iMsg,nVirtKey,keyData,buffer[0],ems2);
		}
		else
		{
			//UT_DEBUGMSG(("        Could not decode char stripped of (%s %s) [result %d], ignoring.\n",
			//			 ((ems&EV_EMS_CONTROL)?"control":""),
			//			 ((ems&EV_EMS_ALT)?"alt":""),
			//			 count));
		}
		return UT_TRUE;
	}
	
	if (count == 2)
	{
		// a [dead-char, invalid-char] sequence, emit the dead-char
		// as a plain char and then decide what to do with the other.
		// Shift-state in the dead-char has already been compensated for.
		// we exclude Control and Alt from consideration because we don't
		// have the settings for when the dead-char was pressed -- we only
		// have the settings as of the second character.
			
		//UT_DEBUGMSG(("    Emitting dead-char as plain char: %c\n",buffer[0]));
		_emitChar(pView,hWnd,iMsg,nVirtKey,keyData,buffer[0],0);
		buffer[0] = buffer[1];
	}
	
	// try to handle the remaining (or only) character.

	if (buffer[0] >= 0x20)
	{
		// a normal character
		//
		// if AltGr or Shift+AltGr were down and we got a character,
		// then we probably don't want to use the Control+Alt in our
		// state machine, so we don't pass "ems" in the call to _emitChar().
		// we don't care about SHIFT, since it's already accounted for
		// in the character.
		//
		// we allow ALT (without CONTROL) to go thru so that the menu
		// will work.

		EV_EditModifierState ems2 = ems & (EV_EMS_CONTROL|EV_EMS_ALT);
		if (ems2 == (EV_EMS_CONTROL|EV_EMS_ALT))
			ems2 = 0;
		
		_emitChar(pView,hWnd,iMsg,nVirtKey,keyData,buffer[0],ems2);
		return UT_TRUE;
	}
	else
	{
		// windows maps control-a and friends to [0x00 - 0x1f].
		// we want control characters to appear as the actual
		// character and a control bit set in the ems.

		EV_EditModifierState ems2 = ems & (EV_EMS_CONTROL|EV_EMS_ALT);

		keyState[VK_CONTROL] &= ~0x80;
		keyState[VK_MENU] &= ~0x80;
		keyState[VK_LCONTROL] &= ~0x80;
		keyState[VK_LMENU] &= ~0x80;
		keyState[VK_RCONTROL] &= ~0x80;
		keyState[VK_RMENU] &= ~0x80;
		
		count = ToAsciiEx(nVirtKey,scancode,keyState,(WORD *)buffer,0,s_hKeyboardLayout);

		if (count == 1)
		{
			//UT_DEBUGMSG(("        Remapped ControlChar to: %c (%s %s)\n",
			//			 buffer[0],
			//			 ((ems2&EV_EMS_CONTROL)?"control":""),
			//			 ((ems2&EV_EMS_ALT)?"alt":"")));

			_emitChar(pView,hWnd,iMsg,nVirtKey,keyData,buffer[0],ems2);
		}
		else
		{
			//UT_DEBUGMSG(("        Could not decode ControlChar stripped of (%s %s) [result %d], ignoring.\n",
			//			 ((ems2&EV_EMS_CONTROL)?"control":""),
			//			 ((ems2&EV_EMS_ALT)?"alt":""),
			//			 count));
		}
		return UT_TRUE;
	}
}

void ev_Win32Keyboard::_emitChar(AV_View * pView,
								 HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData,
								 BYTE b, EV_EditModifierState ems)
{
	// do the dirty work of pumping this character thru the state machine.

	//UT_DEBUGMSG(("        Char: %c (%s %s %s)\n",b,
	//			 (ems&EV_EMS_SHIFT)?"shift":"",
	//			 (ems&EV_EMS_CONTROL)?"control":"",
	//			 (ems&EV_EMS_ALT)?"alt":""));

	UT_uint16 charData = (UT_uint16)b;

	EV_EditMethod * pEM;
	EV_EditEventMapperResult result = m_pEEM->Keystroke(EV_EKP_PRESS|ems|charData,&pEM);

	switch (result)
	{
	case EV_EEMR_BOGUS_START:
		//UT_DEBUGMSG(("    Unbound StartChar: %c\n",b));
#ifdef HACK_FOR_MENU
		// Let Alt+F and friends raise menus.
		if ((ems & (EV_EMS_CONTROL|EV_EMS_ALT)) == EV_EMS_ALT)
			_translateMessage(hWnd,iMsg,nVirtKey,keyData);
#endif
		break;

	case EV_EEMR_BOGUS_CONT:
		//UT_DEBUGMSG(("    Unbound ContChar: %c\n",b));
		break;

	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeKeyboardMethod(pView,pEM,&charData,1);
		break;

	case EV_EEMR_INCOMPLETE:
		//MSG(keyData,(("    Non-Terminal-Char: %c\n",b)));
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return;
}


UT_Bool ev_Win32Keyboard::onChar(AV_View * pView,
								 HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData)
{
	// process the char message.  since we have taken care of everything
	// on the KeyDown, we have nothing to do here.

	//MSG(keyData,(("%s: %p %p\n",((iMsg==WM_CHAR)?"wm_char":"wm_syschar"),nVirtKey,keyData)));

	UT_Bool bIgnoreCharacter = UT_TRUE;
#ifdef HACK_FOR_MENU
	// see the note in KeyDown.  we need this to get the system to
	// process the key and do the menu activation thing.
	bIgnoreCharacter = UT_FALSE;
#endif
	return bIgnoreCharacter;
}

/*****************************************************************/
/*****************************************************************/

EV_EditBits ev_Win32Keyboard::_getModifierState(void)
{
	EV_EditBits eb = 0;

	if (GetKeyState(VK_SHIFT) & 0x8000)
		eb |= EV_EMS_SHIFT;
	if (GetKeyState(VK_CONTROL) & 0x8000)
		eb |= EV_EMS_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		eb |= EV_EMS_ALT;

	return eb;
}

void ev_Win32Keyboard::_translateMessage(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	MSG new_msg;

	new_msg.hwnd	= hwnd;
	new_msg.message = iMsg;
	new_msg.wParam = wParam;
	new_msg.lParam = lParam;
	new_msg.time = GetMessageTime();
	new_msg.pt.x = 0;
	new_msg.pt.y = 0;
	TranslateMessage(&new_msg);
}

/*****************************************************************/
/*****************************************************************/

const static EV_EditBits s_Table_NVK[] =
{	0,					/*                   0x00 */
	EV_NVK__IGNORE__,	/* VK_LBUTTON        0x01 */
	EV_NVK__IGNORE__,	/* VK_RBUTTON        0x02 */
	EV_NVK__IGNORE__,	/* VK_CANCEL         0x03 */
	EV_NVK__IGNORE__,	/* VK_MBUTTON        0x04 */
	0,					/*                   0x05 */
	0,					/*                   0x06 */
	0,					/*                   0x07 */
	EV_NVK_BACKSPACE,	/* VK_BACK           0x08 */
	EV_NVK_TAB,			/* VK_TAB            0x09 */
	0,					/*                   0x0A */
	0,					/*                   0x0B */
	EV_NVK__IGNORE__,	/* VK_CLEAR          0x0C */
	EV_NVK_RETURN,		/* VK_RETURN         0x0D */
	0,					/*                   0x0E */
	0,					/*                   0x0F */
	EV_NVK__IGNORE__,	/* VK_SHIFT          0x10 */
	EV_NVK__IGNORE__,	/* VK_CONTROL        0x11 */
	EV_NVK__IGNORE__,	/* VK_MENU           0x12 */
	EV_NVK__IGNORE__,	/* VK_PAUSE          0x13 */
	EV_NVK__IGNORE__,	/* VK_CAPITAL        0x14 */
	0,					/*                   0x15 */
	0,					/*                   0x16 */
	0,					/*                   0x17 */
	0,					/*                   0x18 */
	0,					/*                   0x19 */
	0,					/*                   0x1A */
	EV_NVK_ESCAPE,		/* VK_ESCAPE         0x1B */
	0,					/*                   0x1C */
	0,					/*                   0x1D */
	0,					/*                   0x1E */
	0,					/*                   0x1F */
	EV_NVK_SPACE,		/* VK_SPACE          0x20 */
	EV_NVK_PAGEUP,		/* VK_PRIOR          0x21 */
	EV_NVK_PAGEDOWN,	/* VK_NEXT           0x22 */
	EV_NVK_END,			/* VK_END            0x23 */
	EV_NVK_HOME,		/* VK_HOME           0x24 */
	EV_NVK_LEFT,		/* VK_LEFT           0x25 */
	EV_NVK_UP,			/* VK_UP             0x26 */
	EV_NVK_RIGHT,		/* VK_RIGHT          0x27 */
	EV_NVK_DOWN,		/* VK_DOWN           0x28 */
	EV_NVK__IGNORE__,	/* VK_SELECT         0x29 */
	EV_NVK__IGNORE__,	/* VK_PRINT          0x2A */
	EV_NVK__IGNORE__,	/* VK_EXECUTE        0x2B */
	EV_NVK__IGNORE__,	/* VK_SNAPSHOT       0x2C */
	EV_NVK_INSERT,		/* VK_INSERT         0x2D */
	EV_NVK_DELETE,		/* VK_DELETE         0x2E */
	EV_NVK_HELP,		/* VK_HELP           0x2F */
	0,0,0,0,0,0,0,0,0,0,				/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
	                    0,0,0,0,0,0,	/* 0x3a -- 0x3f */
	0,									/* 0x40 */
	  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* VK_A -- */
	0,0,0,0,0,0,0,0,0,0,0,				/*      -- VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */
	EV_NVK__IGNORE__,	/* VK_LWIN           0x5B (the left flag button on Win95 keyboards) */
	EV_NVK__IGNORE__,	/* VK_RWIN           0x5C (the right flag button on Win95 keyboards) */
	EV_NVK_MENU_SHORTCUT,	/* VK_APPS		 0x5D (button with a drop-down-menu on newer Win95 keyboards) */
	0,					/*                   0x5E */
	0,					/*                   0x5F */
	0,					/* VK_NUMPAD0        0x60 */
	0,					/* VK_NUMPAD1        0x61 */
	0,					/* VK_NUMPAD2        0x62 */
	0,					/* VK_NUMPAD3        0x63 */
	0,					/* VK_NUMPAD4        0x64 */
	0,					/* VK_NUMPAD5        0x65 */
	0,					/* VK_NUMPAD6        0x66 */
	0,					/* VK_NUMPAD7        0x67 */
	0,					/* VK_NUMPAD8        0x68 */
	0,					/* VK_NUMPAD9        0x69 */
	0,					/* VK_MULTIPLY       0x6A */
	0,					/* VK_ADD            0x6B */
	0,					/* VK_SEPARATOR      0x6C */
	0,					/* VK_SUBTRACT       0x6D */
	0,					/* VK_DECIMAL        0x6E */
	0,					/* VK_DIVIDE         0x6F */
	EV_NVK_F1,			/* VK_F1             0x70 */
	EV_NVK_F2,			/* VK_F2             0x71 */
	EV_NVK_F3,			/* VK_F3             0x72 */
	EV_NVK_F4,			/* VK_F4             0x73 */
	EV_NVK_F5,			/* VK_F5             0x74 */
	EV_NVK_F6,			/* VK_F6             0x75 */
	EV_NVK_F7,			/* VK_F7             0x76 */
	EV_NVK_F8,			/* VK_F8             0x77 */
	EV_NVK_F9,			/* VK_F9             0x78 */
	EV_NVK_F10,			/* VK_F10            0x79 */
	EV_NVK_F11,			/* VK_F11            0x7A */
	EV_NVK_F12,			/* VK_F12            0x7B */
	EV_NVK_F13,			/* VK_F13            0x7C */
	EV_NVK_F14,			/* VK_F14            0x7D */
	EV_NVK_F15,			/* VK_F15            0x7E */
	EV_NVK_F16,			/* VK_F16            0x7F */
	EV_NVK_F17,			/* VK_F17            0x80 */
	EV_NVK_F18,			/* VK_F18            0x81 */
	EV_NVK_F19,			/* VK_F19            0x82 */
	EV_NVK_F20,			/* VK_F20            0x83 */
	EV_NVK_F21,			/* VK_F21            0x84 */
	EV_NVK_F22,			/* VK_F22            0x85 */
	EV_NVK_F23,			/* VK_F23            0x86 */
	EV_NVK_F24,			/* VK_F24            0x87 */
	0,					/*                   0x88 */
	0,					/*                   0x89 */
	0,					/*                   0x8A */
	0,					/*                   0x8B */
	0,					/*                   0x8C */
	0,					/*                   0x8D */
	0,					/*                   0x8E */
	0,					/*                   0x8F */
	EV_NVK__IGNORE__,	/* VK_NUMLOCK        0x90 */
	EV_NVK__IGNORE__,	/* VK_SCROLL         0x91 */
	0,					/*                   0x92 */
	0,					/*                   0x93 */
	0,					/*                   0x94 */
	0,					/*                   0x95 */
	0,					/*                   0x96 */
	0,					/*                   0x97 */
	0,					/*                   0x98 */
	0,					/*                   0x99 */
	0,					/*                   0x9A */
	0,					/*                   0x9B */
	0,					/*                   0x9C */
	0,					/*                   0x9D */
	0,					/*                   0x9E */
	0,					/*                   0x9F */
	EV_NVK__IGNORE__,	/* VK_LSHIFT         0xA0 (not sent) */
	EV_NVK__IGNORE__,	/* VK_RSHIFT         0xA1 (not sent) */
	EV_NVK__IGNORE__,	/* VK_LCONTROL       0xA2 (not sent) */
	EV_NVK__IGNORE__,	/* VK_RCONTROL       0xA3 (not sent) */
	EV_NVK__IGNORE__,	/* VK_LMENU          0xA4 (not sent) */
	EV_NVK__IGNORE__,	/* VK_RMENU          0xA5 (not sent) */
	            0,0,0,0,0,0,0,0,0,0,			/* 0xA6 - 0xAF */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			/* 0xB0 - 0xBF */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			/* 0xC0 - 0xCF */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			/* 0xD0 - 0xDF */
	0,0,0,0,0,									/* 0xE0 - 0xE4 */
	EV_NVK__IGNORE__,	/* VK_PROCESSKEY     0xE5 */
	            0,0,0,0,0,0,0,0,0,0,			/* 0xE6 - 0xEF */
	0,0,0,0,0,0,								/* 0xF0 - 0xF5 */
	EV_NVK__IGNORE__,	/* VK_ATTN           0xF6 */
	EV_NVK__IGNORE__,	/* VK_CRSEL          0xF7 */
	EV_NVK__IGNORE__,	/* VK_EXSEL          0xF8 */
	EV_NVK__IGNORE__,	/* VK_EREOF          0xF9 */
	EV_NVK__IGNORE__,	/* VK_PLAY           0xFA */
	EV_NVK__IGNORE__,	/* VK_ZOOM           0xFB */
	EV_NVK__IGNORE__,	/* VK_NONAME         0xFC */
	EV_NVK__IGNORE__,	/* VK_PA1            0xFD */
	EV_NVK__IGNORE__,	/* VK_OEM_CLEAR      0xFE */
	0					/*                   0xFF */
};

static EV_EditBits s_mapVirtualKeyCodeToNVK(WPARAM nVirtKey)
{
	// map the given virtual key into a "named virtual key".
	// these are referenced by NVK_ symbol so that the cross
	// platform code can properly refer to them.
	
	UT_ASSERT(nVirtKey <= NrElements(s_Table_NVK));

	return s_Table_NVK[nVirtKey];
}

