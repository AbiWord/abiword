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
#include "ap_QNXClipboard.h"


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_QNXClipboard::AP_QNXClipboard(AP_QNXApp * pApp)
	: XAP_QNXClipboard((XAP_QNXApp *)(pApp))
{
#if 0
#define AddFmt(szFormat)															\
	do {	m_vecFormat_AP_Name.addItem((void *) szFormat);							\
			m_vecFormat_GdkAtom.addItem((void *) gdk_atom_intern(szFormat, UT_FALSE));	\
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
