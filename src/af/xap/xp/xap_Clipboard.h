/* AbiSource Application Framework
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

#ifndef XAP_CLIPBOARD_H
#define XAP_CLIPBOARD_H

#define	AP_CLIPBOARD_TEXTPLAIN_8BIT 		"text-8bit"
#define	AP_CLIPBOARD_TEXTPLAIN_UNICODE 		"text-unicode"
#define AP_CLIPBOARD_RTF 					"rtf"
#define AP_CLIPBOARD_UNKNOWN 				"unknown"

#include "ut_types.h"

class AP_Clipboard
{
public:
	AP_Clipboard();
	
	virtual UT_Bool		open(void) = 0;
	virtual UT_Bool		close(void) = 0;
	virtual UT_Bool		addData(char* format, void* pData, UT_sint32 iNumBytes) = 0;
	virtual UT_sint32	getDataLen(char* format) = 0;
	virtual UT_Bool		getData(char* format, void* pData) = 0;
	virtual UT_sint32	countFormats(void) = 0;
	virtual UT_Bool		hasFormat(char* format) = 0;
	virtual char*		getNthFormat(UT_sint32 n) = 0;
	virtual UT_Bool		clear(void) = 0;

protected:
	UT_Bool	m_bOpen;
};

#endif /* XAP_CLIPBOARD_H */
