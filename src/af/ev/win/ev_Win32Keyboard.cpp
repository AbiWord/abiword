/* AbiSource Program Utilities
 * Copyright (c) 2002 Jordi Mas i Hernàndez - jmas@softcatala.org
 * 			 (c) 1998-2000 AbiSource, Inc.	
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

#include <windows.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_Win32OS.h"
#include "ev_Keyboard.h"
#include "ev_NamedVirtualKey.h"
#include "ev_Win32Keyboard.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "xap_EncodingManager.h"

/*
	What are we doing here
	
	Originally Abiword folks tried to handle the keyboard their "own 
	way", that included not calling TranslateMessage() and trying to
	interpret the keyboard thru WM_KEYDOWN ignoring most WM_CHAR. This 
	caused serval problems, including ALT+XXX not working and other 
	keyboard related issues.
	
	What we do now is:
	
	- Keep this code as simple as possible
	
	- We always call TranslateMessage() and we let Windows do that job,
	as all the applications do. 
	
	- We process all the WM_CHAR messages except the ones that were Abiword
	command , and we pass those to Windows 
	
	- We process only the special keys thru WM_KEYDOWN message.
	
	Jordi 10/11/2002
	
*/

#define _WIN32KEY_DEBUG 1

// I really hate doing this, but this constant is only defined with _WIN32_WINNT >=
// 0x0501, i.e., winXP, and I do want to enable the lot
#ifndef WM_UNICHAR
#define WM_UNICHAR 0x109
#endif

static EV_EditBits s_mapVirtualKeyCodeToNVK(WPARAM nVirtKey);

/*****************************************************************/
/*****************************************************************/

ev_Win32Keyboard::ev_Win32Keyboard(EV_EditEventMapper * pEEM)
	: EV_Keyboard(pEEM),
	  m_hKeyboardLayout(0),
	  m_iconv(UT_ICONV_INVALID),
	  m_bIsUnicodeInput(false),
	  m_bWasAnAbiCommand(false)
{
	HINSTANCE hInstUser = LoadLibraryW(L"USER32.DLL");
	if (hInstUser)
	{
		m_pToUnicodeEx = reinterpret_cast<int (*)(UINT,UINT,CONST PBYTE,LPWSTR,int,UINT,HKL)>
			(GetProcAddress(hInstUser, "ToUnicodeEx"));
		FreeLibrary(hInstUser);
	}
	
	remapKeyboard(GetKeyboardLayout(0));
}

ev_Win32Keyboard::~ev_Win32Keyboard()
{
	if( m_iconv != UT_ICONV_INVALID )
		UT_iconv_close( m_iconv );
}


void ev_Win32Keyboard::remapKeyboard(HKL hKeyboardLayout)
{
	char  szCodePage[16];

	if( m_iconv != UT_ICONV_INVALID )
	{
		UT_iconv_close( m_iconv );
		m_iconv = UT_ICONV_INVALID;
	}
	if( hKeyboardLayout != 0 )
	{
		strcpy( szCodePage, "CP" );
		if( GetLocaleInfoA( LOWORD( hKeyboardLayout ),
						   LOCALE_IDEFAULTANSICODEPAGE,
						   &szCodePage[2],
						   sizeof( szCodePage ) / sizeof( szCodePage[0] ) - 2 ))
		{
			// Unicode locale?
			// NT-based systems (at least XP) always produce unicode input irrespective of
			// the ANSI locale -- see WM_CHAR on MSDN and bug 9374
			// 
			// (It would be more efficient to do the NT test before calling
			// GetLocaleInfo(), but for maintanence reasons it is better here.)
			
			if( UT_IsWinNT() || !strcmp( szCodePage, "CP0" ) )
			{
				const char *szUCS2Name
					= XAP_EncodingManager::get_instance()->getNativeUnicodeEncodingName();
				
				UT_ASSERT(szUCS2Name);
				m_bIsUnicodeInput = true;
				strcpy( szCodePage, szUCS2Name );
			}
			else
				m_bIsUnicodeInput = false;

			UT_DEBUGMSG(("New keyboard codepage: %s\n",szCodePage));

			m_iconv = UT_iconv_open( "UCS-4-INTERNAL", szCodePage );
		}

		m_hKeyboardLayout = hKeyboardLayout;
	}
}

