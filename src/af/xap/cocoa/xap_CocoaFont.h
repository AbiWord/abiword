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

#ifndef XAP_COCOAFONT_H
#define XAP_COCOAFONT_H

#import <Cocoa/Cocoa.h>

#include "ut_vector.h"
#include "gr_Graphics.h"


/*
  We derive our handle from GR_Font so we can be passed around the GR
  contexts as a native Cocoa font.
  GR_Font is a completely virtual class with no data or accessors of
  its own.
*/

class XAP_CocoaFont : public GR_Font
{
public:
	XAP_CocoaFont();
	XAP_CocoaFont(NSFont * font);
	XAP_CocoaFont(const XAP_CocoaFont & copy);

	~XAP_CocoaFont();

	ATSUStyle               makeAtsuStyle(NSFont *font) const;
	NSFont * 				getNSFont(void) const 
		{ return m_font; }

	UT_uint32				getSize(void);
	const char * 			getName(void);
	
	float					getAscent();
	float					getDescent(); /* returns -descent because it is <0 on CG */
	float					getHeight();
	void 					getCoverage(UT_NumberVector& coverage);
	
	virtual UT_sint32 		measureUnremappedCharForCache(UT_UCSChar cChar) const;
//
// UT_Rect of glyph in Logical units.
// rec.left = bearing Left (distance from origin to start)
// rec.width = width of the glyph
// rec.top = distance from the origin to the top of the glyph
// rec.height = total height of the glyph

	virtual bool glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics *);

	enum RemapFont {
		rf_None = 0,
		rf_Symbols,
		rf_Dings
	};
	bool					needsRemap()
		{
			return m_rfRemap;
		}
	static RemapFont		remapFont(NSFont * font);
	static UT_UCS4Char		remapChar(UT_UCS4Char charCode, RemapFont rf);
	UT_UCS4Char				remapChar(UT_UCS4Char charCode) const 
		{ return m_rfRemap ? remapChar(charCode, m_rfRemap) : charCode; }

private:
	RemapFont						m_rfRemap;

	NSFont *						m_font;
	mutable ATSUStyle                       m_styleForCache;

	void					_resetMetricsCache();
	static void				_initMetricsLayouts(void);

	/*!
		Measure the char for the given NSFont
	 */
	UT_sint32				_measureChar(UT_UCSChar cChar, ATSUStyle style) const;

	/* metrics cache */
	UT_uint32						_m_size;
	volatile float							_m_ascent;
	volatile float							_m_descent;
	volatile float							_m_height;
	UT_NumberVector *				_m_coverage;
	mutable unichar         _m_text[2]; /**< buffer for the text layout */

	/*! static metrics stuff */
	static ATSUTextLayout       s_atsuLayout;
};

@interface XAP_CocoaFontFamilyHelper : NSObject
{
	NSString *				m_FontFamilyName;
	NSAttributedString *	m_AttributedFontFamilyName;

	NSMutableArray *		m_FontNames;
	NSMutableArray *		m_FontMembers;
	NSMutableArray *		m_AttributedFontMembers;

	unsigned				m_count;

	int						m_indexRegular;
	int						m_indexItalic;
	int						m_indexBold;
	int						m_indexBoldItalic;
}
+ (NSMutableDictionary *)fontFamilyHelperDictionary:(NSMutableDictionary *)referenceDictionary;

- (id)initWithFontFamilyName:(NSString *)fontFamilyName known:(BOOL)known;
- (void)dealloc;

- (NSString *)fontFamilyName;
- (NSAttributedString *)attributedFontFamilyName;

- (unsigned)count;

- (NSArray *)fontNames;
- (NSArray *)fontMembers;
- (NSArray *)attributedFontMembers;

/* These return -1 if there is no matching font member.
 */
- (int)indexRegular;
- (int)indexItalic;
- (int)indexBold;
- (int)indexBoldItalic;

- (void)addFontReferences:(NSMutableDictionary *)referenceDictionary;
@end

@interface XAP_CocoaFontReference : NSObject
{
	NSString *						m_FontFamily;

	XAP_CocoaFontFamilyHelper *		m_FontFamilyHelper;

	unsigned						m_index;
}
- (id)initWithFontFamily:(NSString *)fontFamily helper:(XAP_CocoaFontFamilyHelper *)helper index:(unsigned)index;
- (void)dealloc;

- (NSString *)fontFamily;

- (XAP_CocoaFontFamilyHelper *)fontFamilyHelper;

- (unsigned)index;
@end

#endif /* XAP_COCOAFONT_H */
