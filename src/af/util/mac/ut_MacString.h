/* AbiSource Program Utilities
 * Copyright (C) 2000 Hubert Figuiere <hfiguiere@teaser.fr>
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
 
/*
	Mac specific utilies like Str255 to C strings conversion
*/

#ifndef __UT_MAC_STRING_H__
#define __UT_MAC_STRING_H__

#include <string.h>
#include <stdlib.h>

/* Mac headers */
#include <MacTypes.h>
#include <CFString.h>

void UT_C2PStrWithConversion (const char *inStr, StringPtr outStr, CFStringBuiltInEncodings inCharset, 
                              CFStringBuiltInEncodings outCharset);

inline void C2PStr (Str255 pString, const char * str)
{
	pString [0] = (unsigned char)strlen(str);
	memcpy (&pString[1], str, pString[0] );
}

inline void P2CStr (char * str, const Str255 pString)
{
	memcpy (str, (char*)&(pString [1]), pString[0]);
	str [pString [0]] = 0;
}

#endif
