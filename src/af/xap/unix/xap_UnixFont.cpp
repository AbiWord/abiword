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

#include <fstream.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixFont.h"

#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)
#define ASSERT_MEMBERS	do { UT_ASSERT(m_name); UT_ASSERT(m_fontfile); UT_ASSERT(m_metricfile); } while (0)

AP_UnixFont::AP_UnixFont(void)
{
	m_name = NULL;
	m_style = AP_UnixFont::STYLE_LAST;
		
	m_fontfile = NULL;
	m_metricfile = NULL;

	m_PFAFile = NULL;
	
 	m_font = NULL;
}

AP_UnixFont::~AP_UnixFont(void)
{
	DELETEP(m_name);
	
	DELETEP(m_fontfile);
	DELETEP(m_metricfile);
	DELETEP(m_PFAFile);

	// leave GdkFont * alone
}

UT_Bool AP_UnixFont::openFileAs(char * name, char * fontfile,
								char * metricfile, AP_UnixFont::style s)
{
	// test all our data to make sure we can continue
	if (!name)
		return UT_FALSE;
	if (!fontfile)
		return UT_FALSE;
	if (!metricfile)
		return UT_FALSE;

	ifstream font;
	ifstream metric;

	font.open(fontfile);
	if (!font)
	{
		UT_DEBUGMSG(("Can't open font file [%s].\n", fontfile));
		goto CloseFilesExit;
	}
	font.close();
	
	metric.open(metricfile);
	if (!metric)
	{
		UT_DEBUGMSG(("Can't open metric file [%s].\n", metricfile));
		goto CloseFilesExit;
	}
	// TODO use the metric file for something (like parsing it
	// and holding some information)
	metric.close();
	
	// save to memebers
	UT_cloneString(m_name, name);
	UT_cloneString(m_fontfile, fontfile);
	UT_cloneString(m_metricfile, metricfile);
	m_style = s;

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

GdkFont * AP_UnixFont::getGdkFont(void)
{
	ASSERT_MEMBERS;
	return m_font;
}
