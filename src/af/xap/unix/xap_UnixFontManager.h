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

#ifndef AP_UNIXFONTMANAGER_H
#define AP_UNIXFONTMANAGER_H


#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"

#include "xap_UnixFont.h"

/*****************************************************************/

class AP_UnixFontManager
{
public:
	AP_UnixFontManager(void);
	~AP_UnixFontManager(void);

	UT_Bool					setFontPath(const char * searchpath);
	UT_Bool					scavengeFonts(void);

	UT_uint32				getCount(void);

	AP_UnixFont **			getAllFonts(void);
	AP_UnixFont *			getDefaultFont(void);
	AP_UnixFont * 			getFont(const char * fontname,
									AP_UnixFont::style s);
		
protected:

	void					_allocateThisFont(const char * line,
											  const char * workingdir);
	void					_addFont(AP_UnixFont * font);

	// perhaps this should be a hash to avoid duplicates?
	UT_Vector				m_searchPaths;
	UT_HashTable 			m_fontHash;
	
};

#endif /* AP_UNIXFONTMANAGER_H */
