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

#include <Clipboard.h>
#include <string.h>
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"

#include "xap_BeOSClipboard.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_BeOSClipboard::XAP_BeOSClipboard()
	: XAP_FakeClipboard()
{
	lastCopy = NULL;
}

XAP_BeOSClipboard::~XAP_BeOSClipboard()
{
	clearClipboard();
	if(lastCopy)
		free(lastCopy);
}

UT_Bool XAP_BeOSClipboard::addData(const char* format, void* pData, UT_sint32 iNumBytes)
{
	BMessage* clip = (BMessage *)NULL;
	
	if( be_clipboard->Lock() )
	{
		if((clip = be_clipboard->Data() ))
		{
			if( strcmp(format , "rtf") == 0)
			{
				clip->AddData("text/rtf" , B_MIME_TYPE , pData , iNumBytes - 1);
				be_clipboard->Commit();
				return UT_TRUE;
			}
			else if( strcmp(format , "text-8bit") == 0)
			{
				clip->AddData("text/plain" , B_MIME_TYPE , pData , iNumBytes - 1);
				be_clipboard->Commit();
				return UT_TRUE;
			}
		}
		be_clipboard->Unlock();
	}
	
	return XAP_FakeClipboard::addData(format, pData, iNumBytes);
}

UT_Bool XAP_BeOSClipboard::hasFormat(const char* format)
{
	BMessage* clip = NULL;
	char* text;
	ssize_t textSize;
	
	if(strcmp(format , "rtf") == 0)
	{
		UT_Bool hasRTF = UT_FALSE;
		
		if( be_clipboard->Lock() )
		{
			if((clip = be_clipboard->Data() ))
			{			
				if( clip->FindData("text/rtf" , B_MIME_TYPE , (const void **)&text , &textSize) == B_OK)
				{
				hasRTF = UT_TRUE;
				}
			}
			be_clipboard->Unlock();
		}
			
		return hasRTF;
	}
	
	if(strcmp(format , "text-8bit") == 0)
	{
		UT_Bool hasPlain = UT_FALSE;
		
		if( be_clipboard->Lock() )
		{
			if((clip = be_clipboard->Data() ))
			{			
				if( clip->FindData("text/plain" , B_MIME_TYPE , (const void **)&text , &textSize) == B_OK)
				{
				hasPlain = UT_TRUE;
				}
			}
			be_clipboard->Unlock();
		}
			
		return hasPlain;
	}
		
	return XAP_FakeClipboard::hasFormat(format);
}

UT_Bool XAP_BeOSClipboard::clearClipboard(void)
{
	if(be_clipboard->Lock() )
	{
		be_clipboard->Clear();
		be_clipboard->Unlock();
	}
	
	return XAP_FakeClipboard::clearClipboard();
}

UT_Bool XAP_BeOSClipboard::getClipboardData(const char * format, void ** ppData, UT_uint32 * pLen)
{
	const char* text;
	ssize_t textSize;
	
	BMessage* clip = (BMessage *)NULL;
	
	if( be_clipboard->Lock() )
	{
		if((clip = be_clipboard->Data() ))
		{
			if( strcmp(format , "rtf") == 0)
			{
				if( clip->FindData("text/rtf" , B_MIME_TYPE , (const void **)&text , &textSize) == B_OK)
				{
					if(lastCopy)
						free(lastCopy);
						
					lastCopy = (char *)malloc(textSize + 1);
					strcpy(lastCopy , text);
					lastCopy[textSize] = '\0';
					
					*ppData = (void *)lastCopy;
					*pLen = textSize;
					return UT_TRUE;
				}
				else
				{
					*pLen = 0;
				}
			}
			else if( strcmp(format , "text-8bit") == 0)
			{
				if( clip->FindData("text/plain" , B_MIME_TYPE , (const void **)&text , &textSize) == B_OK)
				{
					if(lastCopy)
						free(lastCopy);
						
					lastCopy = (char *)malloc(textSize + 1);
					strcpy(lastCopy , text);
					lastCopy[textSize] = '\0';
					
					*ppData = (void *)lastCopy;
					*pLen = textSize;
					return UT_TRUE;
				}
				else
				{
					*pLen = 0;
				}
			}
		}
		be_clipboard->Unlock();
	}
	
	return XAP_FakeClipboard::getClipboardData(format,ppData,pLen);
}