/*

	Processes WM_KEYDOWN messages related to special keys
	
*/
bool ev_Win32Keyboard::onKeyDown(AV_View * pView,
				 HWND /*hWnd*/, UINT /*iMsg*/, WPARAM nVirtKey, LPARAM keyData)
{

	m_bWasAnAbiCommand = false;
	
	EV_EditMethod * pEM;

	EV_EditModifierState ems = _getModifierState();
	EV_EditBits nvk;
	
	int						charLen;
	UT_UCSChar				charData[2];

	UT_UNUSED(keyData);
	UT_DEBUGMSG(("WIN32KEY_DEBUG->onKeyDown %x, %x\n", nVirtKey, keyData));

	// ALT key for windows {menus, ... }, ALT+XXX for special chars, etc
	if (((ems & EV_EMS_ALT) != 0) && ((ems & EV_EMS_CONTROL) == 0))
	{
		#ifdef  _WIN32KEY_DEBUG
		UT_DEBUGMSG(("WIN32KEY_DEBUG->onKeyDown return true (EV_EMS_CONTROL)\n"));
		#endif
		return true;
	}

	/*
		This is a Alt+gr combination in an international keyboard
	*/
	if (GetKeyState(VK_RMENU) & 0x8000)
	{
		#ifdef  _WIN32KEY_DEBUG
		UT_DEBUGMSG(("WIN32KEY_DEBUG->Alt+gr (EV_EMS_CONTROL)\n"));
		#endif
		return true;
	}
	
	// Get abiword keyid
	nvk = s_mapVirtualKeyCodeToNVK(nVirtKey);

	// If it is not a special key or a CTRL combination, there is nothing to do
	if (nvk == EV_NVK__IGNORE__ || (nvk == 0  && ((ems & EV_EMS_CONTROL) == 0)))
	{
		#ifdef  _WIN32KEY_DEBUG
		UT_DEBUGMSG(("WIN32KEY_DEBUG->onKeyDown return true (IGNORE)\n"));
		#endif
		return true;
	}	  		

	if (nvk != 0)
	{	// Special key
		charLen = 0;
		charData[0] = nvk;
	}
	else
	{	// Non-special key with CTRL 

#if 0
		// this causes bug 9618
		WCHAR	char_value[2];
		BYTE	keyboardState[256];

		::GetKeyboardState(keyboardState);

		// Here we pretend the CTRL key is not pressed, otherwise windows will try and convert it
		// into a control code, this is not what we want
		keyboardState[VK_CONTROL] &= 0x7F;		// mask off high bit

		if (ToAsciiEx(nVirtKey, keyData & 0x00FF0000, keyboardState, (unsigned short*) &char_value[0], 0,
					  m_hKeyboardLayout)==0)
			return true;
		charLen		= 1;
		charData[0]	= UT_UCSChar(char_value [0] & 0x000000FF);		
		charData[1]	= 0;
#else
		charLen = 1;
		charData[0] = (UT_UCS4Char)MapVirtualKeyEx(nVirtKey, 2, m_hKeyboardLayout);

		if(!charData[0]) // no mapping
			return true;
		
		charData[1]	= 0;

		if((ems & EV_EMS_SHIFT) == 0)
		{
			// shift not pressed; MapVirtualKeyEx() always returns capital letter, so we
			// have to lowercase it
			charData[0] = UT_UCS4_tolower(charData[0]);
		}
		
#endif
	}
	

	switch (m_pEEM->Keystroke(EV_EKP_PRESS | ems | charData[0], &pEM)) //#define EV_EKP_PRESS			((EV_EditKeyPress)		0x00800000)
	{
		
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:			
	case EV_EEMR_INCOMPLETE:		// a non-terminal node in state machine
		return false;		
		
	case EV_EEMR_COMPLETE:			// a terminal node in state machine			
		UT_ASSERT(pEM);			
		invokeKeyboardMethod(pView, pEM, charData, charLen);
		m_bWasAnAbiCommand = true;
		return true;
		
	default:
		UT_ASSERT(0);
		return false;
	}

	return 	true;
}

bool ev_Win32Keyboard::onIMEChar(AV_View * pView,
									HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	WCHAR b = wParam;

	// 2nd byte of MBCS is in high byte of word
	if (!m_bIsUnicodeInput && (wParam & 0xff00))
		b = ((BYTE)(wParam >> 8)) | ((BYTE)wParam << 8);
	
	_emitChar(pView,hWnd,iMsg,wParam,lParam,b,0);
	return true;
}

