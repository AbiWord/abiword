/* AbiWord
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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#include "ut_types.h"
#include "ut_vector.h"
#include "ap_Clipboard.h"
#include "ap_UnixClipboard.h"

#include <gdk/gdk.h>

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_UnixClipboard::AP_UnixClipboard(void)
	: XAP_UnixClipboard()
{
#define AddFmt(szFormat,cf)	do { m_vecFormat.addItem(szFormat); m_vecCF.addItem((void*)cf); } while (0)

	AddFmt(AP_CLIPBOARD_ABIWORD_1,		gdk_atom_intern("application/abiword", FALSE));
	AddFmt(AP_CLIPBOARD_TEXTPLAIN_8BIT,	GDK_SELECTION_TYPE_STRING);
	AddFmt(AP_CLIPBOARD_TEXTPLAIN_UNICODE,	GDK_SELECTION_TYPE_STRING);	// probably NT only
   	AddFmt(AP_CLIPBOARD_RTF,		gdk_atom_intern("text/rtf", FALSE));
	AddFmt(AP_CLIPBOARD_IMAGE,		GDK_SELECTION_TYPE_PIXMAP);
	AddFmt(AP_CLIPBOARD_UNKNOWN,		GDK_NONE);			// must be last

	// We don't need to free these strings in our destructor
	// because we did not allocate any of the string pointers.

#undef AddFmt
}
