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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#import <Cocoa/Cocoa.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_CocoaFont.h"
#include "xap_EncodingManager.h"
#include "xap_App.h"
//#include "ut_AdobeEncoding.h"

#if 0
//this one is for use with qsort
static int s_compareUniWidths(const void * w1, const void * w2)
{
	const uniWidth   * W1 = (const uniWidth   * ) w1;
	const uniWidth   * W2 = (const uniWidth   * ) w2;

	if(W1->ucs < W2->ucs)
		return -1;
	if(W1->ucs > W2->ucs)
		return 1;
	return 0;
}

//this one is for use with bsearch
static int s_compareUniWidthsChar(const void * c, const void * w)
{
	const UT_UCSChar * C = (const UT_UCSChar * ) c;
	const uniWidth   * W = (const uniWidth   * ) w;
	if(*C < W->ucs)
		return -1;
	if(*C > W->ucs)
		return 1;
	return 0;
}
#endif

#ifdef DEBUG
#define ASSERT_MEMBERS	do { UT_ASSERT(m_name); UT_ASSERT(m_nsName); } while (0)
#else
#define ASSERT_MEMBERS
#endif

/*******************************************************************/

XAP_CocoaFontHandle::XAP_CocoaFontHandle()
  : m_font(NULL), m_size(0)
{
}

XAP_CocoaFontHandle::XAP_CocoaFontHandle(const XAP_CocoaFont * font, UT_uint32 size)
  : m_size(size)
{
	m_font = new XAP_CocoaFont (*font);
}

XAP_CocoaFontHandle::XAP_CocoaFontHandle(const XAP_CocoaFontHandle & copy)
  : GR_Font(copy),  m_size(copy.m_size)
{
	m_font = new XAP_CocoaFont (*copy.m_font);
}

XAP_CocoaFontHandle::~XAP_CocoaFontHandle()
{
	DELETEP (m_font);
}

NSFont * XAP_CocoaFontHandle::getNSFont(void)
{
	if (m_font)
		return m_font->getNSFont(m_size);
	else
		return NULL;
}

UT_uint32 XAP_CocoaFontHandle::getSize(void)
{
	return m_size;
}

#if 0
void XAP_CocoaFontHandle::explodeCocoaFonts(XAP_CocoaFont ** pSingleByte, XAP_CocoaFont ** pMultiByte)
{
	if(m_font==NULL)
	{
		*pSingleByte = NULL;
		*pMultiByte = NULL;
		return;
	}

	if(m_font->is_CJK_font())
	{
		*pMultiByte = m_font;
		*pSingleByte = m_font->getMatchCocoaFont();
	}
	else
	{
		*pSingleByte = m_font;
		*pMultiByte = m_font->getMatchCocoaFont();
	}		
}

void XAP_CocoaFontHandle::explodeGdkFonts(GdkFont* & non_cjk_one,GdkFont*& cjk_one)
{
	if(m_font->is_CJK_font())
	  {
		non_cjk_one=getMatchGdkFont();
		cjk_one=getGdkFont();
	  }
	else
	  {
		non_cjk_one=getGdkFont();
		cjk_one=getMatchGdkFont();
	  }
}
#endif

/*******************************************************************/		

XAP_CocoaFont::XAP_CocoaFont(void)
  :  m_fontKey(NULL), m_name(NULL), m_nsName(nil), m_style(STYLE_LAST),
     m_bufpos(0), 
     m_bisCopy(false)
{
	//UT_DEBUGMSG(("XAP_CocoaFont:: constructor (void)\n"));	
}

XAP_CocoaFont::XAP_CocoaFont(const XAP_CocoaFont & copy)
{
	//UT_DEBUGMSG(("XAP_CocoaFont:: copy constructor\n"));

	m_name = NULL;
	m_nsName = nil;

	m_fontKey = NULL;
	setName (copy.getName());
	m_style = copy.m_style;
	m_bufpos = 0;

	m_bisCopy = true;
}

