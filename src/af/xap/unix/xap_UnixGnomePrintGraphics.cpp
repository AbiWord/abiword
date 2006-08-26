/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* AbiSource Application Framework
 * Copyright (C) 2003 Dom Lachowicz
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
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "xap_Strings.h"
#include "xap_UnixGnomePrintGraphics.h"
#include "gr_UnixImage.h"
#include "xap_UnixFrameImpl.h"
#include "ut_string_class.h"
#include "xap_UnixDialogHelper.h"
#include "gr_UnixGraphics.h"

#include <libgnomeprintui/gnome-print-job-preview.h>

#include "xap_Frame.h"
#include "fv_View.h"
#include "fp_PageSize.h"
#include "ut_OverstrikingChars.h"

static inline bool isItalic(XAP_UnixFont::style s)
{
	return ((s == XAP_UnixFont::STYLE_ITALIC) || (s == XAP_UnixFont::STYLE_BOLD_ITALIC));
}

static inline GnomeFontWeight getGnomeFontWeight(XAP_UnixFont::style s)
{
	if (s == XAP_UnixFont::STYLE_BOLD_ITALIC ||
		s == XAP_UnixFont::STYLE_BOLD)
		return GNOME_FONT_BOLD;
	return GNOME_FONT_BOOK;
}

static const struct {
	char * abi;
	char * gp_id;
} paperMap[] = {
	{"Letter",         "USLetter"},
	{"Legal",          "USLegal"},
	{"Folio",          "Executive"},
	{"1/3 A4",         "A4_3"},
	{"1/4 A4",         "A4_4"},
	{"1/8 A4",         "A4_8"},
	{"1/4 A3",         "A3_4"},
	{"1/3 A5",         "A5_3"},
	{"DL Envelope",    "DL"},
	{"C6/C5 Envelope", "C6_C5"},
	{"Envelope No10",  "Envelope_No10"},
	{"Envelope 6x9",   "Envelope_6x9"},
};

GnomePrintConfig * XAP_UnixGnomePrintGraphics::s_setup_config (XAP_Frame * pFrame)
{
	FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
	GnomePrintConfig * cfg = gnome_print_config_default();
	
	const GnomePrintUnit *unit = gnome_print_unit_get_by_abbreviation (reinterpret_cast<const guchar*>("mm"));
	
	gnome_print_config_set (cfg, reinterpret_cast<const guchar *>(GNOME_PRINT_KEY_PAPER_SIZE), reinterpret_cast<const guchar *>("Custom"));
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAGE_MARGIN_TOP), pView->getPageSize().MarginTop(DIM_MM), unit);
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM), pView->getPageSize().MarginBottom(DIM_MM), unit);
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAGE_MARGIN_LEFT), pView->getPageSize().MarginLeft(DIM_MM), unit);
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT), pView->getPageSize().MarginRight(DIM_MM), unit);
	gnome_print_config_set_int (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_NUM_COPIES), 1);	

	if (pView->getPageSize().isPortrait()) {
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_WIDTH), pView->getPageSize().Width (DIM_MM), unit);
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_HEIGHT), pView->getPageSize().Height (DIM_MM), unit);

		gnome_print_config_set (cfg, reinterpret_cast<const guchar *>(GNOME_PRINT_KEY_PAGE_ORIENTATION) , reinterpret_cast<const guchar *>("R0"));
	}
	else {
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_WIDTH), pView->getPageSize().Height (DIM_MM), unit);
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_HEIGHT), pView->getPageSize().Width (DIM_MM), unit);

		gnome_print_config_set (cfg, reinterpret_cast<const guchar *>(GNOME_PRINT_KEY_PAGE_ORIENTATION) , reinterpret_cast<const guchar *>("R90"));
	}

	return cfg;
}

