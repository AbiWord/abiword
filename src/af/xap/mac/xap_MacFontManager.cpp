/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
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
#include "ut_debugmsg.h"
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



ATSUStyle  XAP_MacFontManager::findFont (const char* pszFontFamily, 
										const char* pszFontStyle, 
										const char* pszFontVariant, 
										const char* pszFontWeight, 
										const char* pszFontStretch, 
										const float fFontSize)
{
	OSStatus	status;
	ATSUStyle	style;
	bool		isItalic = false;
	bool		isBold	= false;
	
	UT_DEBUGMSG (("findFont not implemented !!\n"));
	status = ATSUCreateStyle (&style);
	UT_ASSERT (status == noErr);
	
	if (strcmp(pszFontStyle, "italic") == 0) {
		isItalic = true;
	}
	if (strcmp(pszFontWeight, "bold") == 0) {
		isBold = true;
	}
	
	return style;
}




/* MakeThemeATSUIStyle creates a simple ATSUI style record
    that based on the current theme font ID that can be used in
    calls to the ATSUI text drawing routines. */ 
/* see http://developer.apple.com/qa/qa2001/qa1027.html for me reference */
OSStatus XAP_MacFontManager::_makeThemeATSUIStyle(ThemeFontID themeFontID,
												  ATSUStyle *theStyle) 
{
	OSStatus err;
	ATSUStyle localStyle;
	ATSUFontID atsuFont;
	Fixed atsuSize;
	short atsuOrientation, fontFamily, fontSize;
	Str255 fontName;
	Style fontStyle;
	Boolean trueV = true, falseV = false;
    
	/* Three parrallel arrays for setting up attributes. */
	ATSUAttributeTag theTags[] = {
		kATSUFontTag, kATSUSizeTag, kATSUVerticalCharacterTag,
		kATSUQDBoldfaceTag, kATSUQDItalicTag, kATSUQDUnderlineTag, 
		kATSUQDCondensedTag, kATSUQDExtendedTag
	};
	ByteCount theSizes[] = {
		sizeof(ATSUFontID), sizeof(Fixed), sizeof(UInt16),
		sizeof(Boolean), sizeof(Boolean), sizeof(Boolean),
		sizeof(Boolean), sizeof(Boolean)
	};
	ATSUAttributeValuePtr theValues[] = {
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL
	};
	
	/* set up locals */
	localStyle = NULL;
	atsuFont = 0;
	atsuSize = 0x00080000;
	atsuOrientation = kATSUStronglyHorizontal;
	/* or atsuOrientation = kATSUStronglyVertical */
    
	/* calculate the theme font parameters */
	err = GetThemeFont( themeFontID, smSystemScript,
						fontName,  &fontSize, &fontStyle);
	if (err != noErr) goto bail;
	atsuSize = FixRatio(fontSize, 1);
    
	/* set the values array to point to our locals */
	theValues[0] = &atsuFont;
	theValues[1] = &atsuSize;
	theValues[2] = &atsuOrientation;
	theValues[3] = ((fontStyle & bold) != 0 ? &trueV : &falseV);
	theValues[4] = ((fontStyle & italic) != 0 ? &trueV : &falseV);
	theValues[5] = ((fontStyle & underline) != 0 ? &trueV : &falseV);
	theValues[6] = ((fontStyle & condense) != 0 ? &trueV : &falseV);
	theValues[7] = ((fontStyle & extend) != 0 ? &trueV : &falseV);
    
	/* calculate the font ID */
	GetFNum( fontName, &fontFamily);
	err = ATSUFONDtoFontID( fontFamily, fontStyle, &atsuFont);
	if (err != noErr) goto bail;
    
	/* find the font ID */
	err = ATSUFindFontFromName((Ptr)fontName+1, (long)fontName[0],
							   kFontFullName, kFontMacintoshPlatform,
							   kFontRomanScript, kFontNoLanguage, &atsuFont);
	if (err != noErr) 
		goto bail;
    
	/* create a style */
	err = ATSUCreateStyle(&localStyle);
	if (err != noErr) goto bail;
    
	/* set the style attributes */  
	err = ATSUSetAttributes( localStyle,
							 sizeof(theTags)/sizeof(theTags[0]), 
							 theTags, theSizes, theValues );
	if (err != noErr) 
		goto bail;
    
	/* store the new style for the caller */    
	*theStyle = localStyle;
	return noErr;
 bail:
	if (localStyle != NULL) ATSUDisposeStyle(localStyle);
	return err;
}






