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

#include <windows.h>
#include <richedit.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ap_Clipboard.h"
#include "ap_Win32Clipboard.h"
#include "pd_Document.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Win32Clipboard::AP_Win32Clipboard(void)
	: XAP_Win32Clipboard(), m_pClipboardDoc(NULL)
{
#define AddFmt(szFormat,cf)	do { m_vecFormat.addItem((void*)szFormat); m_vecCF.addItem((void *)cf); } while (0)

	// TODO We may want to add CF_LOCALE to supplement CF_TEXT
	AddFmt(AP_CLIPBOARD_TEXTPLAIN_8BIT,		CF_TEXT);
	AddFmt(AP_CLIPBOARD_TEXTPLAIN_UCS2,		CF_UNICODETEXT);
	AddFmt(AP_CLIPBOARD_RTF,				RegisterClipboardFormat(CF_RTF));
	AddFmt(AP_CLIPBOARD_BMP,				CF_BITMAP);					
	AddFmt(AP_CLIPBOARD_UNKNOWN,			0);					// must be last

	// We don't need to g_free these strings in our destructor
	// because we did not allocate any of the string pointers.

#undef AddFmt
}

AP_Win32Clipboard::~AP_Win32Clipboard()
{
	if(m_pClipboardDoc)
		m_pClipboardDoc->unref();
}

void AP_Win32Clipboard::setClipboardDoc(PD_Document * pDoc)
{
	if(m_pClipboardDoc)
		m_pClipboardDoc->unref();
		   
	m_pClipboardDoc = pDoc;
}