XAP_UnixGnomePrintGraphics::XAP_UnixGnomePrintGraphics(GnomePrintJob *gpm, bool isPreview)
	: GR_Graphics ()
{
	m_gpm          = gpm;
	m_gpc          = gnome_print_job_get_context(gpm);
	
	GnomePrintConfig * cfg = gnome_print_job_get_config (gpm);

	const GnomePrintUnit *from;
	const GnomePrintUnit *to = gnome_print_unit_get_by_abbreviation (reinterpret_cast<const guchar*>("Pt"));

	gnome_print_config_get_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_WIDTH), &m_width, &from);
	gnome_print_convert_distance (&m_width, from, to);

	gnome_print_config_get_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_HEIGHT), &m_height, &from);
	gnome_print_convert_distance (&m_height, from, to);
	m_height = getDeviceResolution()*m_height/72.;
	m_width = getDeviceResolution()*m_width/72.;

	m_bIsPreview     = isPreview;
	m_fm             = static_cast<XAP_UnixApp *>(XAP_App::getApp())->getFontManager();
	m_bStartPrint    = false;
	m_bStartPage     = false;
	m_pCurrentFont   = NULL;
	m_pCurrentPSFont = NULL;	
	m_cs             = GR_Graphics::GR_COLORSPACE_COLOR;
	m_bIsSymbol      = false;
	m_bIsDingbat     = false;
}

// useful for bonobo
XAP_UnixGnomePrintGraphics::XAP_UnixGnomePrintGraphics(GnomePrintContext *ctx,
													   double inWidthDevice, double inHeightDevice)
{
	m_gpm          = NULL;
	m_gpc          = ctx;
	
	m_width  = inWidthDevice;
	m_height = inHeightDevice;

	m_bIsPreview     = false;
	m_fm             = static_cast<XAP_UnixApp *>(XAP_App::getApp())->getFontManager();
	m_bStartPrint    = false;
	m_bStartPage     = false;
	m_pCurrentFont   = NULL;
	m_pCurrentPSFont = NULL;	
	m_cs             = GR_Graphics::GR_COLORSPACE_COLOR;
	m_bIsSymbol      = false;
	m_bIsDingbat     = false;
}

GnomeFont * XAP_UnixGnomePrintGraphics::_allocGnomeFont(PSFont* pFont)
{
	XAP_UnixFont *uf          = pFont->getUnixFont();
	XAP_UnixFont::style style = uf->getStyle();
	
	return gnome_font_find_closest_from_weight_slant(reinterpret_cast<const guchar *>(uf->getName()), getGnomeFontWeight(style), 
													 isItalic(style), (double)(pFont->getSize()));
}

XAP_UnixGnomePrintGraphics::~XAP_UnixGnomePrintGraphics()
{
	_destroyFonts ();

	if (m_pCurrentFont != NULL && GNOME_IS_FONT(m_pCurrentFont))
		gnome_font_unref(m_pCurrentFont);
}

UT_sint32 XAP_UnixGnomePrintGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	UT_UCSChar realChar;
	if(!m_bIsSymbol && !m_bIsDingbat)
	{
		realChar = c;
	}
	else if (m_bIsSymbol)
	{
		realChar = static_cast<UT_UCSChar>(adobeToUnicode(c));
	}
	else
	{
		realChar = c;
	}
	float fWidth = m_pCurrentPSFont->measureUnRemappedChar(realChar, m_pCurrentPSFont->getSize())
		* (float)getResolution() / (float)getDeviceResolution();
	return static_cast<UT_uint32>(rint(fWidth));
}

void XAP_UnixGnomePrintGraphics::drawGlyph (UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_ASSERT_NOT_REACHED ();
}

void XAP_UnixGnomePrintGraphics::getCoverage (UT_NumberVector& coverage)
{
	UT_ASSERT_NOT_REACHED ();
}

GnomeGlyphList * XAP_UnixGnomePrintGraphics::_createGlyphList ()
{
	GnomeGlyphList * glist;

	glist = gnome_glyphlist_new ();

	gnome_glyphlist_font (glist, m_pCurrentFont);

	gint color;

	color  = ((gint)m_currentColor.m_red) << 24;
	color |= ((gint)m_currentColor.m_grn) << 16;
	color |= ((gint)m_currentColor.m_blu) << 8;

	if (!m_currentColor.isTransparent ())
		color |= 0xff;

	gnome_glyphlist_color (glist, color);

	return glist;
}

