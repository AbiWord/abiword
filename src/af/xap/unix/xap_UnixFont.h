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

#include <X11/Xft/Xft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

class ps_Generate;
class XAP_UnixFontManager;

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

	XAP_UnixFont(XAP_UnixFontManager * pFM);
	XAP_UnixFont(const XAP_UnixFont & copy);
	
	~XAP_UnixFont(void);

	bool 					openFileAs(const char * fontfile,
									   const char * metricfile,
									   const char * family,
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

	UT_uint16				getCharWidth(UT_UCSChar c) const;
	
	bool					embedInto(ps_Generate& ps);
	bool					openPFA(void);
	char					getPFAChar(void);
	bool					closePFA(void);	
	const char * 			getFontKey(void);
	XftFont*				getDeviceXftFont(UT_uint32 pixelsize, UT_uint32 zoomPercentage) const; // don't measure me!
	XftFont*				getLayoutXftFont(UT_uint32 pixelsize) const; // don't draw me!

	bool					is_TTF_font() const {return (m_fontType == FONT_TYPE_TTF);}
	bool					is_PS_font()  const {return ((m_fontType == FONT_TYPE_PFA) || (m_fontType == FONT_TYPE_PFB));}
	font_type				getFontType() const {return m_fontType;}

	void					getCoverage(UT_Vector& coverage);

	float					getAscender(UT_uint32 iSize) const;
	float					getDescender(UT_uint32 iSize) const;
	float					measureUnRemappedChar(const UT_UCSChar c, UT_uint32 iSize) const;
	UT_String				getPostscriptName() const;
	
	void					setFontManager(XAP_UnixFontManager * pFm);
	
private:
	bool					_createTtfSupportFiles();
	bool					_createPsSupportFiles();
	struct allocFont
	{
		UT_uint32			pixelSize;
		XftFont*			xftFont;
	};

	void					_makeFontKey();
	char * 					m_fontKey;

	// a cache of XftFont * at a given size
	mutable UT_Vector		m_allocFonts;
	
	char * 					m_name;
	XAP_UnixFont::style		m_style;
	char * 					m_xlfd;
	
	// The next line is the info that is given back to us by parseAFM. The line after that is our own mangled one to follow Unicode.
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
	void					_deleteEncodingTable();

	font_type				m_fontType;
	bool                    m_bisCopy;

	XAP_UnixFontManager	*	m_pFontManager;
	
	XftFont*				getFontFromCache(UT_uint32 pixelsize, bool bIsLayout, UT_uint32 zoomPercentage) const;
	void					insertFontInCache(UT_uint32 pixelsize, XftFont* pXftFont) const;
	void					fetchXftFont(UT_uint32 pixelsize) const;

	/* last used font.  Only usable for when we don't care about the pixel size */
	mutable XftFont*		m_pXftFont;
	mutable GR_CharWidths	m_cw;
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

	XftFont * 					getLayoutXftFont(void);
	XftFont * 					getDeviceXftFont(UT_uint32 zoomPercentage);

	UT_uint32					getSize(void) { return m_size; }
	void						setSize(UT_uint32 size) { m_size = size; }
	
	XAP_UnixFont*				getUnixFont() const { return m_font; }
	virtual const char*			getFamily() const { return getUnixFont()->getName(); }

// private:
	XAP_UnixFont*				m_font;
	UT_uint32					m_size;
};

/*****************************************************************/
/* little locker for a face.  At least by now I will leave it out of a pimpl */
class XftFaceLocker
{
public:
	XftFaceLocker(XftFont* pFont = NULL);
	XftFaceLocker(const XftFaceLocker& other);
	~XftFaceLocker();

	XftFaceLocker& operator= (const XftFaceLocker& other);

	FT_Face                 getFace() { return m_pFace; }

private:
	XftFont*		m_pFont;
	FT_Face			m_pFace;
};

#endif /* XAP_UNIXFONT_H */
