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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fstream.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixFont.h"

#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)
#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)

#define ASSERT_MEMBERS	do { UT_ASSERT(m_name); UT_ASSERT(m_fontfile); UT_ASSERT(m_metricfile); } while (0)

AP_UnixFont::AP_UnixFont(void)
{
	m_name = NULL;
	m_style = AP_UnixFont::STYLE_LAST;
		
	m_fontfile = NULL;
	m_metricfile = NULL;

	m_PFAFile = NULL;
	
	m_fontKey = NULL;
}

AP_UnixFont::~AP_UnixFont(void)
{
	FREEP(m_name);
	
	FREEP(m_fontfile);
	FREEP(m_metricfile);

	DELETEP(m_PFAFile);
	
	FREEP(m_fontKey);

	UT_VECTOR_PURGEALL(allocFont *, m_allocFonts);
	
	// leave GdkFont * alone
}

UT_Bool AP_UnixFont::openFileAs(const char * fontfile,
								const char * metricfile,
								const char * xlfd,
								AP_UnixFont::style s)
{
	// test all our data to make sure we can continue
	if (!fontfile)
		return UT_FALSE;
	if (!metricfile)
		return UT_FALSE;
	if (!xlfd)
		return UT_FALSE;
	
	ifstream font;
	ifstream metric;
	
	font.open(fontfile);
	if (!font)
	{
		goto CloseFilesExit;
	}
	font.close();
	
	metric.open(metricfile);
	if (!metric)
	{
		goto CloseFilesExit;
	}
	// TODO use the metric file for something (like parsing it
	// and holding some information)
	metric.close();

	// strip our proper face name out of the XLFD
	char * newxlfd;
	UT_cloneString(newxlfd, xlfd);

	// run past the first field (foundry)
	strtok(newxlfd, "-");
	// save the second to a member
	UT_cloneString(m_name, strtok(NULL, "-"));
	
	free(newxlfd);
	
	// save to memebers
	UT_cloneString(m_fontfile, fontfile);
	UT_cloneString(m_metricfile, metricfile);
	m_style = s;
	UT_cloneString(m_xlfd, xlfd);

	// update our key so we can be identified
	_makeFontKey();

	return UT_TRUE;

 CloseFilesExit:
	if (font)
		font.close();
	if (metric)
		metric.close();

	return UT_FALSE;
}
	
const char * AP_UnixFont::getName(void)
{
	ASSERT_MEMBERS;
	return m_name;
}

AP_UnixFont::style AP_UnixFont::getStyle(void)
{
	ASSERT_MEMBERS;
	return m_style;
}

const char * AP_UnixFont::getFontfile(void)
{
	ASSERT_MEMBERS;
	
	return m_fontfile;
}

const char * AP_UnixFont::getMetricfile(void)
{
	ASSERT_MEMBERS;
	return m_metricfile;
}

const char * AP_UnixFont::getXLFD(void)
{
	ASSERT_MEMBERS;
	return m_xlfd;
}

UT_Bool AP_UnixFont::openPFA(void)
{
	ASSERT_MEMBERS;
	
	m_PFAFile = new ifstream;
	m_PFAFile->open(m_fontfile);

	if (!m_PFAFile)
	{
		UT_DEBUGMSG(("Font file [%s] can not be opened for reading.\n",
					 m_fontfile));
		return UT_FALSE;
	}

	return UT_TRUE;
}

UT_Bool AP_UnixFont::closePFA(void)
{
	if (m_PFAFile)
	{
		m_PFAFile->close();
		delete m_PFAFile;
		return UT_TRUE;
	}
	return UT_FALSE;
}

char AP_UnixFont::getPFAChar(void)
{
	char ch = 0;
	m_PFAFile->get(ch);
	return ch;
}

const char * AP_UnixFont::getFontKey(void)
{
	ASSERT_MEMBERS;
	return m_fontKey;
}

GdkFont * AP_UnixFont::getGdkFont(UT_uint16 pointsize)
{
	ASSERT_MEMBERS;

	allocFont * entry = NULL;

	// this might return NULL, but that means a font at a certain
	// size couldn't be found
	UT_uint32 l;
	UT_uint32 count;
	for (l = 0, count = m_allocFonts.getItemCount();
		 l < count; entry = (allocFont *) m_allocFonts.getNthItem(l), l++)
	{
		if (entry && entry->pointSize == pointsize)
			return entry->gdkFont;
	}
		
	// GDK/X wants to load fonts with point sizes 2 and up
	if (pointsize <= 1)
		return NULL;

	/*
	  NOTE: when we get the XLFD, it will (most likely) have a "0"
	  for both its pixel size and point size.  This means the X
	  server will scale the font to any requested size.  This also
	  means that it's up to us to re-format the XLFD in this font
	  so that the proper point size is in the proper field.  Also,
	  X wants requests in decipoints, so we multiply by 10 while we're
	  at it.  As a sidenote, if the font does NOT have a "0" for
	  the point size, it was registered at a specific size and (1)
	  users shouldn't do that and (2) it should probably work anyway,
	  since X will scale fonts for you (with pretty horrible
	  results sometimes).
	*/

	// add 5; 1 for the new NULL, 4 for the max size the new size
	// number could use
	char * newxlfd = (char *) calloc(strlen(m_xlfd) + 5, sizeof (char));
	char * oldcursor = (char *) m_xlfd;
	char * newcursor = (char *) newxlfd;
	int dashcount = 0;
	while (*oldcursor != NULL)
	{
		if (*oldcursor == '-')
			dashcount++;
		
		// after the eighth, insert the size
		if (dashcount == 8)
		{
			// do the copy
			*newcursor = *oldcursor;
			 
			// do a temp termination so strcat() does the right thing
			newcursor++;
			*newcursor = 0;
			char number[5];
			g_snprintf(number, 5, "%d", pointsize);
			strcat(newxlfd, number);
			// advance by the number of characters in the number string
			newcursor += strlen(number);
			oldcursor++;

			dashcount++;
			
			continue;
		}
		else
		{
			// copy the character
			*newcursor = *oldcursor;
		}

		newcursor++;
		oldcursor++;
	}

	GdkFont * gdkfont = NULL;
	gdkfont = gdk_font_load(newxlfd);

	// this should never happen, since everything in the font list
	// is in a fonts.dir, which lists fonts X has access too.
	UT_ASSERT(gdkfont);

	free(newxlfd);

	allocFont * item = new allocFont;
	item->pointSize = pointsize;
	item->gdkFont = gdkfont;
	m_allocFonts.addItem((void *) item);

	return gdkfont;
}

void AP_UnixFont::_makeFontKey(void)
{
	ASSERT_MEMBERS;

	// if we already have a key, free it
	FREEP(m_fontKey);
	
	// allocate enough to combine name, seperator, style, and NULL into key.
	// this won't work if we have styles that require two digits in decimal.
	char * key = (char *) calloc(strlen(m_name) + 1 + 1 + 1, sizeof(char));
	UT_ASSERT(key);

	sprintf(key, "%s@%d", m_name, m_style);

	// point member our way
	m_fontKey = key;
}