void XAP_UnixGnomePrintGraphics::drawChars(const UT_UCSChar* pChars, 
										   int iCharOffset, int iLength,
										   UT_sint32 xoff, UT_sint32 yoff,
										   int * pCharWidths)
{
	if (!m_bStartPage)
		return;

	// keep x values (including widths) in layout units to minimize rounding errors, tdu as late as possible
	yoff = scale_ydir (tdu(yoff + getFontAscent()));

	gnome_print_gsave (m_gpc);
	gnome_print_moveto (m_gpc, 0, 0);

	XAP_UnixFont * uF = m_pCurrentPSFont->getUnixFont();
	XftFaceLocker locker(uF->getLayoutXftFont(GR_CharWidthsCache::CACHE_FONT_SIZE));			
	FT_Face pFace = locker.getFace();
	
	GnomeGlyphList * pGL = _createGlyphList ();
	UT_sint32 i, advance;
	for(i = iCharOffset, advance = 0; i < iLength; i++)
		{
			FT_UInt glyph_index;

			if (m_bIsSymbol)
				glyph_index = FT_Get_Char_Index(pFace, adobeToUnicode (pChars[iCharOffset + i]));
			else
				glyph_index = FT_Get_Char_Index(pFace, pChars[iCharOffset + i]);

			//			gnome_glyphlist_moveto (pGL, scale_xdir (tdu (xoff + advance)), yoff);
			gnome_glyphlist_moveto (pGL, tduD (xoff + advance), yoff);
			gnome_glyphlist_glyph (pGL, glyph_index);

			if (pCharWidths)
				advance += pCharWidths[i];
			else {
				advance += measureUnRemappedChar (pChars[i]);
			}
		}

	gnome_print_glyphlist  (m_gpc, pGL);
	gnome_glyphlist_unref (pGL);

	gnome_print_grestore (m_gpc);
}

void XAP_UnixGnomePrintGraphics::drawLine (UT_sint32 x1, UT_sint32 y1,
										   UT_sint32 x2, UT_sint32 y2)
{
	if (!m_bStartPage)
		return;

	gnome_print_moveto (m_gpc, scale_xdir (tdu(x1)), scale_ydir (tdu(y1)));
	gnome_print_lineto (m_gpc, scale_xdir (tdu(x2)), scale_ydir (tdu(y2)));
	gnome_print_stroke (m_gpc);
}

void XAP_UnixGnomePrintGraphics::setFont(GR_Font* pFont)
{
	PSFont * psFont = (static_cast<PSFont*> (pFont));
	m_pCurrentPSFont = psFont;
	m_bIsSymbol = false;
	m_bIsDingbat = false;
	char * szUnixFontName = UT_strdup(psFont->getUnixFont()->getName());
	const char * szFontName = UT_lowerString(szUnixFontName);

	if (szFontName)
	{
		if(strstr(szFontName,"symbol") != NULL)
		{
			if(strstr(szFontName,"star") != NULL)
				m_bIsSymbol = false;
			else
				m_bIsSymbol = true;
		}
		if(strstr(szFontName,"dingbat") != NULL)
			m_bIsDingbat = true;
	}
	FREEP(szFontName);

	if(m_pCurrentFont && GNOME_IS_FONT(m_pCurrentFont))
		gnome_font_unref (m_pCurrentFont);

	m_pCurrentFont = _allocGnomeFont(psFont);
}

bool XAP_UnixGnomePrintGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
		{
		case DGP_SCREEN:
		case DGP_OPAQUEOVERLAY:
			return false;
		case DGP_PAPER:
			return true;
		default:
			UT_ASSERT_NOT_REACHED ();
			return false;
		}
}

void XAP_UnixGnomePrintGraphics::getColor(UT_RGBColor& clr)
{
	clr = m_currentColor;
}

