/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

ATSUTextLayout XAP_CocoaFont::s_atsuLayout = NULL;

/*******************************************************************/


XAP_CocoaFont::XAP_CocoaFont()
  : GR_Font(),
	m_font(nil),
	m_styleForCache(NULL),
	_m_coverage(NULL)
{
	m_hashKey = "";
	_resetMetricsCache();

	m_rfRemap = rf_None;
}

XAP_CocoaFont::XAP_CocoaFont(NSFont* font)
  : GR_Font(),
	m_font(nil),
	m_styleForCache(NULL),
	_m_coverage(NULL)
{
	m_hashKey = [[font fontName] UTF8String];
	m_font = font;
	[m_font retain];
	_resetMetricsCache();

	m_rfRemap = remapFont(m_font);
}

XAP_CocoaFont::XAP_CocoaFont(const XAP_CocoaFont & copy)
  : GR_Font(copy),
	m_font(nil),
	m_styleForCache(NULL),
	_m_coverage(NULL)
{
	m_hashKey = copy.hashKey();
	m_font = [copy.getNSFont() copy];
	_resetMetricsCache();

	m_rfRemap = remapFont(m_font);
}

XAP_CocoaFont::~XAP_CocoaFont()
{
	// release on nil is completely safe
	[m_font release];
	if (m_styleForCache) {
		ATSUDisposeStyle(m_styleForCache);
	}
	DELETEP(_m_coverage);
}


ATSUStyle XAP_CocoaFont::makeAtsuStyle(NSFont *font) const
{
	ATSUStyle atsuStyle;
	OSStatus status;
	
	status = ::ATSUCreateStyle(&atsuStyle);
	UT_ASSERT(status == 0);

	CFStringRef fontName = (CFStringRef)[font fontName];
	ATSFontRef fontRef = ATSFontFindFromPostScriptName(fontName, 0);
	if (fontRef == kATSUInvalidFontID) {
		fontRef = ATSFontFindFromName(fontName, 0);
		if (fontRef == kATSUInvalidFontID) {
			NSLog(@"Unable to find ATSU font %@", fontName);
		}
	}
	
	// upside down - Flipped
	CGAffineTransform transform = CGAffineTransformMakeScale (1,-1);
	Fixed fontSize = FloatToFixed([font pointSize]);
	
	ATSUAttributeTag styleTags[] = { 
		kATSUSizeTag, 
		kATSUFontTag, 
		kATSUFontMatrixTag
	};
	ByteCount styleSizes[] = {
		sizeof(Fixed), 
		sizeof(ATSFontRef), 
		sizeof(CGAffineTransform) 
	};
	ATSUAttributeValuePtr styleValues[] = { 
		&fontSize, 
		&fontRef, 
		&transform  
	};
	status = ATSUSetAttributes (atsuStyle, 3, styleTags, styleSizes, styleValues);
	UT_ASSERT(status == 0);

	return atsuStyle;
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
	return [[m_font fontName] UTF8String];
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
	memset(_m_text, 0, sizeof(_m_text));
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
		UT_ASSERT(_m_descent >= 0);
	}
	return _m_descent;
}

float XAP_CocoaFont::getHeight()
{
	if (_m_height == 0.0)
	{
		_m_height = getAscent() + getDescent();
	}
	return _m_height;
}

void XAP_CocoaFont::getCoverage(UT_NumberVector& coverage)
{
	UT_uint32 i, begin;
	bool lastState = false;
	bool currentState = false;
	if (_m_coverage) {
		xxx_UT_DEBUGMSG(("getCoverage(): return cached coverage\n"));
		coverage = *_m_coverage;
		return;
	}
	_m_coverage = new UT_NumberVector(10);
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
				_m_coverage->push_back(begin);
				_m_coverage->push_back(i - begin);
				xxx_UT_DEBUGMSG(("getCoverage(): adding range %lu - %lu\n", begin, i - begin));
			}
			lastState = currentState;
		}
	}
	coverage = *_m_coverage;
}

// UT_Rect of glyph in Logical units.
// rec.left = bearing Left (distance from origin to start)
// rec.width = width of the glyph
// rec.top = distance from the origin to the top of the glyph
// rec.height = total height of the glyph

