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

#ifndef XAP_UNIXFONT_H
#define XAP_UNIXFONT_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_bytebuf.h"

#include "gr_Graphics.h"
#include "gr_CharWidths.h"

#include "xap_UnixPSParseAFM.h"
#include "xap_UnixFontXLFD.h"

#include "ut_AdobeEncoding.h"
#ifdef USE_XFT
#include </usr/local/include/X11/Xft/Xft.h>
#include </usr/local/include/X11/Xft/XftFreetype.h>
#endif


class ps_Generate;

struct uniWidth
{
	UT_UCSChar ucs;
	UT_uint16  width;
};

typedef enum {FONT_TYPE_PFA, FONT_TYPE_PFB, FONT_TYPE_TTF, FONT_TYPE_UNKNOWN} font_type;

class ABI_EXPORT XAP_UnixFont
{
 public:

	enum style
	{
		STYLE_NORMAL = 0,
		STYLE_BOLD,
		STYLE_ITALIC,
		STYLE_BOLD_ITALIC,
		STYLE_OUTLINE,
		STYLE_BOLD_OUTLINE,
		STYLE_LAST	// this must be last
	};

	XAP_UnixFont(void);
	XAP_UnixFont(const XAP_UnixFont & copy);
	
	~XAP_UnixFont(void);

	bool 					openFileAs(const char * fontfile,
									   const char * metricfile,
									   const char * xlfd,
									   XAP_UnixFont::style s);
	void					setName(const char * name);
	const char * 			getName(void) const;

	void					setStyle(XAP_UnixFont::style s);
	XAP_UnixFont::style		getStyle(void) const;

	const char * 			getFontfile(void) const;
	const char * 			getMetricfile(void) const;
	const encoding_pair*	loadEncodingFile(void);
	const encoding_pair*	loadEncodingFile(char * file);
	const encoding_pair*	getEncodingTable() const {return m_pEncodingTable;}
	UT_uint32				getEncodingTableSize() const {return m_iEncodingTableSize;}

	void					setXLFD(const char * xlfd);
	const char * 			getXLFD(void) const;

	ABIFontInfo *			getMetricsData(void);
#ifdef USE_XFT
	UT_uint16				getCharWidth(UT_UCSChar c) const;
#else
	UT_uint16				getCharWidth(UT_UCSChar c);
#endif

	
	bool					embedInto(ps_Generate& ps);
	bool					openPFA(void);
	char					getPFAChar(void);
	bool					closePFA(void);	
	bool                    isSizeInCache(UT_uint32 pixelsize);
	const char * 			getFontKey(void);
#ifdef USE_XFT
	XftFont*				getXftFont(UT_uint32 pixelsize) const;
#else
	GdkFont *				getGdkFont(UT_uint32 pixelsize);

	GdkFont *				getMatchGdkFont(UT_uint32 size);
	XAP_UnixFont *          getMatchUnixFont(void);
#endif

	bool					is_TTF_font() const {return (m_fontType == FONT_TYPE_TTF);}
	bool					is_PS_font()  const {return ((m_fontType == FONT_TYPE_PFA) || (m_fontType == FONT_TYPE_PFB));}
	font_type				getFontType() const {return m_fontType;}

	void					getCoverage(UT_Vector& coverage);

#ifdef USE_XFT
	float					getAscender(UT_uint32 iSize) const;
	float					getDescender(UT_uint32 iSize) const;
	float					measureUnRemappedChar(const UT_UCSChar c, UT_uint32 iSize) const;
	UT_String				getPostscriptName() const;
#endif

protected:
	bool					_createTtfSupportFiles();
	bool					_createPsSupportFiles();
	struct allocFont
	{
		UT_uint32			pixelSize;
#ifdef USE_XFT
		XftFont*			xftFont;
#else
		GdkFont *			gdkFont;
#endif
	};

	void					_makeFontKey();
	char * 					m_fontKey;

	// a cache of GdkFont * at a given size
	mutable UT_Vector		m_allocFonts;
	
	char * 					m_name;
	XAP_UnixFont::style		m_style;
	char * 					m_xlfd;
	
	// The next line is the info that is given back to us by parseAFM. The line after that is our own mangled one to follow Unicode.
	ABIFontInfo *			m_metricsData;
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
	static XAP_UnixFont *			s_defaultNonCJKFont[4];
	static XAP_UnixFont *			s_defaultCJKFont[4];

	inline bool is_CJK_font() const { return m_is_cjk; }
	void set_CJK_font(bool v) { m_is_cjk=v; }
	inline int get_CJK_Ascent() const { return m_cjk_font_metric.ascent; }
	inline int get_CJK_Descent() const { return m_cjk_font_metric.descent; }
	inline int get_CJK_Width() const { return m_cjk_font_metric.width; }
	void set_CJK_Ascent(int i) { m_cjk_font_metric.ascent=i; }
	void set_CJK_Descent(int i) { m_cjk_font_metric.descent=i; }
	void set_CJK_Width(int i) { m_cjk_font_metric.width=i; }

private:
#ifdef USE_XFT
	XftFont*				getFontFromCache(UT_uint32 pixelsize) const;
	void					insertFontInCache(UT_uint32 pixelsize, XftFont* pXftFont) const;

	/* last used font.  Only usable for when we don't care about the pixel size */
	mutable XftFont*		m_pXftFont;
	mutable GR_CharWidths	m_cw;
#endif
};

/* Values found in PFB files */
#define	PFB_MARKER	0x80
#define PFB_ASCII	1
#define PFB_BINARY	2
#define PFB_DONE	3

/*****************************************************************/

/*
  We derive our handle from GR_Font so we can be passed around the GR
  contexts as a native Unix font, much like a Windows font handle.
  GR_Font is a completely virtual class with no data or accessors of
  its own.
*/

class XAP_UnixFontHandle : public GR_Font
{
 public:

	XAP_UnixFontHandle();
	XAP_UnixFontHandle(XAP_UnixFont * font, UT_uint32 size);	

#ifdef USE_XFT
	XftFont * 					getXftFont(void);
#else
	GdkFont * 					getGdkFont(void);
#endif
	UT_uint32					getSize(void) { return m_size; }
	void						setSize(UT_uint32 size) { m_size = size; }
	
	XAP_UnixFont*				getUnixFont() const { return m_font; }
	virtual const char*			getFamily() const { return getUnixFont()->getName(); }

#ifndef USE_XFT
	GdkFont*					getMatchGdkFont()	{ return m_font? m_font->getMatchGdkFont(m_size): NULL; }
	
	void						explodeGdkFonts(GdkFont* & non_cjk_one,GdkFont*& cjk_one);	
	void						explodeUnixFonts(XAP_UnixFont**  pSingleByte, XAP_UnixFont** pMultiByte);
#endif

// private:
	XAP_UnixFont*				m_font;
	UT_uint32					m_size;
};

/*****************************************************************/
/* little locker for a face.  At least by now I will leave it out of a pimpl */
#ifdef USE_XFT
class XftFaceLocker
{
public:
	XftFaceLocker(XftFont* pFont = NULL);
	XftFaceLocker(const XftFaceLocker& other);
	~XftFaceLocker();

	XftFaceLocker& operator= (const XftFaceLocker& other);

	FT_Face			getFace() { return m_pFace; }

private:
	XftFont*		m_pFont;
	FT_Face			m_pFace;
};
#endif

#endif /* XAP_UNIXFONT_H */
