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
 



#ifndef EV_EDITVIRTUALKEY_H
#define EV_EDITVIRTUALKEY_H

#include "ev_EditBits.h"

class EV_NamedVirtualKey
{
public:
	static const char *	getName(EV_EditBits eb);
	static EV_EditBits	getEB(const char * szName);
};

// EV_NVK_'s are named virtual keys.
// NOTE: the list below must contiguous and
// NOTE: match the list in ev_NamedVirtualKey.cpp.

#define EV_NVK_BACKSPACE	EV_NamedKey(0x0001)
#define EV_NVK_SPACE		EV_NamedKey(0x0002)
#define EV_NVK_TAB			EV_NamedKey(0x0003)
#define EV_NVK_RETURN		EV_NamedKey(0x0004)
#define EV_NVK_ESCAPE		EV_NamedKey(0x0005)
#define EV_NVK_PAGEUP		EV_NamedKey(0x0006)
#define EV_NVK_PAGEDOWN		EV_NamedKey(0x0007)
#define EV_NVK_END			EV_NamedKey(0x0008)
#define EV_NVK_HOME			EV_NamedKey(0x0009)
#define EV_NVK_LEFT			EV_NamedKey(0x000a)
#define EV_NVK_UP			EV_NamedKey(0x000b)
#define EV_NVK_RIGHT		EV_NamedKey(0x000c)
#define EV_NVK_DOWN			EV_NamedKey(0x000d)
#define EV_NVK_INSERT		EV_NamedKey(0x000e)
#define EV_NVK_DELETE		EV_NamedKey(0x000f)
#define EV_NVK_HELP			EV_NamedKey(0x0010)
#define EV_NVK_F1			EV_NamedKey(0x0011)
#define EV_NVK_F2			EV_NamedKey(0x0012)
#define EV_NVK_F3			EV_NamedKey(0x0013)
#define EV_NVK_F4			EV_NamedKey(0x0014)
#define EV_NVK_F5			EV_NamedKey(0x0015)
#define EV_NVK_F6			EV_NamedKey(0x0016)
#define EV_NVK_F7			EV_NamedKey(0x0017)
#define EV_NVK_F8			EV_NamedKey(0x0018)
#define EV_NVK_F9			EV_NamedKey(0x0019)
#define EV_NVK_F10			EV_NamedKey(0x001a)
#define EV_NVK_F11			EV_NamedKey(0x001b)
#define EV_NVK_F12			EV_NamedKey(0x001c)
#define EV_NVK_F13			EV_NamedKey(0x001d)
#define EV_NVK_F14			EV_NamedKey(0x001e)
#define EV_NVK_F15			EV_NamedKey(0x001f)
#define EV_NVK_F16			EV_NamedKey(0x0020)
#define EV_NVK_F17			EV_NamedKey(0x0021)
#define EV_NVK_F18			EV_NamedKey(0x0022)
#define EV_NVK_F19			EV_NamedKey(0x0023)
#define EV_NVK_F20			EV_NamedKey(0x0024)
#define EV_NVK_F21			EV_NamedKey(0x0025)
#define EV_NVK_F22			EV_NamedKey(0x0026)
#define EV_NVK_F23			EV_NamedKey(0x0027)
#define EV_NVK_F24			EV_NamedKey(0x0028)
#define EV_NVK_F25			EV_NamedKey(0x0029)
#define EV_NVK_F26			EV_NamedKey(0x002a)
#define EV_NVK_F27			EV_NamedKey(0x002b)
#define EV_NVK_F28			EV_NamedKey(0x002c)
#define EV_NVK_F29			EV_NamedKey(0x002d)
#define EV_NVK_F30			EV_NamedKey(0x002e)
#define EV_NVK_F31			EV_NamedKey(0x002f)
#define EV_NVK_F32			EV_NamedKey(0x0030)
#define EV_NVK_F33			EV_NamedKey(0x0031)
#define EV_NVK_F34			EV_NamedKey(0x0032)
#define EV_NVK_F35			EV_NamedKey(0x0033)


#define EV_NVK__FIRST__		EV_NVK_BACKSPACE	// must be set to first in the list
#define EV_NVK__LAST__		EV_NVK_F35			// must be set to last in the list 

#define EV_COUNT_NVK		(EV_NVK_ToNumber(EV_NVK__LAST__)+1)	// +1 to include zero which we skipped

#define EV_NVK__IGNORE__	EV_NamedKey(0xffff)



#endif /* EV_EDITVIRTUALKEY_H */
