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

#include "xap_UnixPSParseAFM.h"
  
class XAP_UnixFont
{
 public:

	typedef enum
	{
		STYLE_NORMAL = 0,
		STYLE_BOLD,
		STYLE_ITALIC,
		STYLE_BOLD_ITALIC,
		STYLE_LAST	// this must be last
	} style;

	
	XAP_UnixFont(void);
	XAP_UnixFont(XAP_UnixFont & copy);
	
	~XAP_UnixFont(void);

	UT_Bool 				openFileAs(const char * fontfile,
									   const char * metricfile,
									   const char * xlfd,
									   XAP_UnixFont::style s);
	void					setName(const char * name);
	const char * 			getName(void);

	void					setStyle(XAP_UnixFont::style s);
	XAP_UnixFont::style		getStyle(void);

	const char * 			getFontfile(void);
	const char * 			getMetricfile(void);

	void					setXLFD(const char * xlfd);
	const char * 			getXLFD(void);

	ABIFontInfo *			getMetricsData(void);
	UT_uint16 *				getUniWidths(void);
	
	UT_Bool					openPFA(void);
	char					getPFAChar(void);
	UT_Bool					closePFA(void);	

	const char * 			getFontKey(void);
	GdkFont *				getGdkFont(UT_uint32 pixelsize);

protected:

	struct allocFont
	{
		UT_uint32			pixelSize;
		GdkFont *			gdkFont;
	};

	void					_makeFontKey();
	char * 					m_fontKey;

	// a cache of GdkFont * at a given size
	UT_Vector				m_allocFonts;
	
	char * 					m_name;
	XAP_UnixFont::style		m_style;
	char * 					m_xlfd;
	
	// The next line is the info that is given back to us by parseAFM. The line after that is our own mangled one to follow Unicode.
	ABIFontInfo *			m_metricsData;
	UT_uint16 *				m_uniWidths;

	char * 					m_fontfile;
	char *					m_metricfile;

	// The font file proper
	FILE *	 				m_PFFile;
	UT_Bool					m_PFB;
	UT_ByteBuf				m_buffer;
	UT_uint32				m_bufpos;
	char					_getPFBChar(void);
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
	XAP_UnixFontHandle(XAP_UnixFontHandle & copy);
	~XAP_UnixFontHandle();

	GdkFont * 		getGdkFont(void);
	UT_uint32		getSize(void);
	
	// data items
	XAP_UnixFont *				m_font;
	UT_uint32					m_size;
};

#endif /* XAP_UNIXFONT_H */