bool XAP_CocoaFont::glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG)
{
	bool bHaveGlyph = false;

	NSGlyph aGlyph;

	NSRect rect;

	UT_UCS4Char c = remapChar(g, remapFont(m_font));

	UT_ASSERT(UT_NOT_IMPLEMENTED);
#if 0  
	if (!m_LayoutHelper)
	{
		m_LayoutHelper = new XAP_CocoaFont_LayoutHelper(m_font);
	}
	if (m_LayoutHelper) {
		if (m_LayoutHelper->setUnichar(c, aGlyph)) {// convert from unicode to glyph
			if ([m_font glyphIsEncoded:aGlyph])	{
				bHaveGlyph = true;
				rect = [m_font boundingRectForGlyph:aGlyph];
				rec.width  = static_cast<UT_sint32>(pG->ftluD(rect.size.width));
				rec.height = static_cast<UT_sint32>(pG->ftluD(rect.size.height));
				rec.left   = static_cast<UT_sint32>(pG->ftluD(rect.origin.x));
				rec.top    = static_cast<UT_sint32>(pG->ftluD(rect.origin.y)) + rec.height;

				if (![[m_font familyName] isEqualToString:@"cmex10"]) {
					NSSize adv = [m_font advancementForGlyph:aGlyph];
					if (adv.width > rect.size.width) {
						rec.width = static_cast<UT_sint32>(pG->ftluD(adv.width));
					}
				}
			}
		}
	}
#endif
	return bHaveGlyph;
}

UT_sint32 XAP_CocoaFont::measureUnremappedCharForCache(UT_UCSChar cChar) const
{
	if (m_styleForCache == NULL)
	{
		NSFont *fontForCache = [[NSFontManager sharedFontManager] convertFont:m_font toSize:GR_CharWidthsCache::CACHE_FONT_SIZE];

		m_styleForCache = makeAtsuStyle(fontForCache);
	}
	return _measureChar (cChar, m_styleForCache);
}

#define LAYOUT_CONTAINER_WIDTH 10000.0f

void XAP_CocoaFont::_initMetricsLayouts(void)
{
	OSStatus status;
	status = ATSUCreateTextLayout(&s_atsuLayout);
	UT_ASSERT(status == 0);
    ATSLineLayoutOptions lineLayoutOptions = (kATSLineFractDisable | kATSLineDisableAutoAdjustDisplayPos | 
											  kATSLineUseDeviceMetrics | kATSLineDisableAllLayoutOperations |
											  kATSLineKeepSpacesOutOfMargin | kATSLineHasNoHangers);
    ATSUAttributeTag tags[] = { 
		kATSULineLayoutOptionsTag 
	};
    ByteCount sizes[] = { 
		sizeof(ATSLineLayoutOptions) 
	};
    ATSUAttributeValuePtr values[] = { 
		&lineLayoutOptions 
	};
    status = ATSUSetLayoutControls(s_atsuLayout, 1, tags, sizes, values);
	UT_ASSERT(status == 0);
//    status = ATSUSetTransientFontMatching(s_atsuLayout, YES);
//	UT_ASSERT(status == 0);
}

UT_sint32 XAP_CocoaFont::_measureChar(UT_UCSChar cChar, ATSUStyle style) const
{
	if (!s_atsuLayout) {
		_initMetricsLayouts();
	}
	OSStatus status;
	UT_uint32 charWidth = 0;

	unichar c2 = remapChar(cChar, remapFont(m_font));
	_m_text[0] = c2;
	status = ATSUSetTextPointerLocation(s_atsuLayout, _m_text, 0, 1, 1);
	UT_ASSERT(status == 0);
	status = ATSUSetRunStyle(s_atsuLayout, m_styleForCache, 0, 1);
	UT_ASSERT(status == 0);

	ATSUTextMeasurement after;
	status = ::ATSUGetUnjustifiedBounds(s_atsuLayout, 0, 1,
										NULL, &after, NULL, NULL);
	UT_ASSERT(status == 0);
	charWidth = FixedToInt(after);

	return charWidth;
}