void XAP_UnixGnomePrintGraphics::setColor(const UT_RGBColor& clr)
{
	if (!m_bStartPrint || m_currentColor == clr)
		return;

	m_currentColor = clr;

	double red   = static_cast<double>(m_currentColor.m_red / 255.0);
	double green = static_cast<double>(m_currentColor.m_grn / 255.0);
	double blue  = static_cast<double>(m_currentColor.m_blu / 255.0);
	gnome_print_setrgbcolor(m_gpc,red,green,blue);
}

void XAP_UnixGnomePrintGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	if (!m_bStartPage)
		return;

 	m_dLineWidth = tduD(static_cast<double>(iLineWidth));
	gnome_print_setlinewidth (m_gpc, m_dLineWidth); 
}

bool XAP_UnixGnomePrintGraphics::startPrint(void)
{
	UT_ASSERT(!m_bStartPrint || !m_gpm);
	m_bStartPrint = true;
	return _startDocument();
}

bool XAP_UnixGnomePrintGraphics::startPage (const char *szPageLabel,
											UT_uint32 pageNo, bool portrait, 
											UT_uint32 width, UT_uint32 height)
{
	if (m_bStartPage)
		_endPage();
	m_bStartPage = true;
	m_bNeedStroked = false;
	return _startPage(szPageLabel);
}

bool XAP_UnixGnomePrintGraphics::endPrint()
{
	if (m_bStartPage)
		_endPage();
	return _endDocument();
}

void XAP_UnixGnomePrintGraphics::setColorSpace(GR_Graphics::ColorSpace c)
{
	m_cs = c;
}

GR_Graphics::ColorSpace XAP_UnixGnomePrintGraphics::getColorSpace(void) const
{
	return m_cs;
}

UT_uint32 XAP_UnixGnomePrintGraphics::getDeviceResolution(void) const
{
	return _getResolution();
}

void XAP_UnixGnomePrintGraphics::_drawAnyImage (GR_Image* pImg, 
												UT_sint32 xDest, 
												UT_sint32 yDest, bool rgb)
{
	UT_sint32 iDestWidth  = pImg->getDisplayWidth ();
	UT_sint32 iDestHeight = pImg->getDisplayHeight ();
	
	GR_UnixImage * pImage = static_cast<GR_UnixImage *>(pImg);
	GdkPixbuf * image = pImage->getData ();
	UT_return_if_fail (image);
	
	gint width, height, rowstride;
	const guchar * pixels;
	
	/* The pixbuf should contain unscaled (raw) image data for best results. 
	See XAP_UnixGnomePrintGraphics::createNewImage for details */
	width     = gdk_pixbuf_get_width (image);
	height    = gdk_pixbuf_get_height (image);
	rowstride = gdk_pixbuf_get_rowstride (image);
	pixels    = gdk_pixbuf_get_pixels (image);

	gnome_print_gsave (m_gpc);
	gnome_print_translate (m_gpc, xDest, yDest - iDestHeight);

	//float scale_x = static_cast<float>(iDestWidth)/width;
	//float scale_y = static_cast<float>(iDestHeight)/height;
	gnome_print_scale (m_gpc, iDestWidth, iDestHeight);

	/* Not sure about the grayimage part, but the other 2 are correct */
	if (!rgb)
		gnome_print_grayimage (m_gpc, pixels, width, height, rowstride);
	else if (gdk_pixbuf_get_has_alpha (image))
		gnome_print_rgbaimage (m_gpc, pixels, width, height, rowstride);
	else
		gnome_print_rgbimage (m_gpc, pixels, width, height, rowstride);

	gnome_print_grestore(m_gpc);
}