XAP_CocoaFont::~XAP_CocoaFont(void)
{
	
	FREEP(m_name);
	if (m_nsName != nil) {
		[m_nsName release];
	}

	FREEP(m_fontKey);

	[m_allocFonts release];
}


void XAP_CocoaFont::setName(const char * name)
{
	FREEP(m_name);
	UT_DEBUGMSG (("XAP_CocoaFont::setName(%s)\n", name));
	UT_cloneString(m_name, name);
	if (m_nsName != nil) {
		[m_nsName release];
	}
	m_nsName = [[NSString alloc] initWithCString:m_name];
	_makeFontKey ();
}

const char * XAP_CocoaFont::getName(void) const
{
	ASSERT_MEMBERS;
	return m_name;
}

/*
NSString * XAP_CocoaFont::getName(void)
{
	ASSERT_MEMBERS;
	return m_nsName;
}
*/

void XAP_CocoaFont::setStyle(XAP_CocoaFont::style s)
{
	m_style = s;
}

XAP_CocoaFont::style XAP_CocoaFont::getStyle(void) const
{
	ASSERT_MEMBERS;
	return m_style;
}



const char * XAP_CocoaFont::getFontKey(void) const
{
	ASSERT_MEMBERS;
	return m_fontKey;
}
/*!
 * returns true if the requested pixelsize is in the cache.
\params pixelsize: This of the font.
\returns true if found
*/
bool XAP_CocoaFont::isSizeInCache(UT_uint32 pixelsize)
{
	NSFont* font = [m_allocFonts objectForKey:[NSNumber numberWithUnsignedLong:pixelsize]];
	return (font != nil);
}

NSFont * XAP_CocoaFont::getNSFont(UT_uint32 pixelsize)
{
	NSNumber* key = [NSNumber numberWithUnsignedLong:pixelsize];
	NSFont * nsfont = [m_allocFonts objectForKey:key];
	if (nsfont)
	{
		return nsfont;
	}
	// If the font is really, really small (an EXTREMELY low Zoom can trigger this) some
	// fonts will be calculated to 0 height.  Bump it up to 2 since the user obviously
	// doesn't care about readability anyway.  :)
	if (pixelsize < 2) {
		pixelsize = 2;
	}

	// TODO  add any other special requests, like for a specific encoding
	// TODO  or registry, or resolution here


	int s;
	switch(m_style)
	{
	case XAP_CocoaFont::STYLE_NORMAL:
		s=0;
		break;
	case XAP_CocoaFont::STYLE_BOLD:
		s=1;
		break;
	case XAP_CocoaFont::STYLE_ITALIC:
		s=2;
		break;
	case XAP_CocoaFont::STYLE_BOLD_ITALIC:
		s=3;
		break;
	default:
		s=0;
	}
	if ((!m_name) || (strcmp (m_name, "Default") == 0)) {
		nsfont = [[NSFont labelFontOfSize:(float)pixelsize] retain];
	}
	else {
		nsfont = [[NSFont fontWithName:m_nsName size:(float)pixelsize] retain];
	}
	if (nsfont)
	{
		[m_allocFonts setObject:nsfont forKey:key];
	}
	else
	{
		NSLog(@"XAP_CocoaFont::getNSFont(): Font not found !");
	}

	return nsfont;
}

void XAP_CocoaFont::_makeFontKey()
{
	ASSERT_MEMBERS;

	// if we already have a key, free it
	FREEP(m_fontKey);
	
	// allocate enough to combine name, seperator, style, and NULL into key.
	// this won't work if we have styles that require two digits in decimal.
	char * key = (char *) UT_calloc(strlen(m_name) + 1 + 1 + 1, sizeof(char));
	UT_ASSERT(key);

	char * copy;
	UT_cloneString(copy, m_name);
	UT_upperString(copy);
	
	sprintf(key, "%s@%d", copy, m_style);

	FREEP(copy);
	
	// point member our way
	m_fontKey = key;
}


