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
#include <stdlib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "xap_UnixFontManager.h"

#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

AP_UnixFontManager::AP_UnixFontManager(void)
{
}

AP_UnixFontManager::~AP_UnixFontManager(void)
{
	UT_VECTOR_PURGEALL(char *, m_vSearchPaths);
	UT_VECTOR_PURGEALL(AP_UnixFont *, m_vFonts);
}

UT_Bool AP_UnixFontManager::setFontPath(const char * searchpath)
{
	char ** pstring = (char **) &searchpath;
	char * token = NULL;

	while ((token = strsep(pstring, ";")))
		m_vSearchPaths.addItem(token);

	// TODO should we be checking each path to see if it's good?
	
	return UT_TRUE;
}

UT_Bool AP_UnixFontManager::scavengeFonts(void)
{
	// TODO do lots of work finding and reading
	// the fonts.dir files to get the data we
	// pass when we open the fonts we create below.

	// for testing now, we just add a fake font or two
	AP_UnixFont * b = new AP_UnixFont;
	if (b->openFileAs("century schoolbook", "../../lib/unix/fonts/c059013l.pfa",
					  "../../lib/unix/fonts/c059013l.afm", AP_UnixFont::STYLE_NORMAL))
		_addFont(b);

	b = new AP_UnixFont;
	if (b->openFileAs("century schoolbook", "../../lib/unix/fonts/c059016l.pfa",
					  "../../lib/unix/fonts/c059016l.afm", AP_UnixFont::STYLE_BOLD))
		_addFont(b);

	b = new AP_UnixFont;
	if (b->openFileAs("century schoolbook", "../../lib/unix/fonts/c059033l.pfa",
					  "../../lib/unix/fonts/c059033l.afm", AP_UnixFont::STYLE_ITALIC))
		_addFont(b);

	b = new AP_UnixFont;
	if (b->openFileAs("century schoolbook", "../../lib/unix/fonts/c059036l.pfa",
					  "../../lib/unix/fonts/c059036l.afm", AP_UnixFont::STYLE_BOLD_ITALIC))
		_addFont(b);
	
	return UT_TRUE;
}


UT_uint32 AP_UnixFontManager::getCount(void)
{
	return m_vFonts.getItemCount();
}

AP_UnixFont ** AP_UnixFontManager::getAllFonts(void)
{
	UT_uint32 count = m_vFonts.getItemCount();
	
	AP_UnixFont ** table = new AP_UnixFont * [count];

	UT_ASSERT(table);

	for (UT_uint32 i = 0; i < count; i++)
	{
		table[i] = (AP_UnixFont *) m_vFonts.getNthItem(i);
	}

	return table;
}

AP_UnixFont * AP_UnixFontManager::getFont(const char * fontname,
										  AP_UnixFont::style s)
{
	AP_UnixFont * tempFont = NULL;
	
	UT_uint32 count = m_vFonts.getItemCount();
	
	for (UT_uint32 i = 0; i < count; i++)
	{
		tempFont = (AP_UnixFont *) m_vFonts.getNthItem(i);
		UT_ASSERT(tempFont);
		if (tempFont->getStyle() == s)
			return tempFont;
	}
	return NULL;
}

void AP_UnixFontManager::_addFont(AP_UnixFont * font)
{
	m_vFonts.addItem((void *) font);
}
