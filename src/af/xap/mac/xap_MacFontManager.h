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


#ifndef __XAP_MACFONTMANAGER_H__
#define __XAP_MACFONTMANAGER_H__

#include <ATSUnicode.h>
#include <Appearance.h>

#include "gr_MacFont.h"


class XAP_MacFontManager
{
public:
	XAP_MacFontManager(void);
	~XAP_MacFontManager(void);

//	bool					setFontPath(const char * searchpath);
//	bool					scavengeFonts(void);

	UT_uint32				getCount(void);

	GR_MacFont **			getAllFonts(void);
	GR_MacFont *			getDefaultFont(void);
	GR_MacFont *			getFont(const char * fontname, ATSUStyle s);
	ATSUStyle			findFont (const char* pszFontFamily, 
										const char* pszFontStyle, 
										const char* pszFontVariant, 
										const char* pszFontWeight, 
										const char* pszFontStretch, 
										const float fFontSize);
	OSStatus _makeThemeATSUIStyle(ThemeFontID themeFontID, ATSUStyle *theStyle);
protected:

//	void					_allocateThisFont(const char * line,
//	    						                              const char * workingdir, int iLine);
//	void 				_allocateCJKFont(const char * line, int iLine);
//	void					_addFont(XAP_UnixFont * font);

	// perhaps this should be a hash to avoid duplicates?
//	UT_Vector				m_searchPaths;

//	UT_HashTable 			m_fontHash;
 private:

};


#endif
