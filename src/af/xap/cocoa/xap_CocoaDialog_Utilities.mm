/* AbiWord
 * Copyright (C) 2002 Hubert Figuiere
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




#import <Cocoa/Cocoa.h>

#include "xap_CocoaDialog_Utilities.h"

void LocalizeControl (id control, const XAP_StringSet * pSS, XAP_String_Id stringId)
{
	char buf [1024];
	_convertLabelToMac(buf, sizeof (buf), pSS->getValue(stringId));

	if ([control isKindOfClass:[NSButton class]] 
	     || [control isKindOfClass:[NSBox class]]) {
		[control setTitle:[NSString stringWithUTF8String:buf]];
	}
	else if ([control isKindOfClass:[NSTabViewItem class]]) {
		[control setLabel:[NSString stringWithUTF8String:buf]];
	}
//	else 

}


/*
	Strip the '&' et '_' from the label
	
	\param buf the result buffer
	\param bufSize the allocated size for buf
	\param label the label to convert
 */
void _convertLabelToMac (char * buf, size_t bufSize, const char * label)
{
	UT_ASSERT(label && buf);
	UT_ASSERT(strlen (label) < bufSize);

	strcpy (buf, label);

	char * src, *dst;
	src = dst = buf;
	while (*src)
	{
		*dst = *src;
		src++;
#warning I am still wondering why some string have "&" and the other have "underscore"
		if ((*dst != '&') && (*dst != '_'))
			dst++;
	}
	*dst = 0;
}