void XAP_UnixGnomePrintGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, 
										   UT_sint32 yDest)
{
	if (!m_bStartPage)
		return;

	xDest = scale_xdir (tdu(xDest));
	yDest = scale_ydir (tdu(yDest));

   	if (pImg->getType() != GR_Image::GRT_Raster) 
	    pImg->render(this, xDest, yDest);
	else {
		switch(m_cs)
			{
			case GR_Graphics::GR_COLORSPACE_COLOR:
				_drawAnyImage(pImg, xDest, yDest, true);
				break;
			case GR_Graphics::GR_COLORSPACE_GRAYSCALE:
			case GR_Graphics::GR_COLORSPACE_BW:
				_drawAnyImage(pImg, xDest, yDest, false);
				break;
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
	}
}

/*!
Creates and sets up a new image for printing.
@param iDisplayWidth the width of the image to display. Values are in layout units.
@param iDisplayHeight the height of the image to display. Values are in layout units.
@return a new image if successful, NULL otherwise
*/
GR_Image* XAP_UnixGnomePrintGraphics::createNewImage(const char* pszName, 
													 const UT_ByteBuf* pBB, 
													 UT_sint32 iDisplayWidth,
													 UT_sint32 iDisplayHeight, 
													 GR_Image::GRType iType)
{
	GR_Image* pImg = NULL;

   	if (iType == GR_Image::GRT_Raster)
		pImg = new GR_UnixImage(pszName,iType);
   	else if (iType == GR_Image::GRT_Vector)
		pImg = new GR_VectorImage(pszName);
	
	// make sure we don't scale the image yet to not loose any information
	pImg->convertFromBuffer(pBB, -1, -1);
	pImg->setDisplaySize(tdu(iDisplayWidth), tdu(iDisplayHeight));

	return pImg;
}

/***********************************************************************/
/*                          Protected things                           */
/***********************************************************************/

bool XAP_UnixGnomePrintGraphics::_startDocument(void)
{
	return true;
}

bool XAP_UnixGnomePrintGraphics::_startPage(const char * szPageLabel)
{
	if (!m_gpm)
		return true ;
	
	gnome_print_beginpage(m_gpc, reinterpret_cast<const guchar *>(szPageLabel));

	return true;
}

bool XAP_UnixGnomePrintGraphics::_endPage(void)
{
	if(m_bNeedStroked)
		gnome_print_stroke(m_gpc);
	
	if (!m_gpm)
		return true;

	gnome_print_showpage(m_gpc);
	return true;
}

bool XAP_UnixGnomePrintGraphics::_endDocument(void)
{
	if(!m_gpm)
		return true;

	gnome_print_job_close(m_gpm);
		
	if(!m_bIsPreview)
		gnome_print_job_print(m_gpm);
	else
		{
			const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
			GtkWidget * preview = gnome_print_job_preview_new (m_gpm, 
															   reinterpret_cast<const guchar *>(pSS->getValue(XAP_STRING_ID_DLG_UP_PrintPreviewTitle)));
			gtk_widget_show(GTK_WIDGET(preview));

			// To center the dialog, we need the frame of its parent.
			XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(XAP_App::getApp()->getLastFocussedFrame()->getFrameImpl());
			
			// Get the GtkWindow of the parent frame
			GtkWidget * parentWindow = pUnixFrameImpl->getTopLevelWindow();

			centerDialog(parentWindow, preview);
		}
	
	g_object_unref(G_OBJECT(m_gpm));
	return true;
}

UT_uint32 XAP_UnixGnomePrintGraphics::_getResolution(void) const
{
	return 72; // was 72
}

void XAP_UnixGnomePrintGraphics::fillRect(const UT_RGBColor& c, 
										  UT_sint32 x, UT_sint32 y, 
										  UT_sint32 w, UT_sint32 h)
{
	if (!m_bStartPage)
		return;

	// set the bgcolor
	UT_RGBColor old (m_currentColor);
	setColor (c);
	double dx,dy,dw,dh;
	dx = tduD(x); dy = static_cast<double>(scale_ydir (tdu(y))); dw = tduD(w); dh = tduD(h);

	gnome_print_newpath (m_gpc);
	gnome_print_moveto (m_gpc, dx,   dy);		
	gnome_print_lineto (m_gpc, dx+dw, dy);
	gnome_print_lineto (m_gpc, dx+dw, dy-dh);
	gnome_print_lineto (m_gpc, dx,   dy-dh);
	gnome_print_lineto (m_gpc, dx,   dy);
	gnome_print_closepath (m_gpc);
	gnome_print_fill (m_gpc);
	
	// reset color to its original state
	setColor (old);
}

void XAP_UnixGnomePrintGraphics::setClipRect(const UT_Rect* pRect)
{
}

/***********************************************************************/
/*                    Things that souldn't happen                      */
/***********************************************************************/

void XAP_UnixGnomePrintGraphics::setCursor(GR_Graphics::Cursor)
{
	UT_ASSERT_NOT_REACHED ();
}

GR_Graphics::Cursor XAP_UnixGnomePrintGraphics::getCursor(void) const
{
	UT_ASSERT_NOT_REACHED ();
	return GR_CURSOR_INVALID;
}

void XAP_UnixGnomePrintGraphics::xorLine(UT_sint32, UT_sint32, UT_sint32, 
										 UT_sint32)
{
	UT_ASSERT_NOT_REACHED ();
}

void XAP_UnixGnomePrintGraphics::polyLine(UT_Point * /* pts */, 
										  UT_uint32 /* nPoints */)
{
	UT_ASSERT_NOT_REACHED ();
}

void XAP_UnixGnomePrintGraphics::invertRect(const UT_Rect* /*pRect*/)
{
	UT_ASSERT_NOT_REACHED ();
}

void XAP_UnixGnomePrintGraphics::clearArea(UT_sint32 /*x*/, UT_sint32 /*y*/,
										   UT_sint32 /*width*/, UT_sint32 /*height*/)
{
	UT_ASSERT_NOT_REACHED ();
}

void XAP_UnixGnomePrintGraphics::scroll(UT_sint32, UT_sint32)
{
	UT_ASSERT_NOT_REACHED ();
}

void XAP_UnixGnomePrintGraphics::scroll(UT_sint32 /* x_dest */,
										UT_sint32 /* y_dest */,
										UT_sint32 /* x_src */,
										UT_sint32 /* y_src */,
										UT_sint32 /* width */,
										UT_sint32 /* height */)
{
	UT_ASSERT_NOT_REACHED ();
}

UT_RGBColor * XAP_UnixGnomePrintGraphics::getColor3D(GR_Color3D /*c*/)
{
	UT_ASSERT_NOT_REACHED ();
	return NULL;
}

void XAP_UnixGnomePrintGraphics::setColor3D(GR_Color3D /*c*/)
{
	UT_ASSERT_NOT_REACHED ();
}

GR_Font* XAP_UnixGnomePrintGraphics::getGUIFont()
{
	UT_ASSERT_NOT_REACHED ();
	return NULL;
}

void XAP_UnixGnomePrintGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT_NOT_REACHED ();
}

