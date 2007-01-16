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
#include <Ph.h>

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

#define Ph_CLIPBOARD_RTF "RTF"
#define Ph_CLIPBOARD_TEXT "TEXT"

bool XAP_QNXClipboard::addData(const char* format, void* pData, UT_sint32 iNumBytes)
{

	if(strcmp(Ph_CLIPBOARD_TEXT,format)==0){
		PhClipboardCopyString(PhInputGroup(0),(char*)pData);
	}
	else if(strcmp(Ph_CLIPBOARD_RTF,format)==0) {
	PhClipHeader clip = {Ph_CLIPBOARD_RTF,iNumBytes,0,pData};
	PhClipboardCopy(PhInputGroup(0),1,&clip);
	}
	else
		XAP_FakeClipboard::addData(format, pData, iNumBytes);
return true;
}

bool XAP_QNXClipboard::hasFormat(const char *format)
{
	void *cbdata;
	cbdata=PhClipboardPasteStart(PhInputGroup(0));
	if(!PhClipboardPasteType(cbdata,(char*)format)|| (XAP_FakeClipboard::hasFormat(format)==false))
	{
		PhClipboardPasteFinish(cbdata);	
		return false;
	}
PhClipboardPasteFinish(cbdata);
return true;
}

bool XAP_QNXClipboard::clearClipboard()
{
	return XAP_FakeClipboard::clearClipboard();
}

bool XAP_QNXClipboard::getClipboardData(const char* format, void ** ppData, UT_uint32 * pLen)
{
	void *cbdata;
	PhClipHeader *clip;
	cbdata=PhClipboardPasteStart(PhInputGroup(0));
	clip=PhClipboardPasteType(cbdata,(char*)format);	
if(clip){
	(char*)*ppData=g_strdup((char*)clip->data);
	*pLen=strlen((char*)*ppData);
	PhClipboardPasteFinish(cbdata);
	return true;
} 
return	XAP_FakeClipboard::getClipboardData(format, ppData, pLen);
}
