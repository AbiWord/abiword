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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

#ifndef AP_WIN32CLIPBOARD_H
#define AP_WIN32CLIPBOARD_H

#include "xap_Win32Clipboard.h"

// TODO I chose "ucs2" rather than "unicode" or "16bit"
// TODO in case it might conflict with "utf8" in the future
#define	AP_CLIPBOARD_TEXTPLAIN_8BIT 		"text-8bit"
#define	AP_CLIPBOARD_TEXTPLAIN_UCS2 		"text-ucs2"
#define AP_CLIPBOARD_RTF 					"rtf"
#define AP_CLIPBOARD_BMP					"bitmap"
#define AP_CLIPBOARD_UNKNOWN 				"unknown"

class PD_Document;

class ABI_EXPORT AP_Win32Clipboard : public XAP_Win32Clipboard
{
public:
	AP_Win32Clipboard(void);
	~AP_Win32Clipboard();

	void setClipboardDoc(PD_Document * pDoc);
	PD_Document * getClipboardDoc() {return m_pClipboardDoc;}

private:
	PD_Document * m_pClipboardDoc;
};

#endif /* AP_WIN32CLIPBOARD_H */