typedef struct {
	UT_UCS4Char	unicode;
	const char *	description;
} UT_SpecialCharacter;

#if 0
static UT_SpecialCharacter s_unicode_extra[] = {
	{ 0x00A0, "Non-breaking space" },
	{ 0x2206, "Increment (Delta)"  },
	{ 0x2126, "Ohm sign (Omega)"   },
	{ 0x00B5, "Micro sign (mu)"    },
	{ 0x2044, "Fraction slash"     }
};
#endif

/* Unicode mapping for Symbol (provided by Adobe, available from http://www.unicode.org/)
 */
static UT_UCS4Char s_unicode_4_Symbol_lo[95] = {
	  0x0020, 
	/*0x00A0,*/0x0021, 0x2200, 0x0023,  0x2203,  0x0025, 0x0026, 0x220B,  0x0028, 0x0029, 0x2217, 0x002B,  0x002C,  0x2212, 0x002E, 0x002F, 
	  0x0030,  0x0031, 0x0032, 0x0033,  0x0034,  0x0035, 0x0036, 0x0037,  0x0038, 0x0039, 0x003A, 0x003B,  0x003C,  0x003D, 0x003E, 0x003F, 
	  0x2245,  0x0391, 0x0392, 0x03A7,  0x0394, 
	                                  /*0x2206,*/0x0395, 0x03A6, 0x0393,  0x0397, 0x0399, 0x03D1, 0x039A,  0x039B,  0x039C, 0x039D, 0x039F, 
	  0x03A0,  0x0398, 0x03A1, 0x03A3,  0x03A4,  0x03A5, 0x03C2, 0x03A9, 
	                                                           /*0x2126,*/0x039E, 0x03A8, 0x0396, 0x005B,  0x2234,  0x005D, 0x22A5, 0x005F, 
	  0xF8E5,  0x03B1, 0x03B2, 0x03C7,  0x03B4,  0x03B5, 0x03C6, 0x03B3,  0x03B7, 0x03B9, 0x03D5, 0x03BA,  0x03BB,/*0x00B5,*/ 
	                                                                                                                0x03BC, 0x03BD, 0x03BF, 
	  0x03C0,  0x03B8, 0x03C1, 0x03C3,  0x03C4,  0x03C5, 0x03D6, 0x03C9,  0x03BE, 0x03C8, 0x03B6, 0x007B,  0x007C,  0x007D, 0x223C
};

static UT_UCS4Char s_unicode_4_Symbol_hi[95] = {
	0x20AC, 0x03D2, 0x2032, 0x2264,/*0x2044,*/ 
	                                 0x2215, 0x221E, 0x0192, 0x2663,  0x2666, 0x2665, 0x2660, 0x2194,  0x2190, 0x2191, 0x2192, 0x2193, 
	0x00B0, 0x00B1, 0x2033, 0x2265,  0x00D7, 0x221D, 0x2202, 0x2022,  0x00F7, 0x2260, 0x2261, 0x2248,  0x2026, 0xF8E6, 0xF8E7, 0x21B5, 
	0x2135, 0x2111, 0x211C, 0x2118,  0x2297, 0x2295, 0x2205, 0x2229,  0x222A, 0x2283, 0x2287, 0x2284,  0x2282, 0x2286, 0x2208, 0x2209, 
	0x2220, 0x2207, 0x00AE, 0x00A9,  0x2122, 0x220F, 0x221A, 0x22C5,  0x00AC, 0x2227, 0x2228, 0x21D4,  0x21D0, 0x21D1, 0x21D2, 0x21D3, 
	0x25CA, 0x3008, 0x00AE, 0x00A9,  0x2122, 0x2211, 0xF8EB, 0xF8EC,  0xF8ED, 0xF8EE, 0xF8EF, 0xF8F0,  0xF8F1, 0xF8F2, 0xF8F3, 0xF8F4, 
0xF8FF,     0x3009, 0x222B, 0x2320,  0xF8F5, 0x2321, 0xF8F6, 0xF8F7,  0xF8F8, 0xF8F9, 0xF8FA, 0xF8FB,  0xF8FC, 0xF8FD, 0xF8FE
};

