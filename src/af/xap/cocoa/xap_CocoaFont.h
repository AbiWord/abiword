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

#import <AppKit/AppKit.h>

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
	XAP_CocoaFont(XAP_CocoaFont & copy);
	
	~XAP_CocoaFont(void);

	bool 				openFileAs(const char * fontfile,
									   const char * metricfile,
									   const char * xlfd,
									   XAP_CocoaFont::style s);
	void					setName(const char * name);
	const char * 			getName(void);

	void					setStyle(XAP_CocoaFont::style s);
	XAP_CocoaFont::style		getStyle(void);

	const char * 			getFontfile(void);
	const char * 			getMetricfile(void);
	const encoding_pair*	loadEncodingFile(void);
	const encoding_pair*	loadEncodingFile(char * file);
	const encoding_pair*	getEncodingTable() const {return m_pEncodingTable;}
	inline UT_uint32				getEncodingTableSize() const {return m_iEncodingTableSize;}

	void					setXLFD(const char * xlfd);
	const char * 			getXLFD(void);

//	ABIFontInfo *			getMetricsData(void);
	UT_uint16				getCharWidth(UT_UCSChar c);
	
	bool					openPFA(void);
	char					getPFAChar(void);
	bool					closePFA(void);	
	bool                    isSizeInCache(UT_uint32 pixelsize);
	const char * 			getFontKey(void);
	NSFont *				getNSFont(UT_uint32 pixelsize);

	NSFont *				getMatchNSFont(UT_uint32 size);
	XAP_CocoaFont *          getMatchCocoaFont(void);

	bool					is_TTF_font() const {return (m_fontType == FONT_TYPE_TTF);}
	bool					is_PS_font()  const {return ((m_fontType == FONT_TYPE_PFA) || (m_fontType == FONT_TYPE_PFB));}
	font_type				getFontType() const {return m_fontType;}

protected:
	bool					_createTtfSupportFiles();
	bool					_createPsSupportFiles();
	struct allocFont
	{
		UT_uint32			pixelSize;
		NSFont *			gdkFont;
	};

	void					_makeFontKey();
	char * 					m_fontKey;

	// a cache of GdkFont * at a given size
	UT_Vector				m_allocFonts;
	
	char * 					m_name;
	XAP_CocoaFont::style		m_style;
	char * 					m_xlfd;
	
	// The next line is the info that is given back to us by parseAFM. The line after that is our own mangled one to follow Unicode.
//	ABIFontInfo *			m_metricsData;
	uniWidth *				m_uniWidths;

	char * 					m_fontfile;
	char *					m_metricfile;

	// The font file proper
	FILE *	 				m_PFFile;
	bool					m_PFB;
	UT_ByteBuf				m_buffer;
	UT_uint32				m_bufpos;
	
	encoding_pair * 		m_pEncodingTable;
	UT_uint32				m_iEncodingTableSize;
	char					_getPFBChar(void);
	bool					_getMetricsDataFromX(void);
	void					_deleteEncodingTable();

	struct CJK_PSFontMetric
	{
          int ascent;
          int descent;
          int width;
	};
	bool					m_is_cjk;
	CJK_PSFontMetric			m_cjk_font_metric;

//	bool					m_bIsTTF;
	font_type				m_fontType;
	bool                    m_bisCopy;
public:
	static XAP_CocoaFont *			s_defaultNonCJKFont[4];
	static XAP_CocoaFont *			s_defaultCJKFont[4];

	inline bool is_CJK_font()const{ return m_is_cjk; }
	void set_CJK_font(bool v){ m_is_cjk=v; }
	inline int get_CJK_Ascent() const { return m_cjk_font_metric.ascent; }
	inline int get_CJK_Descent()const { return m_cjk_font_metric.descent; }
	inline int get_CJK_Width()const{ return m_cjk_font_metric.width; }
	void set_CJK_Ascent(int i){ m_cjk_font_metric.ascent=i; }
	void set_CJK_Descent(int i){ m_cjk_font_metric.descent=i; }
	void set_CJK_Width(int i){ m_cjk_font_metric.width=i; }

};

/* Values found in PFB files */
#define	PFB_MARKER	0x80
#define PFB_ASCII	1
#define PFB_BINARY	2
#define PFB_DONE	3

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
	XAP_CocoaFontHandle(XAP_CocoaFont * font, UT_uint32 size);	
	XAP_CocoaFontHandle(const XAP_CocoaFontHandle & copy);
	~XAP_CocoaFontHandle();

	NSFont * 		getNSFont(void);
	UT_uint32		getSize(void);
	

	inline XAP_CocoaFont *getCocoaFont()	const{ return m_font; }
	inline NSFont      *getMatchNSFont()	{ return m_font? m_font->getMatchNSFont(m_size): NULL; }
	
//	void explodeGdkFonts(GdkFont* & non_cjk_one,GdkFont*& cjk_one);	
//	void explodeCocoaFonts(XAP_CocoaFont**  pSingleByte, XAP_CocoaFont ** pMultiByte);	
private:
	// data items
	XAP_CocoaFont *				m_font;
	UT_uint32					m_size;
};

#endif /* XAP_COCOAFONT_H */
