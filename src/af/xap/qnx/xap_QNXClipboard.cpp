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

#include <string.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"

#include "xap_QNXClipboard.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_QNXClipboard::XAP_QNXClipboard(XAP_QNXApp * pQNXApp)
{
	m_pQNXApp = pQNXApp;
}

XAP_QNXClipboard::~XAP_QNXClipboard()
{
	clearClipboard();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void XAP_QNXClipboard::initialize(void)
{
   UT_DEBUGMSG(("Clipboard: initializing\n"));
   return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool XAP_QNXClipboard::addData(const char* format, void* pData, UT_sint32 iNumBytes)
{
	return XAP_FakeClipboard::addData(format, pData, iNumBytes);
}

bool XAP_QNXClipboard::hasFormat(const char *format)
{
	return XAP_FakeClipboard::hasFormat(format);
}

bool XAP_QNXClipboard::clearClipboard()
{
	return XAP_FakeClipboard::clearClipboard();
}

bool XAP_QNXClipboard::getClipboardData(const char* format, void ** ppData, UT_uint32 * pLen)
{
	return XAP_FakeClipboard::getClipboardData(format, ppData, pLen);
}
	

