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
#include <sys/types.h>
#include <sys/stat.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_dialogHelper.h"
#include "xap_UnixFont.h"
#include "xap_UnixFontXLFD.h"


#define ASSERT_MEMBERS	do { UT_ASSERT(m_name); UT_ASSERT(m_fontfile); UT_ASSERT(m_metricfile); } while (0)

/*******************************************************************/

XAP_UnixFontHandle::XAP_UnixFontHandle()
{
	m_font = NULL;
	m_size = 0;
}

XAP_UnixFontHandle::XAP_UnixFontHandle(XAP_UnixFont * font, UT_uint32 size)
{
	m_font = font;
	m_size = size;
}

XAP_UnixFontHandle::XAP_UnixFontHandle(XAP_UnixFontHandle & copy)
{
	m_font = copy.m_font;
	m_size = copy.m_size;
}

XAP_UnixFontHandle::~XAP_UnixFontHandle()
{
}

GdkFont * XAP_UnixFontHandle::getGdkFont(void)
{
	if (m_font)
		return m_font->getGdkFont(m_size);
	else
		return NULL;
}

UT_uint32 XAP_UnixFontHandle::getSize(void)
{
	return m_size;
}


/*******************************************************************/		

XAP_UnixFont::XAP_UnixFont(void)
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

XAP_UnixFont::XAP_UnixFont(XAP_UnixFont & copy)
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

XAP_UnixFont::~XAP_UnixFont(void)
{
	FREEP(m_name);
	
	FREEP(m_fontfile);
	FREEP(m_metricfile);

	DELETEP(m_PFAFile);
	
	FREEP(m_fontKey);

	UT_VECTOR_PURGEALL(allocFont *, m_allocFonts);
	
	// leave GdkFont * alone
}

UT_Bool XAP_UnixFont::openFileAs(const char * fontfile,
								const char * metricfile,
								const char * xlfd,
								XAP_UnixFont::style s)
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

void XAP_UnixFont::setName(const char * name)
{
	FREEP(m_name);
	UT_cloneString(m_name, name);
}

const char * XAP_UnixFont::getName(void)
{
	ASSERT_MEMBERS;
	return m_name;
}

void XAP_UnixFont::setStyle(XAP_UnixFont::style s)
{
	m_style = s;
}

XAP_UnixFont::style XAP_UnixFont::getStyle(void)
{
	ASSERT_MEMBERS;
	return m_style;
}

const char * XAP_UnixFont::getFontfile(void)
{
	ASSERT_MEMBERS;
	
	return m_fontfile;
}

const char * XAP_UnixFont::getMetricfile(void)
{
	ASSERT_MEMBERS;
	return m_metricfile;
}

void XAP_UnixFont::setXLFD(const char * xlfd)
{
	FREEP(m_xlfd);
	UT_cloneString(m_xlfd, xlfd);
}

const char * XAP_UnixFont::getXLFD(void)
{
	ASSERT_MEMBERS;
	return m_xlfd;
}

FontInfo * XAP_UnixFont::getMetricsData(void)
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
		fclose(fp);
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

	fclose(fp);
	
	UT_ASSERT(m_metricsData);
	UT_ASSERT(m_metricsData->gfi);
	return m_metricsData;
}

UT_Bool XAP_UnixFont::openPFA(void)
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

UT_Bool XAP_UnixFont::closePFA(void)
{
	if (m_PFAFile)
	{
		fclose(m_PFAFile);
		return UT_TRUE;
	}
	return UT_FALSE;
}

char XAP_UnixFont::getPFAChar(void)
{
	return fgetc(m_PFAFile);
}

const char * XAP_UnixFont::getFontKey(void)
{
	ASSERT_MEMBERS;
	return m_fontKey;
}

GdkFont * XAP_UnixFont::getGdkFont(UT_uint32 pixelsize)
{
	// this might return NULL, but that means a font at a certain
	// size couldn't be found
	UT_uint32 l = 0;
	UT_uint32 count = m_allocFonts.getItemCount();
	allocFont * entry;

	while (l < count)
	{
		entry = (allocFont *) m_allocFonts.getNthItem(l);
		if (entry && entry->pixelSize == pixelsize)
			return entry->gdkFont;
		else
			l++;
	}

	GdkFont * gdkfont = NULL;
	
	// If the font is really, really small (an EXTREMELY low Zoom can trigger this) some
	// fonts will be calculated to 0 height.  Bump it up to 2 since the user obviously
	// doesn't care about readability anyway.  :)
	if (pixelsize < 2)
		pixelsize = 2;

	// create a real object around that string
	XAP_UnixFontXLFD myXLFD(m_xlfd);

	// Must set a pixel size, or a point size, but we're getting
	// automunged pixel sizes appropriate for our resolution from
	// the layout engine, so use pixel sizes.  They're not really
	// much more accurate this way, but they're more consistent
	// with how the layout engine wants fonts.
	myXLFD.setPixelSize(pixelsize);

	// TODO  add any other special requests, like for a specific encoding
	// TODO  or registry, or resolution here

	char * newxlfd = myXLFD.getXLFD();

	gdkfont = gdk_font_load(newxlfd);

	if (!gdkfont)
	{
		free(newxlfd);
		newxlfd = myXLFD.getFallbackXLFD();
	    gdkfont = gdk_font_load(newxlfd);
	}

	if (!gdkfont)
	{
		char message[1024];
		g_snprintf(message, 1024,
				   "Could not load X font:\n\n"
				   "%s\n\n"
				   "This could mean that the X display server font path has not been set\n"
				   "to reflect the addition of the AbiSuite font set.\n"
				   "\n"
				   "Please read the file 'README' included in the Unix font distribution releases\n"
				   "for instructions on installing these fonts.",
				   newxlfd);
		messageBoxOK(message);
		exit(1);
	}

	free(newxlfd);
	
	allocFont * item = new allocFont;
	item->pixelSize = pixelsize;
	item->gdkFont = gdkfont;
	m_allocFonts.addItem((void *) item);

	return gdkfont;
}

void XAP_UnixFont::_makeFontKey(void)
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