/* Unicode mapping for Wingdings font, from http://www.alanwood.net/ with a couple of changes (not sure about 0xAA which I've changed
 * from a four-point star to a heart; is this a Dingbats vs Wingdings difference?)
 */
static UT_UCS4Char s_unicode_4_Wingdings[224] = {
	0x0020, 0x270F, 0x2702, 0x2701,  0x0024, 0x0025, 0x0026, 0x0027,  0x260E, 0x2706, 0x2709, 0x261E,  0x002C, 0x002D, 0x002E, 0x002F, 
	0x0030, 0x0031, 0x0032, 0x2713,  0x0034, 0x0035, 0x231B, 0x2328,  0x0038, 0x0039, 0x003A, 0x003B,  0x003C, 0x003D, 0x2707, 0x270D, 
	0x0040, 0x270C, 0x0042, 0x0043,  0x0044, 0x261C, 0x261E, 0x261D,  0x261F, 0x0049, 0x263A, 0x004B,  0x2639, 0x004D, 0x2620, 0x2690, 
	0x0050, 0x2708, 0x263C, 0x2605,  0x2744, 0x0055, 0x271E, 0x0057,  0x2720, 0x2721, 0x262A, 0x262F,  0x0950, 0x2638, 0x2648, 0x2649, 
	0x264A, 0x264B, 0x254C, 0x264D,  0x264E, 0x264F, 0x2650, 0x2651,  0x2652, 0x2653, 0x0026, 0x0026,  0x25CF, 0x274D, 0x25A0, 0x25A1, 
	0x0070, 0x2751, 0x2752, 0x27A4,  0x2666, 0x25C6, 0x2756, 0x0077,  0x2327, 0x2353, 0x2318, 0x2740,  0x273F, 0x275D, 0x275E, 0x25AF, 
	0x24EA, 0x2460, 0x2461, 0x2462,  0x2463, 0x2464, 0x2465, 0x2466,  0x2467, 0x2468, 0x2469, 0x24FF,  0x2776, 0x2777, 0x2778, 0x2779, 
	0x277A, 0x277B, 0x277C, 0x277D,  0x277E, 0x277F, 0x0096, 0x0097,  0x0098, 0x0099, 0x009A, 0x009B,  0x009C, 0x009D, 0x00B7, 0x2022, 
	0x25AA, 0x25CB, 0x00A2, 0x00A3,  0x25C9, 0x25CE, 0x00A6, 0x25AA,  0x25FB, 0x2666, 0x2665, 0x2605,  0x2736, 0x2734, 0x2739, 0x2735, 
	0x00B0, 0x2316, 0x2727, 0x2311,  0x00B4, 0x272A, 0x2730, 0x00B7,  0x00B8, 0x00B9, 0x00BA, 0x00BB,  0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3,  0x00C4, 0x00C5, 0x00C6, 0x00C7,  0x00C8, 0x00C9, 0x00CA, 0x00CB,  0x00CC, 0x00CD, 0x00CE, 0x00CF, 
	0x00D0, 0x00D1, 0x00D2, 0x00D3,  0x00D4, 0x232B, 0x2326, 0x00D7,  0x27A2, 0x00D9, 0x00DA, 0x00DB,  0x27B2, 0x00DD, 0x00DE, 0x00DF, 
	0x00E0, 0x00E1, 0x00E2, 0x00E3,  0x00E4, 0x00E5, 0x00E6, 0x00E7,  0x2794, 0x00E9, 0x00EA, 0x00EB,  0x00EC, 0x00ED, 0x00EE, 0x21E6, 
	0x21E8, 0x21E7, 0x21E9, 0x2B04,  0x21F3, 0x2B00, 0x2B01, 0x2B03,  0x2B02, 0x25AD, 0x25AB, 0x2717,  0x2713, 0x2612, 0x2611, 0x00FF
};

