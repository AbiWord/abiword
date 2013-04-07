/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#include <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ap_CocoaClipboard.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_CocoaClipboard::AP_CocoaClipboard()
	: XAP_CocoaClipboard()
{
#if 0
	// TODO
#define AddFmt(szFormat)															\
	do {	m_vecFormat_AP_Name.addItem((void *) szFormat);							\
			m_vecFormat_GdkAtom.addItem((void *) gdk_atom_intern(szFormat,FALSE));	\
	} while (0)

   	AddFmt(AP_CLIPBOARD_RTF);
	AddFmt(AP_CLIPBOARD_TEXTPLAIN_8BIT);
	AddFmt(AP_CLIPBOARD_STRING);		// alias for TEXTPLAIN_8BIT

	// TODO deal with multi-byte text (either unicode or utf8 or whatever)
	// TODO add something like the following.  you should be able to test
	// TODO against xemacs.
	// TODO
	// TODO AddFmt(AP_CLIPBOARD_COMPOUND_TEXT);

#undef AddFmt
#endif
}