void ev_Win32Keyboard::_emitChar(AV_View * pView,
								 HWND /*hWnd*/, UINT iMsg, WPARAM nVirtKey, LPARAM /*keyData*/,
								 UT_uint32 b, EV_EditModifierState ems)
{
	// do the dirty work of pumping this character thru the state machine.

	UT_UCSChar charData[2];
	size_t ret;
	if( m_iconv != UT_ICONV_INVALID )
	{
		// convert to 8bit string and null terminate
		size_t len_in, len_out;
		const char *In = (const char *)&b;
		char *Out = (char *)&charData;

		// 2 bytes for Unicode and MBCS
		// 4 bytes for UNICHAR msg
		if(iMsg == WM_UNICHAR)
			len_in = 4;
		else if(m_bIsUnicodeInput || (nVirtKey & 0xff00))
			len_in = 2;
		else
			len_in = 1;
		
		len_out = sizeof(charData);

		if ((ret = UT_iconv( m_iconv, &In, &len_in, &Out, &len_out )) == -1)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		if (Out == (char *)charData)
		{
			// m_iconv is waiting for a combination keystroke. Flush the buffer
		    if ((ret = UT_iconv( m_iconv, NULL, &len_in, &Out, &len_out )) == -1)
			{
			    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
	}
	else
	{
		charData[0] = b;
		charData[1] = 0;
	}

	EV_EditMethod * pEM;
	EV_EditEventMapperResult result = m_pEEM->Keystroke(EV_EKP_PRESS|ems|charData[0],&pEM);

	switch (result)
	{
	case EV_EEMR_BOGUS_START:
		//UT_DEBUGMSG(("    Unbound StartChar: %c\n",b));
		break;

	case EV_EEMR_BOGUS_CONT:
		//UT_DEBUGMSG(("    Unbound ContChar: %c\n",b));
		break;

	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeKeyboardMethod(pView,pEM,charData,1);
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

/*	

	Processes WM_CHAR messages
	
*/
bool ev_Win32Keyboard::onChar(AV_View * pView,
								 HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData)
{
	/* 
		If the key is NOT processed as an Abiword command
		we follow their path and need to emit the char
	*/
	if (m_bWasAnAbiCommand)
	{
		#ifdef  _WIN32KEY_DEBUG
		UT_DEBUGMSG(("WIN32KEY_DEBUG->onChar return\n"));
		#endif

		// reset the command flag; see bug 8928
		m_bWasAnAbiCommand = false;
		return true;
	} 
	
	// Process the key
	_emitChar(pView,hWnd,iMsg,nVirtKey,keyData,nVirtKey,0);
	return true;

}

/*	

	Processes WM_UNICHAR messages
	The nVirtKey already contains utf-32 char, so we do not need to do any iconv translation
*/
bool ev_Win32Keyboard::onUniChar(AV_View * pView,
								 HWND /*hWnd*/, UINT /*iMsg*/, WPARAM nVirtKey, LPARAM /*keyData*/)
{
	// as WM_UNICHAR is not proceeded by WM_KEYDOWN message, we need to reset this flag here.
	m_bWasAnAbiCommand = false;
	
	EV_EditModifierState ems = _getModifierState(); 		

	EV_EditMethod * pEM;
	EV_EditEventMapperResult result = m_pEEM->Keystroke(EV_EKP_PRESS|ems|nVirtKey,&pEM);

	switch (result)
	{
		case EV_EEMR_BOGUS_START:
			//UT_DEBUGMSG(("    Unbound StartChar: %c\n",b));
			break;

		case EV_EEMR_BOGUS_CONT:
			//UT_DEBUGMSG(("    Unbound ContChar: %c\n",b));
			break;

		case EV_EEMR_COMPLETE:
			UT_ASSERT(pEM);
			invokeKeyboardMethod(pView,pEM,(UT_UCS4Char*)&nVirtKey,1);
			break;

		case EV_EEMR_INCOMPLETE:
			//MSG(keyData,(("    Non-Terminal-Char: %c\n",b)));
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
	}

	return true;

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

int ev_Win32Keyboard::_scanCodeToChars(UINT nVirtKey, UINT wScanCode, CONST PBYTE lpKeyState,
									   LPWSTR pwszBuff, int cchBuff)
{
	UT_ASSERT(m_pToUnicodeEx);
	return (*m_pToUnicodeEx)(nVirtKey,wScanCode,lpKeyState,pwszBuff,cchBuff,0,m_hKeyboardLayout);
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
	
	UT_ASSERT(nVirtKey <= G_N_ELEMENTS(s_Table_NVK));

	return s_Table_NVK[nVirtKey];
}

