/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:nil; -*- */
/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2021 Hubert Figuière
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

#pragma once

#import <Cocoa/Cocoa.h>

#include "gr_Graphics.h"

/*
  We derive our handle from GR_Font so we can be passed around the GR
  contexts as a native Cocoa font.
  GR_Font is a completely virtual class with no data or accessors of
  its own.
*/
class GR_CocoaFont
    : public GR_Font
{
public:
    GR_CocoaFont();
    GR_CocoaFont(NSFont* font);
    GR_CocoaFont(const GR_CocoaFont& copy);

    ~GR_CocoaFont();

    NSFont* getNSFont(void) const
    {
        return m_font;
    }

    CGFloat getSize(void) const;
    const char* getName(void) const;
    virtual const char* getFamily() const override;

    CGFloat getAscent() const;
    CGFloat getDescent() const; /* returns -descent because it is <0 on CG */
    CGFloat getHeight() const;
    void getCoverage(UT_NumberVector& coverage) const;

    virtual UT_sint32 measureUnremappedCharForCache(UT_UCSChar cChar) const override;
    //
    // UT_Rect of glyph in Logical units.
    // rec.left = bearing Left (distance from origin to start)
    // rec.width = width of the glyph
    // rec.top = distance from the origin to the top of the glyph
    // rec.height = total height of the glyph

    virtual bool glyphBox(UT_UCS4Char g, UT_Rect& rec, GR_Graphics*) override;

    enum RemapFont {
        rf_None = 0,
        rf_Symbols,
        rf_Dings
    };
    bool needsRemap() const
    {
        return m_rfRemap;
    }
    static RemapFont remapFont(NSFont* font);
    static UT_UCS4Char remapChar(UT_UCS4Char charCode, RemapFont rf);
    UT_UCS4Char remapChar(UT_UCS4Char charCode) const
    {
        return m_rfRemap ? remapChar(charCode, m_rfRemap) : charCode;
    }

private:
    RemapFont m_rfRemap;

    NSFont* m_font;

    void _resetMetricsCache();
    static void _initMetricsLayouts(void);

    /* metrics cache */
    mutable UT_NumberVector* _m_coverage;
    mutable unichar _m_text[2]; /**< buffer for the text layout */
};

@interface GR_CocoaFontFamilyHelper : NSObject {
    NSString* m_FontFamilyName;
    NSAttributedString* m_AttributedFontFamilyName;

    NSMutableArray* m_FontNames;
    NSMutableArray* m_FontMembers;
    NSMutableArray* m_AttributedFontMembers;

    NSUInteger m_count;

    NSInteger m_indexRegular;
    NSInteger m_indexItalic;
    NSInteger m_indexBold;
    NSInteger m_indexBoldItalic;
}
+ (NSMutableDictionary*)fontFamilyHelperDictionary:(NSMutableDictionary*)referenceDictionary;

- (id)initWithFontFamilyName:(NSString*)fontFamilyName known:(BOOL)known;
- (void)dealloc;

- (NSString*)fontFamilyName;
- (NSAttributedString*)attributedFontFamilyName;

- (NSUInteger)count;

- (NSArray*)fontNames;
- (NSArray*)fontMembers;
- (NSArray*)attributedFontMembers;

/* These return -1 if there is no matching font member.
 */
- (NSInteger)indexRegular;
- (NSInteger)indexItalic;
- (NSInteger)indexBold;
- (NSInteger)indexBoldItalic;

- (void)addFontReferences:(NSMutableDictionary*)referenceDictionary;
@end

@interface GR_CocoaFontReference : NSObject {
    NSString* m_FontFamily;
    GR_CocoaFontFamilyHelper* m_FontFamilyHelper;
    NSUInteger m_index;
}
- (id)initWithFontFamily:(NSString*)fontFamily helper:(GR_CocoaFontFamilyHelper*)helper index:(NSUInteger)index;
- (void)dealloc;

- (NSString*)fontFamily;
- (GR_CocoaFontFamilyHelper*)fontFamilyHelper;
- (NSUInteger)index;
@end
