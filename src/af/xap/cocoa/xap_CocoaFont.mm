/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2003 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_CocoaFont.h"


/*******************************************************************/
NSTextStorage *XAP_CocoaFont::s_fontMetricsTextStorage = nil;
NSLayoutManager *XAP_CocoaFont::s_fontMetricsLayoutManager = nil;
NSTextContainer *XAP_CocoaFont::s_fontMetricsTextContainer = nil;


/*******************************************************************/

XAP_CocoaFont::XAP_CocoaFont()
  : GR_Font(),
	m_font(nil),
	m_fontForCache(nil),
	m_fontProps(nil),
	_m_coverage(NULL)
{
	m_hashKey = "";
	_resetMetricsCache();
}

XAP_CocoaFont::XAP_CocoaFont(NSFont* font)
  : GR_Font(),
	m_font(nil),
	m_fontForCache(nil),
	m_fontProps(nil),
	_m_coverage(NULL)
{
	m_hashKey = [[font fontName] UTF8String];
	m_font = font;
	[m_font retain];
	_resetMetricsCache();
}

XAP_CocoaFont::XAP_CocoaFont(const XAP_CocoaFont & copy)
  : GR_Font(copy),
	m_font(nil),
	m_fontForCache(nil),
	m_fontProps(nil),
	_m_coverage(NULL)
{
	m_hashKey = copy.hashKey();
	m_font = [copy.getNSFont() copy];
	_resetMetricsCache();
}

XAP_CocoaFont::~XAP_CocoaFont()
{
	[m_font release];
	[m_fontForCache release];
	[m_fontProps release];
	DELETEP(_m_coverage);
}


UT_uint32 XAP_CocoaFont::getSize(void)
{
	if (_m_size == 0) {
		_m_size = (UT_uint32)[m_font pointSize];
	}
	return _m_size;
}

const char * XAP_CocoaFont::getName(void)
{
	return [[m_font fontName] cString];
}



/*
	metrics get cached for efficiency. These calls are MUCH CHEAPER than an AppKit call
	since Font is persistant inside the XAP_CocoaFont, the is almost no cost doing that
	it should be intersting to see wether it is not even faster to initialize the cache upon
	construction. 
 */
void XAP_CocoaFont::_resetMetricsCache()
{
	_m_ascent = _m_descent = _m_height = 0.0;
	_m_size = 0;
	DELETEP(_m_coverage);
	_m_coverage = NULL;
}


float XAP_CocoaFont::getAscent()
{
	if (_m_ascent == 0.0)
	{
		_m_ascent = [m_font ascender];
	}
	return _m_ascent;
}

float XAP_CocoaFont::getDescent()
{
	if (_m_descent == 0.0)
	{
		_m_descent = -[m_font descender];
	}
	return _m_descent;
}

float XAP_CocoaFont::getHeight()
{
	if (_m_height == 0.0)
	{
		_m_height = [m_font defaultLineHeightForFont];
	}
	return _m_height;
}

void XAP_CocoaFont::getCoverage(UT_Vector& coverage)
{
	UT_uint32 i, begin;
	bool lastState = false;
	bool currentState = false;
	if (_m_coverage) {
		xxx_UT_DEBUGMSG(("getCoverage(): return cached coverage\n"));
		coverage = *_m_coverage;
		return;
	}
	_m_coverage = new UT_Vector(10);
	const char *bitmap = static_cast<const char*>([[[m_font coveredCharacterSet] bitmapRepresentation] bytes]);
	for (i = 0; i < 0xffff; i++) {
		currentState = (bitmap[i >> 3] & (((unsigned int)1) << (i & 7)));
		if (currentState != lastState) {
			xxx_UT_DEBUGMSG(("getCoverage(): changing state at %lu\n", i));
			if (currentState) {
				begin = i;
				xxx_UT_DEBUGMSG(("getCoverage(): begin range\n"));
			}
			else {
				_m_coverage->push_back(reinterpret_cast<void *>(begin));
				_m_coverage->push_back(reinterpret_cast<void *>(i - begin));
				xxx_UT_DEBUGMSG(("getCoverage(): adding range %lu - %lu\n", begin, i - begin));
			}
			lastState = currentState;
		}
	}
	coverage = *_m_coverage;
}

UT_sint32 XAP_CocoaFont::measureUnremappedCharForCache(UT_UCSChar cChar) const
{
	if (m_fontForCache == nil) {
		m_fontForCache = [[NSFontManager sharedFontManager] 
					convertFont:m_font toSize:GR_CharWidthsCache::CACHE_FONT_SIZE];
		m_fontProps = [[NSMutableDictionary alloc] init];
		[m_fontProps setObject:m_fontForCache forKey:NSFontAttributeName];
	}
	return _measureChar (cChar, m_fontForCache);
}

void XAP_CocoaFont::_initMetricsLayouts(void)
{
	s_fontMetricsTextStorage = [[NSTextStorage alloc] init];

	s_fontMetricsLayoutManager = [[NSLayoutManager alloc] init];
	[s_fontMetricsTextStorage addLayoutManager:s_fontMetricsLayoutManager];

	s_fontMetricsTextContainer = [[NSTextContainer alloc] initWithContainerSize:NSMakeSize(10000, 1000)];
	[s_fontMetricsTextContainer setLineFragmentPadding:0];
	[s_fontMetricsLayoutManager addTextContainer:s_fontMetricsTextContainer];
}

UT_sint32 XAP_CocoaFont::_measureChar(UT_UCSChar cChar, NSFont* font) const
{
    NSAttributedString *attributedString;
    NSRange glyphRange;

    if (!s_fontMetricsTextStorage) {
		_initMetricsLayouts();
    }

	unichar c2 = cChar;		// FIXME: I suspect a problem beetween UT_UCSChar and unichar
	NSString * aString =  [[NSString alloc] initWithCharacters:&c2 length:1];
    attributedString = [[NSAttributedString alloc] initWithString:aString attributes:m_fontProps];
    [s_fontMetricsTextStorage setAttributedString:attributedString];
    [attributedString release];
	[aString release];

    glyphRange = [s_fontMetricsLayoutManager glyphRangeForTextContainer:s_fontMetricsTextContainer];
    if (glyphRange.length == 0) {
        return 0;
	}
	
	NSRect rect = [s_fontMetricsLayoutManager boundingRectForGlyphRange:glyphRange 
				inTextContainer:s_fontMetricsTextContainer];
	return static_cast<UT_uint32>(rect.size.width);
}


