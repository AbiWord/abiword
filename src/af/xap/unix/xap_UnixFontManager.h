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

#ifndef XAP_UNIXFONTMANAGER_H
#define XAP_UNIXFONTMANAGER_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <fontconfig/fontconfig.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "xap_UnixApp.h"
#include "xap_UnixFont.h"

/*****************************************************************/

class UT_String;

class ABI_EXPORT XAP_UnixFontManager
{
public:
	XAP_UnixFontManager(void);
	~XAP_UnixFontManager(void);

	bool					scavengeFonts(void);

	UT_Vector *			    getAllFonts(void);
	XAP_UnixFont *			getDefaultFont(GR_Font::FontFamilyEnum f = GR_Font::FF_Roman);

	XAP_UnixFont *			getFont(const char * fontname,
									XAP_UnixFont::style s);

	XAP_UnixFont*			searchFont(const char* pszXftName);

	XAP_UnixFont*			findNearestFont(const char* pszFontFamily,
											const char* pszFontStyle,
											const char* pszFontVariant,
											const char* pszFontWeight,
											const char* pszFontStretch,
											const char* pszFontSize);
	
	void					unregisterFont(XAP_UnixFont * pFont);
	
	// MARCM: this should point to the only instance of XAP_UnixFontManager, 
	// so we can reach our Font Manager from a static context. Without having this static
	// member, font caching can't be implemented without a whole lot of code rewriting
	static XAP_UnixFontManager* pFontManager;

private:

	void					_addFont(XAP_UnixFont* font);
	UT_Vector				m_vecFontCache;

	UT_StringPtrMap 		m_fontHash;
	FcFontSet*		m_pFontSet;
	FcConfig*		m_pConfig;
};

#endif /* XAP_UNIXFONTMANAGER_H */

