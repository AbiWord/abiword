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
#include <sys/stat.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_dialogHelper.h"
#include "xap_UnixFont.h"

#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)
#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)

#define ASSERT_MEMBERS	do { UT_ASSERT(m_name); UT_ASSERT(m_fontfile); UT_ASSERT(m_metricfile); } while (0)

AP_UnixFont::AP_UnixFont(void)
{
	m_name = NULL;
	m_style = STYLE_LAST;
	m_xlfd = NULL;
	
	m_fontfile = NULL;
	m_metricfile = NULL;

	m_metricsData = NULL;
	
	m_PFAFile = NULL;
	
	m_fontKey = NULL;
}

AP_UnixFont::AP_UnixFont(AP_UnixFont & copy)
{
	m_name = NULL;
	m_style = STYLE_LAST;
	m_xlfd = NULL;

	m_fontfile = NULL;
	m_metricfile = NULL;
	
	m_metricsData = NULL;

	m_PFAFile = NULL;

	m_fontKey = NULL;

	openFileAs(copy.getFontfile(),
			   copy.getMetricfile(),
			   copy.getXLFD(),
			   copy.getStyle());
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

	struct stat buf;
	int err;
	
	err = stat(fontfile, &buf);
	UT_ASSERT(err == 0 || err == -1);

	if (! (err == 0 || S_ISREG(buf.st_mode)) )
	{
		return UT_FALSE;
	}
	
	err = stat(metricfile, &buf);
	UT_ASSERT(err == 0 || err == -1);

	if (! (err == 0 || S_ISREG(buf.st_mode)) )
	{
		return UT_FALSE;
	}

	// strip our proper face name out of the XLFD
	char * newxlfd;
	UT_cloneString(newxlfd, xlfd);

	// run past the first field (foundry)
	strtok(newxlfd, "-");
	// save the second to a member
	FREEP(m_name);
	UT_cloneString(m_name, strtok(NULL, "-"));
	
	free(newxlfd);
	
	// save to memebers
	FREEP(m_fontfile);
	UT_cloneString(m_fontfile, fontfile);
	FREEP(m_metricfile);
	UT_cloneString(m_metricfile, metricfile);
	m_style = s;
	FREEP(m_xlfd);
	UT_cloneString(m_xlfd, xlfd);

	// update our key so we can be identified
	_makeFontKey();

	return UT_TRUE;
}

void AP_UnixFont::setName(const char * name)
{
	FREEP(m_name);
	UT_cloneString(m_name, name);
}

const char * AP_UnixFont::getName(void)
{
	ASSERT_MEMBERS;
	return m_name;
}

void AP_UnixFont::setStyle(AP_UnixFont::style s)
{
	m_style = s;
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

void AP_UnixFont::setXLFD(const char * xlfd)
{
	FREEP(m_xlfd);
	UT_cloneString(m_xlfd, xlfd);
}

const char * AP_UnixFont::getXLFD(void)
{
	ASSERT_MEMBERS;
	return m_xlfd;
}

FontInfo * AP_UnixFont::getMetricsData(void)
{
	if (m_metricsData)
		return m_metricsData;

	UT_ASSERT(m_metricfile);
	
	// open up the metrics file, which should have been proven to
	// exist earlier in the construction of this object.
	FILE * fp = fopen(m_metricfile, "r");

	char message[1024];

	if (!fp)
	{
		g_snprintf(message, 1024,
				   "The font metrics file [%s] could\n"
				   "not be opened for parsing.  Please ensure that this file\n"
				   "is present before printing.  Right now, this is a pretty\n"
				   "darn fatal error.",
				   m_metricfile);
		messageBoxOK(message);
		return NULL;
	}

	// call down to the Adobe code
	int result = parseFile(fp, &m_metricsData, P_GW);
	switch (result)
	{
	case parseError:
		g_snprintf(message, 1024,
				   "AbiWord encountered errors parsing the font metrics file\n"
				   "[%s].\n"
				   "These errors were not fatal, but print metrics may be incorrect.",
				   m_metricfile);
		messageBoxOK(message);
		break;
	case earlyEOF:
		g_snprintf(message, 1024,
				   "AbiWord encountered a premature End of File (EOF) while parsing\n"
				   "the font metrics file [%s].\n"
				   "Printing cannot continue.",
				   m_metricfile);
		messageBoxOK(message);
		m_metricsData = NULL;
		break;
	case storageProblem:
		// if we got here, either the metrics file is broken (like it's
		// saying it has 209384098278942398743982 kerning lines coming, and
		// we know we can't allocate that), or we really did run out of memory.
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		m_metricsData = NULL;
		break;
	default:
		// everything is peachy
		break;
	}

	UT_ASSERT(m_metricsData);
	UT_ASSERT(m_metricsData->gfi);
	return m_metricsData;
}

UT_Bool AP_UnixFont::openPFA(void)
{
	ASSERT_MEMBERS;
	
	m_PFAFile = fopen(m_fontfile, "r");

	if (!m_PFAFile)
	{
		char message[1024];
		g_snprintf(message, 1024,
				   "Font data file [%s] can not be opened for reading.\n", m_fontfile);
		messageBoxOK(message);
		return UT_FALSE;
	}

	return UT_TRUE;
}

UT_Bool AP_UnixFont::closePFA(void)
{
	if (m_PFAFile)
	{
		fclose(m_PFAFile);
		return UT_TRUE;
	}
	return UT_FALSE;
}

char AP_UnixFont::getPFAChar(void)
{
	return fgetc(m_PFAFile);
}

const char * AP_UnixFont::getFontKey(void)
{
	ASSERT_MEMBERS;
	return m_fontKey;
}

GdkFont * AP_UnixFont::getGdkFont(UT_uint32 pointsize)
{
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

	GdkFont * gdkfont = NULL;
	
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
			g_snprintf(number, 5, "%ld", pointsize);
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

	gdkfont = gdk_font_load(newxlfd);

	if (!gdkfont)
	{
		char message[1024];
		g_snprintf(message, 1024,
				   "Could not load X font [%s].\n"
				   "If this font is an AbiWord Type 1 font, has this font file\n"
				   "been properly installed according to the instructions at\n"
				   "'http://www.abisource.com/dev_download.phtml#type1'?\n",
				   newxlfd);
		messageBoxOK(message);
		UT_ASSERT(0);
	}

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

	char * copy;
	UT_cloneString(copy, m_name);
	UT_upperString(copy);
	
	sprintf(key, "%s@%d", copy, m_style);

	FREEP(copy);
	
	// point member our way
	m_fontKey = key;
}
