/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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
	The aim of this class is to provide abstraction ove ATSUI for simple purpose.
	TODO: make it part of the XP to be used form XP code for all platforms.
 */

#include "xap_MacFontManager.h"

XAP_MacFontManager::XAP_MacFontManager(void) 
	//: m_fontHash(256)
{
}

XAP_MacFontManager::~XAP_MacFontManager(void)
{
//	UT_VECTOR_PURGEALL(char *, m_searchPaths);
//	UT_HASH_PURGEDATA(XAP_UnixFont *, m_fontHash);
}


UT_uint32 XAP_MacFontManager::getCount(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
}

GR_MacFont **XAP_MacFontManager::getAllFonts(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
}

/*
	Return the default font as specified by the Appearence manager.
	
	WARNING: returned font object should be disposed by caller.
 */
GR_MacFont *XAP_MacFontManager::getDefaultFont(void)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
//	GR_MacFont *font = new GR_MacFont ();
//	return font;
}


GR_MacFont *XAP_MacFontManager::getFont(const char * fontname, ATSUStyle s)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
//	GR_MacFont *font = new GR_MacFont ();
//	return font;
}



ATSUFontID	XAP_MacFontManager::findFont (const char* pszFontFamily, 
										const char* pszFontStyle, 
										const char* pszFontVariant, 
										const char* pszFontWeight, 
										const char* pszFontStretch, 
										const float fFontSize)
{
	OSStatus	status;
    ATSUFontID	atsuiFont;
	ATSUStyle	style;
	bool		isItalic = false;
	bool		isBold	= false;
	
	status = ATSUCreateStyle (&style);
	UT_ASSERT (status == noErr);
	
	if (strcmp(pszFontStyle, "italic") == 0) {
		isItalic = true;
	}
	if (strcmp(pszFontWeight, "bold") == 0) {
		isBold = true;
	}
	
	return atsuiFont;
}


