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

#ifndef XAP_COCOAFONT_H
#define XAP_COCOAFONT_H

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_bytebuf.h"

#include "gr_Graphics.h"

#include "ut_AdobeEncoding.h"

struct uniWidth
{
	UT_UCSChar ucs;
	UT_uint16  width;
};

typedef enum {FONT_TYPE_PFA, FONT_TYPE_PFB, FONT_TYPE_TTF, FONT_TYPE_UNKNOWN} font_type;

class XAP_CocoaFont
{
 public:

	typedef enum
	{
		STYLE_NORMAL = 0,
		STYLE_BOLD,
		STYLE_ITALIC,
		STYLE_BOLD_ITALIC,
		STYLE_OUTLINE,
		STYLE_BOLD_OUTLINE,
		STYLE_LAST	// this must be last
	} style;

	
	XAP_CocoaFont(void);
	XAP_CocoaFont(const XAP_CocoaFont & copy);
	
	~XAP_CocoaFont(void);

	void					setName(const char * name);
	const char * 			getName(void) const;

	void					setStyle(XAP_CocoaFont::style s);
	XAP_CocoaFont::style		getStyle(void) const;

//	ABIFontInfo *			getMetricsData(void);
//	UT_uint16				getCharWidth(UT_UCSChar c);
	
	bool                    isSizeInCache(UT_uint32 pixelsize);
	const char * 			getFontKey(void) const;
	NSFont *				getNSFont(UT_uint32 pixelsize);

//	NSFont *				getMatchNSFont(UT_uint32 size);
//	XAP_CocoaFont *          getMatchCocoaFont(void);


private:
/*
	struct allocFont
	{
		allocFont() { nsFont = nil; };
		~allocFont() { [nsFont release;];}
		UT_uint32			pixelSize;
		NSFont *			nsFont;
	};
 */
	void					_makeFontKey();
	char * 					m_fontKey;

	// cache NSFont. Key is NSNumber (size)
	NSMutableDictionary*	m_allocFonts;
	
	
	char * 					m_name;
	NSString*				m_nsName;
	XAP_CocoaFont::style		m_style;
	
	UT_ByteBuf				m_buffer;
	UT_uint32				m_bufpos;

	bool                    m_bisCopy;
};

/*****************************************************************/

/*
  We derive our handle from GR_Font so we can be passed around the GR
  contexts as a native Cocoa font, much like a Windows font handle.
  GR_Font is a completely virtual class with no data or accessors of
  its own.
*/

class XAP_CocoaFontHandle : public GR_Font
{
 public:

	XAP_CocoaFontHandle();
	XAP_CocoaFontHandle(const XAP_CocoaFont * font, UT_uint32 size);	
	XAP_CocoaFontHandle(const XAP_CocoaFontHandle & copy);
	~XAP_CocoaFontHandle();

	NSFont * 		getNSFont(void);
	UT_uint32		getSize(void);
	

	inline const XAP_CocoaFont *getCocoaFont()	const { return m_font; }
//	inline NSFont      *getMatchNSFont()	{ return m_font? m_font->getMatchNSFont(m_size): NULL; }
	
//	void explodeGdkFonts(GdkFont* & non_cjk_one,GdkFont*& cjk_one);	
//	void explodeCocoaFonts(XAP_CocoaFont**  pSingleByte, XAP_CocoaFont ** pMultiByte);	
private:
	// data items
	XAP_CocoaFont *				m_font;
	UT_uint32					m_size;
};

#endif /* XAP_COCOAFONT_H */