XAP_CocoaFont::RemapFont XAP_CocoaFont::remapFont(NSFont * font)
{
	RemapFont rf = rf_None;

	if (font) {
		if ([[font familyName] isEqualToString:@"Symbol"]) {
			rf = rf_Symbols;
		}
		else if ([[font familyName] isEqualToString:@"Wingdings"    ] ||
				 [[font familyName] isEqualToString:@"Dingbats"     ] ||
				 [[font familyName] isEqualToString:@"Webdings"     ] ||
				 [[font familyName] isEqualToString:@"Zapf Dingbats"]) {
			rf = rf_Dings;
		}
	}
	return rf;
}

UT_UCS4Char XAP_CocoaFont::remapChar(UT_UCS4Char charCode, RemapFont rf)
{
	UT_UCS4Char remappedCharCode = charCode;

	switch (rf) {
	case rf_Symbols:
		if ((charCode & 0xff) == charCode) {
			if (charCode &  0x80) {
				charCode &= 0x7f;

				if ((charCode & 0x60) && (charCode != 0x7f))
					remappedCharCode = s_unicode_4_Symbol_hi[charCode - 0x20];
			}
			else {
				if ((charCode & 0x60) && (charCode != 0x7f))
					remappedCharCode = s_unicode_4_Symbol_lo[charCode - 0x20];
			}
		}
		break;

	case rf_Dings:
		if ((charCode & 0xff) == charCode) {
			if (charCode & 0xe0)
				remappedCharCode = s_unicode_4_Wingdings[charCode - 0x20];
		}
		break;

	default:
		break;
	}
	return remappedCharCode;
}

@implementation XAP_CocoaFontFamilyHelper

+ (NSMutableDictionary *)fontFamilyHelperDictionary:(NSMutableDictionary *)referenceDictionary
{
	NSFontManager * FM = [NSFontManager sharedFontManager];

	NSArray * Families = [FM availableFontFamilies];

	unsigned count = [Families count];

	NSMutableDictionary * helperDictionary = [NSMutableDictionary dictionaryWithCapacity:count];

	for (unsigned i = 0; i < count; i++)
	{
		NSString * Family = [Families objectAtIndex:i];

		XAP_CocoaFontFamilyHelper * helper = [[XAP_CocoaFontFamilyHelper alloc] initWithFontFamilyName:Family known:YES];

		[helperDictionary setObject:helper forKey:Family];

		[helper addFontReferences:referenceDictionary];
		[helper release];
	}
	return helperDictionary;
}

- (id)initWithFontFamilyName:(NSString *)fontFamilyName known:(BOOL)known
{
	if (self = [super init])
	{
		m_FontFamilyName = fontFamilyName;
		[m_FontFamilyName retain];

		NSFontManager * FM = [NSFontManager sharedFontManager];

		m_indexRegular    = -1;
		m_indexItalic     = -1;
		m_indexBold       = -1;
		m_indexBoldItalic = -1;

		if (known)
		{
			NSArray * Members = [FM availableMembersOfFontFamily:m_FontFamilyName];

			m_count = [Members count];

			m_FontNames             = [[NSMutableArray alloc] initWithCapacity:m_count];
			m_FontMembers           = [[NSMutableArray alloc] initWithCapacity:m_count];
			m_AttributedFontMembers = [[NSMutableArray alloc] initWithCapacity:m_count];

			for (unsigned i = 0; i < m_count; i++)
			{
				NSArray * Member = [Members objectAtIndex:i];

				NSString * fontName   = [Member objectAtIndex:0];
				NSString * memberName = [Member objectAtIndex:1];

				NSFont * font = [NSFont fontWithName:fontName size:[NSFont smallSystemFontSize]];

				NSDictionary * attr = font ? [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName] : [NSDictionary dictionary];

				[m_FontNames             addObject:fontName];
				[m_FontMembers           addObject:memberName];
				[m_AttributedFontMembers addObject:[[NSAttributedString alloc] initWithString:memberName attributes:attr]];

				if ([memberName isEqualToString:@"Regular"] ||
					[memberName isEqualToString:@"Plain"])
				{
					m_indexRegular = (int) i;
				}
				else if ([memberName isEqualToString:@"Italic"]  ||
						 [memberName isEqualToString:@"Oblique"] ||
						 [memberName isEqualToString:@"Inclined"])
				{
					m_indexItalic = (int) i;
				}
				else if ([memberName isEqualToString:@"Bold"])
				{
					m_indexBold = (int) i;
				}
				else if ([memberName isEqualToString:@"Bold Italic"]   || [memberName isEqualToString:@"BoldItalic"]  ||
						 [memberName isEqualToString:@"Bold Oblique"]  || [memberName isEqualToString:@"BoldOblique"] ||
						 [memberName isEqualToString:@"Bold Inclined"] || [memberName isEqualToString:@"BoldInclined"])
				{
					m_indexBoldItalic = (int) i;
				}
			}

			unsigned indexRegular = (m_indexRegular >= 0) ? ((unsigned) m_indexRegular) : 0;

			NSArray * Member = [Members objectAtIndex:indexRegular];

			NSString * fontName = [Member objectAtIndex:0];

			NSFont * font = [NSFont fontWithName:fontName size:[NSFont systemFontSize]];

			NSDictionary * attr = font ? [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName] : [NSDictionary dictionary];

			m_AttributedFontFamilyName = [[NSAttributedString alloc] initWithString:m_FontFamilyName attributes:attr];
		}
		else
		{
			NSDictionary * attr = [NSDictionary dictionaryWithObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];

			m_AttributedFontFamilyName = [[NSAttributedString alloc] initWithString:m_FontFamilyName attributes:attr];

			m_count = 1;

			m_FontNames             = [[NSMutableArray alloc] initWithCapacity:1];
			m_FontMembers           = [[NSMutableArray alloc] initWithCapacity:1];
			m_AttributedFontMembers = [[NSMutableArray alloc] initWithCapacity:1];

			[m_FontNames             addObject:fontFamilyName];
			[m_FontMembers           addObject:@"-"];
			[m_AttributedFontMembers addObject:[[NSAttributedString alloc] initWithString:@"-" attributes:attr]];
		}
	}
	return self;
}