void XAP_UnixGnomePrintGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT_NOT_REACHED ();
}

void XAP_UnixGnomePrintGraphics::setPageSize(char* pageSizeName, UT_uint32 iwidth, UT_uint32 iheight)
{
	UT_ASSERT_NOT_REACHED ();
}

/***********************************************************************/
/*                                Fonts                                */
/***********************************************************************/

UT_uint32 XAP_UnixGnomePrintGraphics::getFontAscent(GR_Font *fnt)
{
	PSFont*	hndl = static_cast<PSFont*> (fnt);
	// FIXME we should really be getting stuff from the font in layout units,
	// FIXME but we're not smart enough to do that yet
	// we call getDeviceResolution() to avoid zoom
   	return static_cast<UT_uint32>(hndl->getUnixFont()->getAscender(hndl->getSize()) * getResolution() / getDeviceResolution() + 0.5);
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontAscent()
{
	return getFontAscent(static_cast<GR_Font *>(m_pCurrentPSFont));
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontDescent(GR_Font *fnt)
{
	PSFont*	psfnt = static_cast<PSFont*> (fnt);
	// FIXME we should really be getting stuff from the font in layout units,
	// FIXME but we're not smart enough to do that yet
   	return static_cast<UT_uint32>(psfnt->getUnixFont()->getDescender(psfnt->getSize()) * getResolution() / getDeviceResolution() + 0.5);
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontDescent()
{
	return getFontDescent(static_cast<GR_Font *>(m_pCurrentPSFont));
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontHeight()
{
	return (getFontAscent() + getFontDescent());
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontHeight(GR_Font *fnt)
{
	return (getFontAscent(fnt) + getFontDescent(fnt));
}

GR_Font* XAP_UnixGnomePrintGraphics::_findFont(const char* pszFontFamily, 
											   const char* pszFontStyle, 
											   const char* pszFontVariant,
											   const char* pszFontWeight, 
											   const char* pszFontStretch,
											   const char* pszFontSize,
											   const char* pszLang)
{
	XAP_UnixFont* pUnixFont = m_fm->findNearestFont(pszFontFamily, pszFontStyle, 
													pszFontVariant, pszFontWeight,
													pszFontStretch, pszFontSize);

	UT_uint32 iSize = static_cast<UT_uint32>(UT_convertToPoints(pszFontSize)*getDeviceResolution()/72.);

	XAP_UnixFontHandle* item = new XAP_UnixFontHandle(pUnixFont, iSize);
	UT_ASSERT(item);

	PSFont * pFont = new PSFont(item->getUnixFont(), iSize);
	UT_ASSERT(pFont);
	delete item;

	return pFont;
}

static int
joinToPS (GR_Graphics::JoinStyle js)
{
  switch(js)
    {
    case GR_Graphics::JOIN_MITER: 
      return 0;
    case GR_Graphics::JOIN_ROUND: 
      return 1;
    case GR_Graphics::JOIN_BEVEL: 
      return 2;
    }

  return 1;
}

static int
capToPS (GR_Graphics::CapStyle cs)
{
  switch (cs)
    {
    case GR_Graphics::CAP_BUTT: 
      return 0;
    case GR_Graphics::CAP_ROUND: 
      return 1;
    case GR_Graphics::CAP_PROJECTING: 
      return 2;
    }

  return 1;
}

static const double*
dashToPS (GR_Graphics::LineStyle ls, gint & n_values, double &offset)
{
	static const double on_off_dash [] = {1., 1.};
	static const double double_dash [] = {1., 2.};

	switch(ls)
		{
		case GR_Graphics::LINE_SOLID:
			offset = 0.; n_values = 0; return NULL;
		case GR_Graphics::LINE_ON_OFF_DASH: 
			offset = 0.; n_values = 2; return on_off_dash;
		case GR_Graphics::LINE_DOUBLE_DASH: 
			offset = 0.; n_values = 2; return double_dash;
		case GR_Graphics::LINE_DOTTED: 
			UT_ASSERT(UT_TODO);
			offset = 0.; n_values = 0; return NULL;
		}
	
	n_values = 0; offset = 0.; return NULL;
}

void XAP_UnixGnomePrintGraphics::setLineProperties (double inWidthPixels,
													JoinStyle inJoinStyle,
													CapStyle inCapStyle,
													LineStyle inLineStyle)
{
	if (!m_bStartPage)
		return;

	gnome_print_setlinejoin (m_gpc, joinToPS(inJoinStyle));
	gnome_print_setlinecap (m_gpc, capToPS(inCapStyle));

	gint n_values = 0;
	double offset = 0;
	const double * dash = NULL;

	dash = dashToPS (inLineStyle, n_values, offset);
	gnome_print_setdash (m_gpc, n_values, dash, offset);
}

/***********************************************************************/
/*                Private Scaling Conversion Routines                  */
/***********************************************************************/

UT_sint32 XAP_UnixGnomePrintGraphics::scale_ydir (UT_sint32 in)
{
	double height;

	if (isPortrait())
		height = m_height;
	else
		height = m_width;
	return static_cast<UT_sint32>(height - static_cast<double>(in));
}

UT_sint32 XAP_UnixGnomePrintGraphics::scale_xdir (UT_sint32 in)
{
	return in;
}