- (void)dealloc
{
	[m_FontFamilyName           release];
	[m_AttributedFontFamilyName release];

	[m_FontNames                release];
	[m_FontMembers              release];
	[m_AttributedFontMembers    release];

	[super dealloc];
}

- (NSString *)fontFamilyName
{
	return m_FontFamilyName;
}

- (NSAttributedString *)attributedFontFamilyName
{
	return m_AttributedFontFamilyName;
}

- (unsigned)count
{
	return m_count;
}

- (NSArray *)fontNames
{
	return m_FontNames;
}

- (NSArray *)fontMembers
{
	return m_FontMembers;
}

- (NSArray *)attributedFontMembers
{
	return m_AttributedFontMembers;
}

- (int)indexRegular
{
	return m_indexRegular;
}

- (int)indexItalic
{
	return m_indexItalic;
}

- (int)indexBold
{
	return m_indexBold;
}

- (int)indexBoldItalic
{
	return m_indexBoldItalic;
}

- (void)addFontReferences:(NSMutableDictionary *)referenceDictionary
{
	for (unsigned i = 0; i < m_count; i++)
	{
		NSString * fontName = [m_FontNames objectAtIndex:i];

		XAP_CocoaFontReference * fontRef = [[XAP_CocoaFontReference alloc] initWithFontFamily:m_FontFamilyName helper:self index:i];

		[referenceDictionary setObject:fontRef forKey:fontName];

		[fontRef release];
	}
}

@end

@implementation XAP_CocoaFontReference

- (id)initWithFontFamily:(NSString *)fontFamily helper:(XAP_CocoaFontFamilyHelper *)helper index:(unsigned)index
{
	if (self = [super init])
	{
		m_FontFamily = fontFamily;
		[m_FontFamily retain];
		
		m_FontFamilyHelper = helper;
		[m_FontFamilyHelper retain];

		m_index = index;
	}
	return self;
}

- (void)dealloc
{
	[m_FontFamily       release];
	[m_FontFamilyHelper release];

	[super dealloc];
}

- (NSString *)fontFamily
{
	return m_FontFamily;
}

- (XAP_CocoaFontFamilyHelper *)fontFamilyHelper
{
	return m_FontFamilyHelper;
}

- (unsigned)index
{
	return m_index;
}

@end
