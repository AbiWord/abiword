/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2007 Tomas Frydrych
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gr_UnixPangoGraphics.h"
#include "gr_Painter.h"

#include "xap_App.h"
#include "xap_Prefs.h"
#include "xap_EncodingManager.h"
#include "xap_Strings.h"
#include "xap_Frame.h"

#include "xap_UnixApp.h"
#include "xap_UnixFrameImpl.h"
#include "xap_UnixDialogHelper.h"

#include "gr_UnixImage.h"

#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_vector.h"
#include "ut_locale.h"

// need this to include what Pango considers 'low-level' api
#define PANGO_ENABLE_ENGINE

#include <pango/pango-item.h>
#include <pango/pango-engine.h>
#include <pango/pangoxft.h>
#include <pango/pangofc-fontmap.h>

#ifdef HAVE_PANGOFT2
  #include <pango/pangoft2.h>
#endif

#include <math.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#ifdef ENABLE_PRINT
  #include <libgnomeprint/gnome-print-pango.h>
  #include <libgnomeprint/gnome-print-paper.h>
  #include <libgnomeprintui/gnome-print-job-preview.h>
#endif

UT_uint32 adobeDingbatsToUnicode(UT_uint32 iAdobe);
UT_uint32 adobeToUnicode(UT_uint32 iAdobe);

float fontPoints2float(UT_uint32 iSize, FT_Face pFace, UT_uint32 iFontPoints)
{
	if(pFace == NULL)
		return 0.0;
	float rat = 1.44; // convert from points to inches
	return static_cast<float>(iFontPoints) * static_cast<float>(iSize) * rat  /
		pFace->units_per_EM;
}

UT_uint32      GR_UnixPangoGraphics::s_iInstanceCount = 0;
UT_VersionInfo GR_UnixPangoGraphics::s_Version;
int            GR_UnixPangoGraphics::s_iMaxScript = 0;


static void _pango_item_list_free(GList * items) 
{
	GList * l;
	for( l = items; l ; l = l->next) {
		if(l->data) {
			pango_item_free(static_cast<PangoItem*>(l->data));
			l->data = NULL;
		}
	}
	g_list_free(items);
}



class GR_UnixPangoItem: public GR_Item
{
	friend class GR_UnixPangoGraphics;
	friend class GR_UnixPangoPrintGraphics;
	friend class GR_UnixPangoRenderInfo;
	
  public:
	virtual ~GR_UnixPangoItem(){ if (m_pi) {pango_item_free(m_pi);}};
	
	virtual GR_ScriptType getType() const {return (GR_ScriptType)m_iType;}
	
	virtual GR_Item *     makeCopy() const
	    {
			return new GR_UnixPangoItem(pango_item_copy(m_pi));
		}
	
	virtual GRRI_Type     getClassId() const {return GRRI_UNIX_PANGO;}

  protected:
	GR_UnixPangoItem(PangoItem *pi);
	GR_UnixPangoItem() : m_pi(NULL) { }; // just a dummy used to terminate
	                                     // GR_Itemization list

	PangoItem *m_pi;
	UT_uint32 m_iType;
};

GR_UnixPangoItem::GR_UnixPangoItem(PangoItem *pi):
	m_pi(pi)
{
	// there does not seem to be anything that we could use to identify the
	// items, so we will hash the pointers to the two text engines
	if(!pi)
	{
		m_iType = (UT_uint32)GRScriptType_Void;
	}
	else
	{
		// there does not seem to be anything that we could use to easily
		// identify the script, so we will hash the pointers to the two text
		// engines
		
		void * b[2];
		b[0] = (void*)pi->analysis.shape_engine;
		b[1] = (void*)pi->analysis.lang_engine;

		m_iType = UT_hash32((const char *) &b, 2 * sizeof(void*));
	}
}

class GR_UnixPangoRenderInfo : public GR_RenderInfo
{
  public:
	GR_UnixPangoRenderInfo(GR_ScriptType t):
		GR_RenderInfo(t),
		m_pGlyphs(NULL),
		m_pScaledGlyphs(NULL),
		m_pLogOffsets(NULL),
		m_pJustify(NULL),
		m_iZoom(0),
		m_iCharCount(0),
		m_iShapingAllocNo(0)
	{
		++s_iInstanceCount;
		if(sUTF8 == NULL)
			sUTF8 = new UT_UTF8String("");
	};

	virtual ~GR_UnixPangoRenderInfo()
	{
		delete [] m_pJustify; delete [] m_pLogOffsets;
		if(m_pGlyphs)
			pango_glyph_string_free(m_pGlyphs);
		if(m_pScaledGlyphs)
			pango_glyph_string_free(m_pScaledGlyphs);
		s_iInstanceCount--;

		if(!s_iInstanceCount)
		{
			delete [] s_pLogAttrs;
			s_pLogAttrs = NULL;
			DELETEP(sUTF8);
		}
	};

	virtual GRRI_Type getType() const {return GRRI_UNIX_PANGO;}
	virtual bool append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool split (GR_RenderInfo *&pri, bool bReverse = false);
	virtual bool cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false);
	virtual bool isJustified() const;
	virtual bool canAppend(GR_RenderInfo &ri) const;

	bool getUTF8Text();
	
	inline bool       allocStaticBuffers(UT_uint32 iSize)
	    {
			if(s_pLogAttrs)
				delete [] s_pLogAttrs;

			s_pLogAttrs = new PangoLogAttr[iSize];

			if(!s_pLogAttrs)
				return false;
			
			s_iStaticSize = iSize;
			return true;
	    }

	PangoGlyphString* m_pGlyphs;
	PangoGlyphString* m_pScaledGlyphs;
	int *             m_pLogOffsets;
	int *             m_pJustify;
	UT_uint32         m_iZoom;
	UT_uint32         m_iCharCount;
	UT_uint32         m_iShapingAllocNo;
	
	static UT_UTF8String * sUTF8;
	static GR_UnixPangoRenderInfo * s_pOwnerUTF8;
	static UT_uint32  s_iInstanceCount;
	static UT_uint32  s_iStaticSize;  // size of the static buffers

	static PangoLogAttr *           s_pLogAttrs;
	static GR_UnixPangoRenderInfo * s_pOwnerLogAttrs;
};


GR_UnixPangoRenderInfo * GR_UnixPangoRenderInfo::s_pOwnerUTF8 = NULL;
UT_UTF8String *          GR_UnixPangoRenderInfo::sUTF8 = NULL;
UT_uint32                GR_UnixPangoRenderInfo::s_iInstanceCount = 0;
UT_uint32                GR_UnixPangoRenderInfo::s_iStaticSize = 0;
GR_UnixPangoRenderInfo * GR_UnixPangoRenderInfo::s_pOwnerLogAttrs = NULL;
PangoLogAttr *           GR_UnixPangoRenderInfo::s_pLogAttrs = NULL;


bool GR_UnixPangoRenderInfo::getUTF8Text()
{
	if(s_pOwnerUTF8 == this)
		return true;
	
	UT_return_val_if_fail( m_pText, false );

	UT_TextIterator & text = *m_pText;
	sUTF8->clear();

	// we intentionally run this as far as the iterator lets us, even if that is
	// past the end of this item
	for(; text.getStatus() == UTIter_OK; ++text)
	{
		*sUTF8 += text.getChar();
	}

	s_pOwnerUTF8 = this;

	return true;
}

GR_UnixPangoGraphics::GR_UnixPangoGraphics(GdkWindow * win)
	:
	 m_pFontMap(NULL),
	 m_pContext(NULL),
	 m_pLayoutFontMap(NULL),
	 m_pLayoutContext(NULL),
	 m_bOwnsFontMap(false),
	 m_pPFont(NULL),
	 m_pPFontGUI(NULL),
	 m_pAdjustedPangoFont(NULL),
	 m_pAdjustedLayoutPangoFont(NULL),
	 m_pAdjustedPangoFontSource(NULL),
	 m_iAdjustedPangoFontZoom (0),
	 m_iDeviceResolution(96),
	 m_pWin (win),
 	 m_pGC (NULL),
	 m_pXORGC (NULL),
	 m_pVisual (NULL),
	 m_pXftDraw (NULL),
	 m_iXoff (0),
	 m_iYoff (0),
	 m_bIsSymbol (false),
	 m_bIsDingbat (false)
{
	init ();
}


GR_UnixPangoGraphics::GR_UnixPangoGraphics()
	:
	 m_pFontMap(NULL),
	 m_pContext(NULL),
	 m_pLayoutFontMap(NULL),
	 m_pLayoutContext(NULL),
	 m_bOwnsFontMap(false),
	 m_pPFont(NULL),
	 m_pPFontGUI(NULL),
	 m_pAdjustedPangoFont(NULL),
	 m_pAdjustedLayoutPangoFont(NULL),
	 m_pAdjustedPangoFontSource(NULL),
	 m_iAdjustedPangoFontZoom (0),
	 m_iDeviceResolution(96),
	 m_pWin (NULL),
 	 m_pGC (NULL),
	 m_pXORGC (NULL),
	 m_pVisual (NULL),
	 m_pXftDraw (NULL),
	 m_iXoff (0),
	 m_iYoff (0),
	 m_bIsSymbol (false),
	 m_bIsDingbat (false)
{
	init ();
}


GR_UnixPangoGraphics::~GR_UnixPangoGraphics()
{
	xxx_UT_DEBUGMSG(("Deleting UnixPangoGraphics %x \n",this));
	if(m_pAdjustedPangoFont!= NULL)
	{
		g_object_unref(m_pAdjustedPangoFont);
	}
	if(m_pAdjustedLayoutPangoFont!= NULL)
	{
		g_object_unref(m_pAdjustedLayoutPangoFont);
	}
	if (m_pContext != NULL)
	{
		g_object_unref(m_pContext);
	}
	// NB: m_pFontMap is oft owned by Pango
	if (m_bOwnsFontMap)
		g_object_unref(m_pFontMap);

	_destroyFonts();
	delete m_pPFontGUI;
	g_object_unref(m_pLayoutContext);
	if(m_pLayoutFontMap) {
		// see bug http://bugzilla.gnome.org/show_bug.cgi?id=143542
		pango_fc_font_map_cache_clear((PangoFcFontMap*)m_pLayoutFontMap);
		g_object_unref(m_pLayoutFontMap);
	}

	if (m_pXftDraw)
		g_free(m_pXftDraw);

	UT_VECTOR_SPARSEPURGEALL( UT_Rect*, m_vSaveRect);

	// purge saved pixbufs
	for (UT_uint32 i = 0; i < m_vSaveRectBuf.size (); i++)
	{
		GdkPixbuf * pix = static_cast<GdkPixbuf *>(m_vSaveRectBuf.getNthItem(i));
		g_object_unref (G_OBJECT (pix));
	}

	if (G_IS_OBJECT(m_pGC))
		g_object_unref (G_OBJECT(m_pGC));
	if (G_IS_OBJECT(m_pXORGC))
		g_object_unref (G_OBJECT(m_pXORGC));
	
}

void GR_UnixPangoGraphics::init()
{
	xxx_UT_DEBUGMSG(("Initializing UnixPangoGraphics %x \n",this));
	GdkDisplay * gDisplay = NULL;
	GdkScreen *  gScreen = NULL;

	if (_getDrawable())
	{
		m_pColormap = gdk_rgb_get_colormap();
		m_Colormap = GDK_COLORMAP_XCOLORMAP(m_pColormap);

		gDisplay = gdk_drawable_get_display(_getDrawable());
		gScreen = gdk_drawable_get_screen(_getDrawable());

		GdkDrawable * realDraw;
		if(GDK_IS_WINDOW((_getDrawable())))
		{
				gdk_window_get_internal_paint_info (_getDrawable(), &realDraw,
											&m_iXoff, &m_iYoff);
		}
		else
		{
			    realDraw = _getDrawable();
				m_iXoff = 0;
				m_iYoff = 0;
		}

		//
		// Martin's attempt to make double buffering work.with xft
		//
		m_pGC = gdk_gc_new(realDraw);
		m_pXORGC = gdk_gc_new(realDraw);
		m_pVisual = GDK_VISUAL_XVISUAL( gdk_drawable_get_visual(realDraw));
		m_Drawable = gdk_x11_drawable_get_xid(realDraw);

		m_pXftDraw = XftDrawCreate(GDK_DISPLAY(), m_Drawable,
								   m_pVisual, m_Colormap);
			
		gdk_gc_set_function(m_pXORGC, GDK_XOR);

		GdkColor clrWhite;
		clrWhite.red = clrWhite.green = clrWhite.blue = 65535;
		gdk_colormap_alloc_color (m_pColormap, &clrWhite, FALSE, TRUE);
		gdk_gc_set_foreground(m_pXORGC, &clrWhite);

		GdkColor clrBlack;
		clrBlack.red = clrBlack.green = clrBlack.blue = 0;
		gdk_colormap_alloc_color (m_pColormap, &clrBlack, FALSE, TRUE);
		gdk_gc_set_foreground(m_pGC, &clrBlack);

		m_XftColor.color.red = clrBlack.red;
		m_XftColor.color.green = clrBlack.green;
		m_XftColor.color.blue = clrBlack.blue;
		m_XftColor.color.alpha = 0xffff;
		m_XftColor.pixel = clrBlack.pixel;

		// I only want to set CAP_NOT_LAST, but the call takes all
		// arguments (and doesn't have a default value).  Set the
		// line attributes to not draw the last pixel.

		// We force the line width to be zero because the CAP_NOT_LAST
		// stuff does not seem to work correctly when the width is set
		// to one.

		gdk_gc_set_line_attributes(m_pGC, 0,
								   GDK_LINE_SOLID,
								   GDK_CAP_NOT_LAST,
								   GDK_JOIN_MITER);
			
		gdk_gc_set_line_attributes(m_pXORGC, 0,
								   GDK_LINE_SOLID,
								   GDK_CAP_NOT_LAST,
								   GDK_JOIN_MITER);

		// Set GraphicsExposes so that XCopyArea() causes an expose on
		// obscured regions rather than just tiling in the default background.
		gdk_gc_set_exposures(m_pGC, 1);
		gdk_gc_set_exposures(m_pXORGC, 1);

		m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
		m_cursor = GR_CURSOR_INVALID;
		setCursor(GR_CURSOR_DEFAULT);
		
	}
	else
	{
		gDisplay = gdk_display_get_default();
		gScreen = gdk_screen_get_default();
	}

	
	m_bIsSymbol = false;
	m_bIsDingbat = false;

	bool bGotResolution = false;
	
	if (gScreen && gDisplay)
		{
			int iScreen = gdk_x11_screen_get_screen_number(gScreen);
			Display * disp = GDK_DISPLAY_XDISPLAY(gDisplay);
			m_pContext = pango_xft_get_context(disp, iScreen);
			m_pFontMap = pango_xft_get_font_map(disp, iScreen);

			FcPattern *pattern = FcPatternCreate();
			if (pattern)
			{
				double dpi;
				XftDefaultSubstitute (GDK_SCREEN_XDISPLAY (gScreen),
									  iScreen,
									  pattern);

				if(FcResultMatch == FcPatternGetDouble (pattern,
														FC_DPI, 0, &dpi))
				{
					m_iDeviceResolution = (UT_uint32)round(dpi);
					bGotResolution = true;
				}

				FcPatternDestroy (pattern);
			}
			
			if (!bGotResolution)
			{
				// that didn't work. try getting it from the screen
				m_iDeviceResolution =
					(UT_uint32)round((gdk_screen_get_width(gScreen) * 25.4) /
									 gdk_screen_get_width_mm (gScreen));
			}

			UT_DEBUGMSG(("@@@@@@@@@@@@@ retrieved DPI %d @@@@@@@@@@@@@@@@@ \n",
						 m_iDeviceResolution));
			
		}
#ifdef HAVE_PANGOFT2
	else
	{
		m_iDeviceResolution = 72.;
		m_pFontMap = pango_ft2_font_map_new ();
		pango_ft2_font_map_set_resolution(reinterpret_cast<PangoFT2FontMap*>(m_pFontMap), 
										  m_iDeviceResolution,
										  m_iDeviceResolution);	
		m_pContext = pango_ft2_font_map_create_context(reinterpret_cast<PangoFT2FontMap*>(m_pFontMap));
		m_bOwnsFontMap = true;
	}
	m_pLayoutFontMap = pango_ft2_font_map_new ();
	pango_ft2_font_map_set_resolution(reinterpret_cast<PangoFT2FontMap*>(m_pLayoutFontMap), 
									  getResolution(),
									  getResolution());	
	m_pLayoutContext = pango_ft2_font_map_create_context(reinterpret_cast<PangoFT2FontMap*>(m_pLayoutFontMap));
#endif
}

bool GR_UnixPangoGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
		case DGP_SCREEN:
		case DGP_OPAQUEOVERLAY:
			return true;
		case DGP_PAPER:
			return false;
		default:
			UT_ASSERT(0);
			return false;
	}
}

GR_Graphics *   GR_UnixPangoGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_UNIX, NULL);
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::graphicsAllocator\n"));

	UT_return_val_if_fail(!info.isPrinterGraphics(), NULL);
	GR_UnixAllocInfo &AI = (GR_UnixAllocInfo&)info;

	return new GR_UnixPangoGraphics(AI.m_win);
}

void GR_UnixPangoGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	GR_Painter caretDisablerPainter(this); // not an elegant way to disable all carets, but it works beautifully - MARCM
	UT_sint32 oldDY = tdu(getPrevYOffset());
	UT_sint32 oldDX = tdu(getPrevXOffset());
	UT_sint32 newY = getPrevYOffset() + dy;
	UT_sint32 newX = getPrevXOffset() + dx;
	UT_sint32 ddx = -(tdu(newX) - oldDX);
	UT_sint32 ddy = -(tdu(newY) - oldDY);
	setPrevYOffset(newY);
	setPrevXOffset(newX);
	if(ddx == 0 && ddy == 0)
	{
		return;
	}
	UT_sint32 iddy = labs(ddy);
	bool bEnableSmooth = XAP_App::getApp()->isSmoothScrollingEnabled();
	bEnableSmooth = bEnableSmooth && (iddy < 30) && (ddx == 0);
	if(bEnableSmooth)
	{
		if(ddy < 0)
		{
			UT_sint32 i = 0;
			for(i = 0; i< iddy; i++)
			{
				gdk_window_scroll(m_pWin,0,-1);
			}
		}
		else
		{
			UT_sint32 i = 0;
			for(i = 0; i< iddy; i++)
			{
				gdk_window_scroll(m_pWin,0,1);
			}
		}
	}
	else
	{
		gdk_window_scroll(m_pWin,ddx,ddy);
	}
	setExposePending(true);
}

bool GR_UnixPangoGraphics::startPrint(void)
{
	UT_ASSERT(0);
	return false;
}

bool GR_UnixPangoGraphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
								bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	UT_ASSERT(0);
	return false;
}

bool GR_UnixPangoGraphics::endPrint(void)
{
	UT_ASSERT(0);
	return false;
}

void GR_UnixPangoGraphics::drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
	drawChars(&Char, 0, 1, xoff, yoff, NULL);
}

void GR_UnixPangoGraphics::setColorSpace(GR_Graphics::ColorSpace /* c */)
{
	// we only use ONE color space here now (GdkRGB's space)
	// and we don't let people change that on us.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

GR_Graphics::ColorSpace GR_UnixPangoGraphics::getColorSpace(void) const
{
	return m_cs;
}

void GR_UnixPangoGraphics::setCursor(GR_Graphics::Cursor c)
{
	if (m_cursor == c)
		return;

	m_cursor = c;

	GdkCursorType cursor_number;

	switch (c)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor_number = GDK_LEFT_PTR;
		break;

	case GR_CURSOR_IBEAM:
		cursor_number = GDK_XTERM;
		break;

	//I have changed the shape of the arrow so get a consistent
	//behaviour in the bidi build; I think the new arrow is better
	//for the purpose anyway

	case GR_CURSOR_RIGHTARROW:
		cursor_number = GDK_SB_RIGHT_ARROW; //GDK_ARROW;
		break;

	case GR_CURSOR_LEFTARROW:
		cursor_number = GDK_SB_LEFT_ARROW; //GDK_LEFT_PTR;
		break;

	case GR_CURSOR_IMAGE:
		cursor_number = GDK_FLEUR;
		break;

	case GR_CURSOR_IMAGESIZE_NW:
		cursor_number = GDK_TOP_LEFT_CORNER;
		break;

	case GR_CURSOR_IMAGESIZE_N:
		cursor_number = GDK_TOP_SIDE;
		break;

	case GR_CURSOR_IMAGESIZE_NE:
		cursor_number = GDK_TOP_RIGHT_CORNER;
		break;

	case GR_CURSOR_IMAGESIZE_E:
		cursor_number = GDK_RIGHT_SIDE;
		break;

	case GR_CURSOR_IMAGESIZE_SE:
		cursor_number = GDK_BOTTOM_RIGHT_CORNER;
		break;

	case GR_CURSOR_IMAGESIZE_S:
		cursor_number = GDK_BOTTOM_SIDE;
		break;

	case GR_CURSOR_IMAGESIZE_SW:
		cursor_number = GDK_BOTTOM_LEFT_CORNER;
		break;

	case GR_CURSOR_IMAGESIZE_W:
		cursor_number = GDK_LEFT_SIDE;
		break;

	case GR_CURSOR_LEFTRIGHT:
		cursor_number = GDK_SB_H_DOUBLE_ARROW;
		break;

	case GR_CURSOR_UPDOWN:
		cursor_number = GDK_SB_V_DOUBLE_ARROW;
		break;

	case GR_CURSOR_EXCHANGE:
		cursor_number = GDK_EXCHANGE;
		break;

	case GR_CURSOR_GRAB:
		cursor_number = GDK_HAND1;
		break;

	case GR_CURSOR_LINK:
		cursor_number = GDK_HAND2;
		break;

	case GR_CURSOR_WAIT:
		cursor_number = GDK_WATCH;
		break;

	case GR_CURSOR_HLINE_DRAG:
		cursor_number = GDK_SB_V_DOUBLE_ARROW;
		break;

	case GR_CURSOR_VLINE_DRAG:
		cursor_number = GDK_SB_H_DOUBLE_ARROW;
		break;

	case GR_CURSOR_CROSSHAIR:
		cursor_number = GDK_CROSSHAIR;
		break;

	case GR_CURSOR_DOWNARROW:
		cursor_number = GDK_SB_DOWN_ARROW;
		break;

	case GR_CURSOR_DRAGTEXT:
		cursor_number = GDK_TARGET;
		break;

	case GR_CURSOR_COPYTEXT:
		cursor_number = GDK_DRAPED_BOX;
		break;
	}
	xxx_UT_DEBUGMSG(("cursor set to %d  gdk %d \n",c,cursor_number));
	GdkCursor * cursor = gdk_cursor_new(cursor_number);
	gdk_window_set_cursor(m_pWin, cursor);
	gdk_cursor_unref(cursor);
}

void GR_UnixPangoGraphics::createPixmapFromXPM( char ** pXPM,GdkPixmap *source,
										   GdkBitmap * mask)
{
	source
		= gdk_pixmap_colormap_create_from_xpm_d(_getDrawable(),NULL,
							&mask, NULL,
							pXPM);
}

GR_Graphics::Cursor GR_UnixPangoGraphics::getCursor(void) const
{
	return m_cursor;
}

void GR_UnixPangoGraphics::setColor3D(GR_Color3D c)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	_setColor(m_3dColors[c]);
}

bool GR_UnixPangoGraphics::getColor3D(GR_Color3D name, UT_RGBColor &color)
{
	if (m_bHave3DColors) {
		color.m_red = m_3dColors[name].red >> 8;
		color.m_grn = m_3dColors[name].green >> 8;
		color.m_blu =m_3dColors[name].blue >> 8;
		return true;
	}
	return false;
}

void GR_UnixPangoGraphics::init3dColors(GtkStyle * pStyle)
{
	m_3dColors[CLR3D_Foreground] = pStyle->text[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_Background] = pStyle->bg[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_BevelUp]    = pStyle->light[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_BevelDown]  = pStyle->dark[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_Highlight]  = pStyle->bg[GTK_STATE_PRELIGHT];

	m_bHave3DColors = true;
}

void GR_UnixPangoGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	GR_Painter caretDisablerPainter(this); // not an elegant way to disable all carets, but it works beautifully - MARCM
   	gdk_draw_drawable(_getDrawable(), m_pGC, _getDrawable(), tdu(x_src), tdu(y_src),
   				  tdu(x_dest), tdu(y_dest), tdu(width), tdu(height));
}


UT_uint32 GR_UnixPangoGraphics::getDeviceResolution(void) const
{
	// TODO -- we should get this somewhere from the xft lib
	return m_iDeviceResolution;
}


UT_sint32 GR_UnixPangoGraphics::measureUnRemappedChar(const UT_UCSChar c, UT_uint32 * height)
{
        if (height) { 
		*height = 0;
	}
	UT_sint32 w = measureString(&c, 0, 1, NULL, height);
	return w;
}

bool GR_UnixPangoGraphics::itemize(UT_TextIterator & text, GR_Itemization & I)
{
	// Performance is not of the highest priorty, as this function gets only
	// called once on each text fragment on load or keyboard entry
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::itemize\n"));
	UT_return_val_if_fail( m_pContext, false );
 
	// we need to convert our ucs4 data to utf8 for pango
	UT_UTF8String utf8;
	
	UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
	UT_uint32 iPosStart = text.getPosition();
	UT_uint32 iPosEnd   = text.getUpperLimit();
	UT_return_val_if_fail(iPosEnd < 0xffffffff && iPosEnd >= iPosStart, false);
	
	UT_uint32 iLen = iPosEnd - iPosStart + 1; // including iPosEnd

	UT_uint32 i;
	for(i = 0; i < iLen; ++i, ++text)
	{
		UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
		utf8 += text.getChar();
	}

	UT_uint32 iByteLength = utf8.byteLength();
	
	PangoAttrList *pAttrList = pango_attr_list_new();
	PangoAttrIterator *pIter = pango_attr_list_get_iterator (pAttrList);
	const GR_UnixPangoFont * pFont = (const GR_UnixPangoFont *) I.getFont();

	if (pFont)
	{
		const PangoFontDescription * pfd = pFont->getPangoDescription();
		PangoAttribute * pAttr = pango_attr_font_desc_new (pfd);
		pAttr->start_index = 0;
		pAttr->end_index = iByteLength;
		pango_attr_list_insert(pAttrList, pAttr);		
	}

	const char * pLang = I.getLang();

	if (pLang)
	{
		PangoLanguage  * pl = pango_language_from_string(pLang);
		PangoAttribute * pAttr = pango_attr_language_new (pl);		
		pAttr->start_index = 0;
		pAttr->end_index = iByteLength;
		pango_attr_list_insert(pAttrList, pAttr);		
	}
	
	UT_uint32 iItemCount;

	// this will result in itemization assuming base direction of 0
	// we set the appropriate embedding level later in shape()
	GList *gItems = pango_itemize(m_pContext,
								  utf8.utf8_str(),
								  0, iByteLength,
								  pAttrList, pIter);

	pango_attr_iterator_destroy (pIter);
	pango_attr_list_unref (pAttrList);
	
	iItemCount = g_list_length(gItems);

	// now we process the ouptut
	UT_uint32 iOffset = 0;
	xxx_UT_DEBUGMSG(("itemize: number of items %d\n", iItemCount));
	for(i = 0; i < iItemCount; ++i)
	{
		xxx_UT_DEBUGMSG(("itemize: creating item %d\n", i));
		PangoItem *pItem = (PangoItem *)g_list_nth(gItems, i)->data;
		GR_UnixPangoItem * pI = new GR_UnixPangoItem(pItem);

#if 0 //def DEBUG
		PangoFont * pf = pI->m_pi->analysis.font;
		PangoFontDescription * pfd = pango_font_describe (pf);
		char * pfds = pango_font_description_to_string (pfd);
		
		PangoLanguage * lang = pI->m_pi->analysis.language;

		UT_DEBUGMSG(("@@@@ ===== Item [%s] [%s] =====\n",
					 pfds, pango_language_to_string(lang)));
		
		pango_font_description_free (pfd);
		g_free (pfds);
#endif
		
		I.addItem(iOffset, pI);
		iOffset += pItem->num_chars;
	}

	I.addItem(iPosEnd - iPosStart + 1, new GR_UnixPangoItem());

	g_list_free(gItems);
	
	xxx_UT_DEBUGMSG(("itemize succeeded\n"));
	return true;
}

int *
GR_UnixPangoGraphics::_calculateLogicalOffsets (PangoGlyphString * pGlyphs,
												UT_BidiCharType iVisDir,
												const char * pUtf8)
{
	UT_return_val_if_fail( pGlyphs && pUtf8, NULL );
	
	// pGlyphs contains logical cluster info, which is
	// unfortunately indexed to bytes in the utf-8 string, not characters --
	// this is real pain and we have to convert it.
	
	int * pLogOffsets = new int [pGlyphs->num_glyphs];
 
    // See http://www.abisource.com/mailinglists/abiword-dev/2006/Feb/0081.html
    // for insight how this is supposeed to work and possible optimizations.

	// In LTR text, the values in log_clusters are guaranteed to be increasing,
	// in RTL text, the values in log_clusters are decreasing
	
	if (iVisDir == UT_BIDI_LTR ||
		(pGlyphs->num_glyphs > 1 &&
		 pGlyphs->log_clusters[0] < pGlyphs->log_clusters[1]))
	{
		for(int i = 0; i < pGlyphs->num_glyphs; ++i)
		{
			int iOff = pGlyphs->log_clusters[i];
			pLogOffsets[i] =  g_utf8_pointer_to_offset (pUtf8, pUtf8 + iOff);
		}
	}
	else // GR_ShapingInfo.m_iVisDir == UT_BIDI_RTL)
	{
		for(int i = pGlyphs->num_glyphs - 1; i >= 0; --i)
		{
			int iOff = pGlyphs->log_clusters[i];
			pLogOffsets[i] =  g_utf8_pointer_to_offset (pUtf8, pUtf8 + iOff);
		}
	}

	return pLogOffsets;
}

bool GR_UnixPangoGraphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& ri)
{
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::shape, len %d\n", si.m_iLength));
	UT_return_val_if_fail(si.m_pItem &&
						  si.m_pItem->getClassId() == GRRI_UNIX_PANGO &&
						  si.m_pFont, false);
	
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)si.m_pItem;

	PangoFontset * pfs = NULL;
	PangoFont    * pFontSubst = NULL;
	
	if(!ri)
	{
		// this simply allocates new instance of the RI which this function
		// will fill with meaningful data
		ri = new GR_UnixPangoRenderInfo(pItem->getType());
		UT_return_val_if_fail(ri, false);
	}
	else
	{
		UT_return_val_if_fail(ri->getType() == GRRI_UNIX_PANGO, false);
	}

	GR_UnixPangoRenderInfo * RI = (GR_UnixPangoRenderInfo *)ri;

	// need this so that isSymbol() and isDingbat() are correct
	setFont(si.m_pFont);

	/*
	 * Pango does a royally bad job of the font substitution in
	 * pango_itemize(): it will happily return 'Times New Roman' as
	 * font when we have requested 'Arial', even though the latter is
	 * present and has the necessary coverage. Consequently we have to
	 * do the font substitution manually even on the first shapping.
	 *
	 * If the font has changed from the one for which we previously shapped
	 * (or have not shaped, in which case alloc no is 0), we load a fontset
	 * for the requested font description. Later on, we pick the best font
	 * for each character in this run.
	 */
	if(RI->m_iShapingAllocNo != si.m_pFont->getAllocNumber())
	{
		//UT_DEBUGMSG(("@@@@ ===== Font change %d -> %d\n",
		//			 RI->m_iShapingAllocNo,
		//			 si.m_pFont->getAllocNumber()));
			
		GR_UnixPangoFont * pFont = (GR_UnixPangoFont*)si.m_pFont;

		pfs = pango_font_map_load_fontset (getFontMap(),
										   getContext(),
										   pFont->getPangoDescription(),
										   pItem->m_pi->analysis.language);
	}
	
	UT_UTF8String utf8;
	
	UT_sint32 i;
	for(i = 0; i < si.m_iLength; ++i, ++si.m_Text)
	{
		UT_return_val_if_fail(si.m_Text.getStatus() == UTIter_OK, false);
		UT_UCS4Char c = si.m_Text.getChar();
		if(isSymbol())
			utf8 += adobeToUnicode(c);
		else if(isDingbat())
			utf8 += adobeDingbatsToUnicode(c);
		else
			utf8 += c;

		if (pfs)
		{
			/*
			 * A font change; get the best font for this character
			 */
			PangoFont * font = pango_fontset_get_font (pfs, c);
			
			if (!font)
			{
				/*
				 * We did not find a suitable font -- nothing we can do.
				 */
				UT_DEBUGMSG(("@@@@ ===== Failed to find font for u%04x\n", c));
			}
			else if (pFontSubst && (pFontSubst != font))
			{
				/*
			     * Ok, the font we got for this character does not match
			     * the one we got the for the preceding characters.
			     *
			     * What we could do is to split the run before this character
			     * so we might use two different fonts, but we currently
			     * do not have the infrastructure to do this. Also, doing this
			     * breaks when the missing glyph is a combining character.
			     *
			     * Alternatively, we would need to maintain an internal list
			     * of fonts for each section, but that would mean also to
			     * maintain separate glyph strings, which would be a nightmare.
			     *
			     * We can limit this from happening by preventing items that
			     * will use different font from merging, which I have now done,
			     * but again, this does not work when combining characters are
			     * involved, because we cannot draw the combining character on
			     * it's own.
			     *
			     * TODO -- devise a sensible way of handling this.
			     */
				UT_DEBUGMSG(("@@@@ ===== Font for u%04x does not match "
							 "earlier font\n", c));
				pFontSubst = font;
			}
			else if (pFontSubst == font)
			{
				/* We now have two references to this font, rectify */
				g_object_unref (G_OBJECT (pFontSubst));
			}
			else
			{
				pFontSubst = font;
			}

#if 0 //def DEBUG
			PangoFontDescription * pfd =
				pango_font_describe (font);
			char * s = pango_font_description_to_string (pfd);

			UT_DEBUGMSG(("@@@@ ===== Font for u%04x: %s\n", c, s));
			g_free (s);
			pango_font_description_free (pfd);
#endif
			
		}
	}

	if(pfs) 
	{
		g_object_unref((GObject*)pfs);
		pfs = NULL;
	}
	if (pFontSubst)
	{
		/*
		 * We are doing font substitution -- release the font previously
		 * stored in the PangoAnalysis and replace it with this one.
		 */
		if (pItem->m_pi->analysis.font)
			g_object_unref (G_OBJECT (pItem->m_pi->analysis.font));
		
		pItem->m_pi->analysis.font = (PangoFont*)g_object_ref((GObject*)pFontSubst);
	}
	
	RI->m_iCharCount = si.m_iLength;
	
	if(RI->m_pGlyphs)
	{
		pango_glyph_string_free(RI->m_pGlyphs);
		RI->m_pGlyphs = NULL;
	}
	
	if(RI->m_pScaledGlyphs)
	{
		pango_glyph_string_free(RI->m_pScaledGlyphs);
		RI->m_pScaledGlyphs = NULL;
	}
	
	RI->m_pGlyphs = pango_glyph_string_new();
	RI->m_pScaledGlyphs = pango_glyph_string_new();

	/*
	 * We want to do the shaping on a font at it's actual point size, so we
	 * cannot use the font in our PangoAnalysis structure, which we will
	 * later use for drawing, and which will be adjusted for the current
	 * zoom.
	 */
	UT_LocaleTransactor t(LC_NUMERIC, "C");
	UT_String              s;
	PangoFont            * pPangoFontOrig = pItem->m_pi->analysis.font;
	GR_UnixPangoFont     * pFont = (GR_UnixPangoFont *) si.m_pFont;;
	PangoFontDescription * pfd;
	
	if (pPangoFontOrig)
	{
		pfd = pango_font_describe (pPangoFontOrig);
		double dSize = (double)PANGO_SCALE * pFont->getPointSize();
		pango_font_description_set_size (pfd, (gint)dSize);

#if 0 //def DEBUG
		char * s = pango_font_description_to_string (pfd);
		UT_DEBUGMSG(("@@@@ ===== Shaping with font [%s]\n", s));
		g_free (s);
#endif
	}
	else
	{
		UT_ASSERT_HARMLESS( !pFont->isGuiFont() );
		UT_String_sprintf(s, "%s %f",
						  pFont->getDescription().c_str(),
						  pFont->getPointSize());
		
		pfd = pango_font_description_from_string(s.c_str());
	}

	UT_return_val_if_fail(pfd, false);
	PangoFont * pf = pango_context_load_font(getLayoutContext(), pfd);
	pango_font_description_free(pfd);
	
	// no need to ref pf because it will replaced right after
	pItem->m_pi->analysis.font = pf;

	// need to set the embedding level here based on the level of our run
	pItem->m_pi->analysis.level = si.m_iVisDir == UT_BIDI_RTL ? 1 : 0;
	
	pango_shape(utf8.utf8_str(), utf8.byteLength(),
				&(pItem->m_pi->analysis), RI->m_pGlyphs);
	pango_shape(utf8.utf8_str(), utf8.byteLength(),
				&(pItem->m_pi->analysis), RI->m_pScaledGlyphs);

	pItem->m_pi->analysis.font = pPangoFontOrig;
		
	if(RI->m_pLogOffsets)
	{
		delete [] RI->m_pLogOffsets;
	}

	RI->m_pLogOffsets = _calculateLogicalOffsets(RI->m_pGlyphs,
												 si.m_iVisDir,
												 utf8.utf8_str());
	
	// need to transfer data that we will need later from si to RI
	RI->m_iLength = si.m_iLength;
	RI->m_pItem   = si.m_pItem;
	RI->m_pFont   = si.m_pFont;
	RI->m_iShapingAllocNo = si.m_pFont->getAllocNumber();
	
	RI->m_eShapingResult = GRSR_ContextSensitiveAndLigatures;

	// remove any justification information -- it will have to be recalculated
	delete[] RI->m_pJustify; RI->m_pJustify = NULL;
	
	// we did our calculations at notional 100%
	RI->m_iZoom = 100;

	return true;
}


UT_sint32 GR_UnixPangoGraphics::getTextWidth(GR_RenderInfo & ri)
{
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::getTextWidth\n"));
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
	
	UT_return_val_if_fail( RI.m_pGlyphs && RI.m_pLogOffsets && pItem, 0 );
	
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *) RI.m_pFont;
	UT_return_val_if_fail( pFont, 0 );
	
	//
	// Actually want the layout font here
	//
	PangoFont * pf = _adjustedLayoutPangoFont(pFont, pItem->m_pi->analysis.font);
	PangoFont * pfa = _adjustedPangoFont(pFont, pItem->m_pi->analysis.font);
	
	xxx_UT_DEBUGMSG(("Adjusted Layout font %x Adjusted font %x \n",pf,pfa));
	UT_return_val_if_fail( pf, 0 );

	UT_sint32 iStart = RI.m_iOffset;
	UT_sint32 iEnd   = RI.m_iOffset + RI.m_iLength;
	
	UT_sint32 iWidth =  _measureExtent (RI.m_pGlyphs, pf, RI.m_iVisDir, NULL,
						   RI.m_pLogOffsets, iStart, iEnd);
	xxx_UT_DEBUGMSG(("TextWidths Pango Font %x height %d text width %d \n",
					 pFont, pFont->getAscent(), iWidth));
	return iWidth;
}

/*!
 * Calculates the extents of string corresponding to glyphstring from
 * *character* offset iStart to iEnd (excluding iEnd);
 *
 * iDir is the visual direction of the text
 * 
 * pUtf8 pointer to the corresponding utf8 string; can be NULL if pLogOffsets
 *    is provided
 *    
 * pLogOffsets is array of logical offsets (see
 *    gr_UnixPangoRenderInfo::m_pLogOffsets); if NULL, it will be calculated
 *    using the corresponding utf8 string and pointer returned back; the
 *    caller needs to delete[] it when no longer needed.
 *
 * On return iStart and iEnd contain the offset values that correspond to the
 * returned extent (e.g., if the original iStart and/or iEnd are not legal
 * character postions due to clustering rules, these can be different from
 * the requested values).
 */
UT_uint32 GR_UnixPangoGraphics::_measureExtent (PangoGlyphString * pg,
												PangoFont * pf,
												UT_BidiCharType iDir,
												const char * pUtf8,
												int * & pLogOffsets,
												UT_sint32 & iStart,
												UT_sint32 & iEnd)
{
	UT_return_val_if_fail( pg && pf, 0 );
	PangoRectangle LR;

	// need to convert the char offset and length to glyph offsets
	UT_uint32 iGlyphCount = pg->num_glyphs;
	UT_sint32 iOffsetStart = -1, iOffsetEnd = -1;

	if (!pLogOffsets)
	{
		UT_return_val_if_fail( pUtf8, 0 );
		pLogOffsets = _calculateLogicalOffsets (pg, iDir, pUtf8);
	}

	UT_return_val_if_fail( pLogOffsets, 0 );
	
	// loop running in visual plane
	for(UT_uint32 i = 0; i < iGlyphCount; ++i)
	{
		// have to index glyphs in logical plane to hit our logical start
		// offset before the end offset
		UT_uint32 k = (iDir == UT_BIDI_RTL) ? iGlyphCount - i - 1 : i;

		// test for >= -- in case of combining characters, the requested offset
		// might inside the cluster, which is not legal, we take the first
		// offset given to us
		if(iOffsetStart < 0 && pLogOffsets[k] >= iStart)
		{
			iOffsetStart = k;
			iStart = pLogOffsets[k];
			xxx_UT_DEBUGMSG(("::getTextWidth: iOffsetStart == %d\n",
						 iOffsetStart));
			continue;
		}
		

		if(pLogOffsets[k] >= iEnd)
		{
			iOffsetEnd = k;
			iEnd = pLogOffsets[k];
			xxx_UT_DEBUGMSG(("::getTextWidth: iOffsetEnd == %d\n",
							 iOffsetEnd));
			break;
		}
	}

	UT_ASSERT_HARMLESS( iOffsetStart >= 0 );

	PangoFontDescription * pfd = pango_font_describe (pf);
	int isize = pango_font_description_get_size(pfd);
	xxx_UT_DEBUGMSG(("Font size in _measureExtents %d \n",isize));

	if(iOffsetEnd < 0 && iDir == UT_BIDI_LTR)
	{
		// to the end
		iOffsetEnd = iGlyphCount;
	}

	if (iDir == UT_BIDI_RTL)
	{
		// in RTL text, the start offset will be higher than the end offset
		// and we will want to measure (iOffsetEnd, iOffsetStart>
		UT_sint32 t  = iOffsetStart;
		iOffsetStart = iOffsetEnd + 1; // + 1 excludes iOffsetEnd
		iOffsetEnd   = t + 1;          // + 1 includes iOffsetStart
	}

	UT_return_val_if_fail( iOffsetStart >= 0, 0 );
	
	pango_glyph_string_extents_range(pg,
									 iOffsetStart,
									 iOffsetEnd, pf, NULL, &LR);

	xxx_UT_DEBUGMSG(("::getTextWidth start %d, end %d, w %d, x %d\n",
				 iOffsetStart, iOffsetEnd, LR.width, LR.x));

	return ptlunz(LR.width + LR.x);
}


/*!
    Do any pre-processing that might be needed for rendering our text (This
    function is guaranteed to be called just before renderChars(), but where
    drawing of a single RI item is done inside of a loop, e.g., drawing the
    different segments of partially selected run, this function can be take out
    of the loop.)
*/
void GR_UnixPangoGraphics::prepareToRenderChars(GR_RenderInfo & ri)
{
	// the only thing we need to do here is to make sure that the glyph metrics
	// are calculated to a correct zoom level.
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	if(RI.m_iZoom != getZoomPercentage())
	{
		_scaleCharacterMetrics(RI);
	}
}

/*
 * This is used to get device zoomed PangoFont that is correct for present zoom level.
 * pFont is the font that we are supposed to be using (the user-selected font)
 * pf is the PangoFont that we are actually using (possibly a different,
 * substituted font).
 */
PangoFont *  GR_UnixPangoGraphics::_adjustedPangoFont (GR_UnixPangoFont * pFont, PangoFont * pf)
{
	UT_return_val_if_fail(pFont, NULL);
	
	if (!pf)
		return pFont->getPangoFont();

	/* See if this is not the font we have currently cached */
	if (pFont == m_pAdjustedPangoFontSource &&
		m_iAdjustedPangoFontZoom == getZoomPercentage())
	{
		return m_pAdjustedPangoFont;
	}

	/*
	 * When Pango is doing font substitution for us, the substitute font
	 * we are getting always has size 12pt, so we have to use the size of
	 * our own font to fix this.
	 */
	PangoFontDescription * pfd = pango_font_describe (pf);

	double dSize = pFont->getPointSize ();

	/* We cache this font to avoid all this huha if we can */
	if (m_pAdjustedLayoutPangoFont) 
	{
		g_object_unref(m_pAdjustedLayoutPangoFont);
	}
	if (m_pAdjustedPangoFont) 
	{
		g_object_unref(m_pAdjustedPangoFont);
	}
	pango_font_description_set_size (pfd, (gint)dSize * PANGO_SCALE);
	m_pAdjustedLayoutPangoFont = pango_context_load_font(getLayoutContext(), pfd);
	m_pAdjustedPangoFontSource = pFont;

	dSize =
		(gint)(dSize*(double)PANGO_SCALE *(double)getZoomPercentage() / 100.0);
	pango_font_description_set_size (pfd, (gint)dSize);
	m_pAdjustedPangoFont = pango_context_load_font(getContext(), pfd);
	m_iAdjustedPangoFontZoom = getZoomPercentage();
	
	pango_font_description_free(pfd);
	
	return m_pAdjustedPangoFont;
}


/*
 * This is used to get Layout PangoFont that is correct for present zoom level.
 * pFont is the font that we are supposed to be using (the user-selected font)
 * pf is the PangoFont that we are actually using (possibly a different,
 * substituted font).
 */
PangoFont *  GR_UnixPangoGraphics::_adjustedLayoutPangoFont (GR_UnixPangoFont * pFont, PangoFont * pf)
{
	UT_return_val_if_fail(pFont, NULL);
	
	if (!pf)
		{
			xxx_UT_DEBUGMSG(("Getting Layout font \n"));
			return pFont->getPangoLayoutFont();
		}
	/* See if this is not the font we have currently cached */
	if (pFont == m_pAdjustedPangoFontSource &&
		m_iAdjustedPangoFontZoom == getZoomPercentage())
	{
		return m_pAdjustedLayoutPangoFont;
	}

	/*
	 * When Pango is doing font substitution for us, the substitute font
	 * we are getting always has size 12pt, so we have to use the size of
	 * our own font to fix this.
	 */
	PangoFontDescription * pfd = pango_font_describe (pf);

	double dSize = pFont->getPointSize()*(double)PANGO_SCALE;
   
	xxx_UT_DEBUGMSG(("Setting adjustedLayout point size %f \n",dSize));
	pango_font_description_set_size (pfd, (gint)dSize);

	/* We cache this font to avoid all this huha if we can */
	if (m_pAdjustedLayoutPangoFont) 
	{
		g_object_unref(m_pAdjustedLayoutPangoFont);
	}
	if (m_pAdjustedPangoFont )
	{
		g_object_unref(m_pAdjustedPangoFont);
	}
	m_pAdjustedLayoutPangoFont = pango_context_load_font(getLayoutContext(), pfd);
	m_pAdjustedPangoFontSource = pFont;

	dSize =
		(gint)(dSize* (double)getZoomPercentage() / 100.0);
	pango_font_description_set_size (pfd, (gint)dSize);
	m_pAdjustedPangoFont = pango_context_load_font(getContext(), pfd);
	m_iAdjustedPangoFontZoom = getZoomPercentage();
	
	pango_font_description_free(pfd);
	
	return m_pAdjustedLayoutPangoFont;
}


/*!
    The offset passed to us as part of ri is a visual offset
*/
void GR_UnixPangoGraphics::renderChars(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *)RI.m_pFont;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
	UT_return_if_fail(pItem && pFont && pFont->getPangoFont());
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::renderChars length %d \n",
					 RI.m_iLength));

	if(RI.m_iLength == 0)
		return;

	//
	// Actually want the zoomed device font here
	//
	PangoFont * pf = _adjustedPangoFont(pFont, pItem->m_pi->analysis.font);

	xxx_UT_DEBUGMSG(("Pango renderChars: xoff %d yoff %d\n",
					 RI.m_xoff, RI.m_yoff));
	
	UT_sint32 xoff = _tduX(RI.m_xoff);
	UT_sint32 yoff = _tduY(RI.m_yoff + getFontAscent(pFont));

	UT_return_if_fail(m_pXftDraw && RI.m_pScaledGlyphs);

	// TODO -- test here for the endpoint as well
	if(RI.m_iOffset == 0 &&
	   (RI.m_iLength == (UT_sint32)RI.m_iCharCount || !RI.m_iCharCount))
	{
		xxx_UT_DEBUGMSG(("Doing XFT Render now.\n")); 
		pango_xft_render(m_pXftDraw, &m_XftColor, pf,
						 RI.m_pScaledGlyphs, xoff, yoff);
	}
	else
	{
		// This is really stupid -- Pango provides no way of drawing substrings,
		// so we need to create a new glyph string, that only contains the
		// subset This is complicated by the fact that all offsets in the Pango
		// api are stupid byte offsets in to utf8 strings, not character offsets
		UT_return_if_fail( RI.m_pText );
		UT_TextIterator & text = *RI.m_pText;
		PangoGlyphString gs;

		UT_UTF8String utf8;
		UT_uint32 i;
		
		for(i = 0; i < RI.m_iCharCount && text.getStatus() == UTIter_OK;
			++i, ++text)
		{
			utf8 += text.getChar();
		}

		if(RI.m_iCharCount > i)
		{
			// it seems the iterator run out on us
			// this should probably not happen
			xxx_UT_DEBUGMSG(("gr_UnixPangoGraphics::renderChars: iterator too short\n"));
			return;
		}
		
		UT_sint32 iOffsetStart
			= RI.m_iVisDir == UT_BIDI_RTL ?
			                 RI.m_iCharCount - RI.m_iOffset - 1 : RI.m_iOffset;
		
		const char * pUtf8   = utf8.utf8_str();
		const char * pOffset = g_utf8_offset_to_pointer (pUtf8, iOffsetStart);
		
		if (pOffset)
			iOffsetStart = pOffset - pUtf8;
		
		UT_sint32 iOffsetEnd
			= RI.m_iVisDir == UT_BIDI_RTL ?
			                 RI.m_iCharCount - RI.m_iOffset - RI.m_iLength:
			                 RI.m_iOffset + RI.m_iLength - 1;
		
		pOffset = g_utf8_offset_to_pointer (pUtf8, iOffsetEnd);
		
		if (pOffset)
			iOffsetEnd = pOffset - pUtf8;
		
		// now we need to work out the glyph offsets
		UT_sint32 iGlyphsStart = -1;
		UT_sint32 iGlyphsEnd = -1;
		
		i = 0;
		while(i < (UT_uint32)RI.m_pScaledGlyphs->num_glyphs)
		{
			if(iGlyphsStart < 0 && RI.m_pScaledGlyphs->log_clusters[i] == iOffsetStart)
				iGlyphsStart = i;

			if(RI.m_pScaledGlyphs->log_clusters[i] == iOffsetEnd)
			{
				iGlyphsEnd = i;
				break;
			}
			
			++i;
		}

		// both of these can be 0 (iGlyphsEnd == 0 => only 1 glyph)
		//	UT_return_if_fail( iGlyphsStart >= 0 && iGlyphsEnd >= 0 );
		UT_DEBUGMSG(("Drawing glyph subset from %d to %d (offsets %d, %d)\n",
					 iGlyphsStart, iGlyphsEnd,
					 iOffsetStart, iOffsetEnd));
		
		gs.num_glyphs = iGlyphsEnd - iGlyphsStart + 1; // including the last glyph
		gs.glyphs = RI.m_pScaledGlyphs->glyphs + iGlyphsStart;
		gs.log_clusters = RI.m_pGlyphs->log_clusters + iGlyphsStart;

		pango_xft_render(m_pXftDraw, &m_XftColor, pf,
						 &gs, xoff, yoff);

	}
}

void GR_UnixPangoGraphics::_scaleCharacterMetrics(GR_UnixPangoRenderInfo & RI)
{
	UT_uint32 iZoom = getZoomPercentage();

	xxx_UT_DEBUGMSG(("_scaleCharacterMetrics... \n"));
	for(int i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		RI.m_pScaledGlyphs->glyphs[i].geometry.x_offset =
			_tduX(RI.m_pGlyphs->glyphs[i].geometry.x_offset);


		RI.m_pScaledGlyphs->glyphs[i].geometry.y_offset = _tduY(RI.m_pGlyphs->glyphs[i].geometry.y_offset);

		RI.m_pScaledGlyphs->glyphs[i].geometry.width =_tduX(RI.m_pGlyphs->glyphs[i].geometry.width );
	}
	RI.m_iZoom = iZoom;
}


void GR_UnixPangoGraphics::_scaleJustification(GR_UnixPangoRenderInfo & RI)
{
	RI.m_iZoom = getZoomPercentage();
	return;
}


/*!
   This function is called after shaping and before any operations are done on
   the glyphs. Although Pango does not have a separate character placement
   stage, we need to scale the glyph metrics to appropriate zoom level (since
   we shape @100% zoom).

   NB: this is probably not ideal with int arithmetic as moving from one zoom
       to another, and back we are bound to end up with incorrect metrics due
       to rounding errors.
*/
void GR_UnixPangoGraphics::measureRenderedCharWidths(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	_scaleCharacterMetrics(RI);

	if(RI.m_pJustify)
	{
		_scaleJustification(RI);
	}
}

void GR_UnixPangoGraphics::appendRenderedCharsToBuff(GR_RenderInfo & ri,
													 UT_GrowBuf & buf) const
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
}

/*!
    returns true on success
 */
bool GR_UnixPangoGraphics::_scriptBreak(GR_UnixPangoRenderInfo &ri)
{
	UT_return_val_if_fail(ri.m_pText && ri.m_pGlyphs && ri.m_pItem, false);

	GR_UnixPangoItem * pItem = (GR_UnixPangoItem*)ri.m_pItem;

	// fill the static buffer with UTF8 text
	UT_return_val_if_fail(ri.getUTF8Text(), false);

	// the buffer has to have at least one more slot than the number of glyphs
	if(!ri.s_pLogAttrs || ri.s_iStaticSize < ri.sUTF8->length() + 1)
	{
		UT_return_val_if_fail(ri.allocStaticBuffers(ri.sUTF8->length()+1),
							  false);
	}
	
	pango_break(ri.sUTF8->utf8_str(),
				ri.sUTF8->byteLength(),
				&(pItem->m_pi->analysis),
				ri.s_pLogAttrs, ri.s_iStaticSize);

	ri.s_pOwnerLogAttrs = &ri;
	return true;
}

bool GR_UnixPangoGraphics::canBreak(GR_RenderInfo & ri, UT_sint32 &iNext,
									bool bAfter)
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO &&
						  ri.m_iOffset < ri.m_iLength, false);
	
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	iNext = -1;

	if(!RI.s_pLogAttrs || RI.s_pOwnerLogAttrs != &ri)
	{
		if(!_scriptBreak(RI))
			return false;
	}
	
	UT_uint32 iDelta  = 0;
	if(bAfter)
	{
		// the caller wants to know if break can occur on the (logically) right
		// edge of the given character
		
		if(ri.m_iOffset + 1 >= (UT_sint32)RI.s_iStaticSize)
		{
			// we are quering past what have data for
			return false;
		}

		// we will examine the next character, since Pango tells us about
		// breaking on the left edge
		iDelta = 1;
	}

	if(RI.s_pLogAttrs[ri.m_iOffset + iDelta].is_line_break)
		return true;

	// find the next break
	for(UT_sint32 i = ri.m_iOffset + iDelta + 1; i < RI.m_iLength; ++i)
	{
		if(RI.s_pLogAttrs[i].is_line_break)
		{
			iNext = i - iDelta;
			break;
		}
	}
		
	if(iNext == -1)
	{
		// we have not found any breaks in this run -- signal this to the
		// caller
		iNext = -2;
	}
	
	return false;
}


bool GR_UnixPangoGraphics::needsSpecialCaretPositioning(GR_RenderInfo & ri)
{
	// something smarter is needed here, so we do not go through this for
	// langugages that do not need it.
	return true;
}

UT_uint32 GR_UnixPangoGraphics::adjustCaretPosition(GR_RenderInfo & ri,
													bool bForward)
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	
	if(!RI.s_pLogAttrs || RI.s_pOwnerLogAttrs != &ri)
		_scriptBreak(RI);

	UT_return_val_if_fail( RI.s_pLogAttrs, RI.m_iOffset );
	
	UT_sint32 iOffset = ri.m_iOffset;

	if(bForward)
		while(!RI.s_pLogAttrs[iOffset].is_cursor_position &&
			  iOffset < RI.m_iLength)
			iOffset++;
	else
		while(!RI.s_pLogAttrs[iOffset].is_cursor_position && iOffset > 0)
			iOffset--;
	
	return iOffset;
}

void GR_UnixPangoGraphics::adjustDeletePosition(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	if(ri.m_iOffset + ri.m_iLength >= (UT_sint32)RI.m_iCharCount)
		return;
	
	if(!RI.s_pLogAttrs || RI.s_pOwnerLogAttrs != &ri)
		_scriptBreak(RI);

	UT_return_if_fail( RI.s_pLogAttrs);
	
	// deletion can start anywhere, but can only end on cluster boundary if the
	// base character is included in the deletion
	
	// get the offset of the character that follows the deleted segment
	UT_sint32 iNextOffset = (UT_sint32)ri.m_iOffset + ri.m_iLength;

	if(RI.s_pLogAttrs[iNextOffset].is_cursor_position)
	{
		// the next char is a valid caret position, so we are OK
		return;
	}

	// If we got this far, we were asked to end the deletion before a character
	// that is not a valid caret position. We need to determine if the segment
	// we are asked to delete contains the character's base character; if it
	// does, we have to expand the seletion to delete the entire cluster.

	UT_sint32 iOffset = iNextOffset - 1;
	while(iOffset > 0 && iOffset > ri.m_iOffset &&
		  !RI.s_pLogAttrs[iOffset].is_cursor_position)
		iOffset--;

	if(RI.s_pLogAttrs[iOffset].is_cursor_position)
	{
		// our delete segment includes the base character, so we have to delete
		// the entire cluster
		iNextOffset = iOffset + 1;
		
		while(iNextOffset < (UT_sint32)RI.s_iStaticSize - 1 // -1 because iLogBuffSize is char count +1
			  && !RI.s_pLogAttrs[iNextOffset].is_cursor_position)
			iNextOffset++;

		
		ri.m_iLength = iNextOffset - ri.m_iOffset;
		return;
	}
	
	// two posibilities: we are deleting only a cluster appendage or the run
	// does not contain base character. The latter should probably not happen,
	// but in both cases we will let the delete proceed as is
}

/*!
 * I believe this code clears all the justification points. MES June 2008
 * It returns the total space assigned to justify the text in layout units.
 */
UT_sint32 GR_UnixPangoGraphics::resetJustification(GR_RenderInfo & ri,
												   bool bPermanent)
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	if(!RI.m_pJustify)
		return 0;

	
	UT_sint32 iWidth2 = 0;
	for(UT_sint32 i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		iWidth2 += RI.m_pJustify[i];

		// TODO here we need to substract the amount from pango metrics
		RI.m_pGlyphs->glyphs[i].geometry.width -= RI.m_pJustify[i];
	}
	//
	// This sets the glyphs that will be displayed on screen.
	//
	_scaleCharacterMetrics(RI);

	if(bPermanent)
	{
		delete [] RI.m_pJustify;
		RI.m_pJustify = NULL;
	}
	else
	{
		memset(RI.m_pJustify, 0, RI.m_pGlyphs->num_glyphs * sizeof(int));
	}
	
	// Justification in pango units. Convert to layout units.

	return -ptlunz(iWidth2);
}


UT_sint32 GR_UnixPangoGraphics::countJustificationPoints(const GR_RenderInfo & ri) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;

	UT_return_val_if_fail(RI.m_pText, 0);
	UT_TextIterator & text = *RI.m_pText;
	UT_uint32 iPosEnd   = text.getUpperLimit();

	text.setPosition(iPosEnd);
	UT_return_val_if_fail( text.getStatus()== UTIter_OK, 0 );
	

	UT_sint32 iCount = 0;
	bool bNonBlank = false;
	UT_sint32 iLen = RI.m_iLength;
	
	for(; iLen > 0 && text.getStatus() == UTIter_OK; --text, --iLen)
	{
		UT_UCS4Char c = text.getChar();
		
		if(c != UCS_SPACE)
		{
			bNonBlank = true;
			continue;
		}
		
		// only count this space if this is not last run, or if we
		// have found something other than spaces
		if(!ri.m_bLastOnLine || bNonBlank)
			iCount++;
	}

	if(!bNonBlank)
	{
		return -iCount;
	}
	else
	{
		return iCount;
	}
}

/*!
    We take the same approach as with Uniscribe; we store the justification
    amount in a separate array of the ri and add it to the offsets before we
    draw. We will probably need some static buffers to speed things up

It requires as input RI.m_iJustificationAmount and RI.m_iJustificationPoints.
These are determined in fp_TextRun using calculations in layout units

 */
void GR_UnixPangoGraphics::justify(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &) ri;
	if(!RI.m_iJustificationPoints || !RI.m_iJustificationAmount ||
	   !RI.m_pGlyphs)
		return;

	// make sure that we are not adding apples to oranges
	// We don't need this now.
	//	if(RI.m_iZoom != getZoomPercentage())
	//	_scaleCharacterMetrics(RI);
	
	if(!RI.m_pJustify)
		RI.m_pJustify = new int[RI.m_pGlyphs->num_glyphs];
	
	UT_return_if_fail(RI.m_pJustify);
	memset(RI.m_pJustify, 0, RI.m_pGlyphs->num_glyphs * sizeof(int));
	
	UT_uint32 iExtraSpace = RI.m_iJustificationAmount;
	UT_uint32 iPoints     = RI.m_iJustificationPoints;

	xxx_UT_DEBUGMSG(("::Justify Extra justification space %d \n",iExtraSpace));
	xxx_UT_DEBUGMSG(("::Justify Number of justification points %d \n",iPoints));

	UT_return_if_fail(RI.m_pText);
	
	UT_TextIterator & text = *RI.m_pText;
	UT_sint32 iGlyphCount  = RI.m_pGlyphs->num_glyphs;
	UT_BidiCharType iDir   =
		RI.m_iVisDir % 2 ? UT_BIDI_RTL : UT_BIDI_LTR;

	// Split into two big loops to avoid all the LTR/RTL logic inside -- if
	// you make changes to one branch, please make sure to do the same to the
	// other.
	
	UT_sint32 i; // glyph index
	UT_sint32 j; // text index

	UT_uint32 iSpace = iExtraSpace/iPoints;
	
	if (iDir == UT_BIDI_LTR)
	{
		i = 0;
		j = 0;
		
		while (text.getStatus() == UTIter_OK &&
			   i < iGlyphCount &&
			   j < RI.m_iLength)
		{
			UT_UCS4Char c = text.getChar();
		
			if(c == UCS_SPACE)
			{
			

				// iSpace is in layout units. Convert to pango units

				RI.m_pJustify[i] = ltpunz(iSpace);

				iPoints--;

				// add this amount the pango units
				xxx_UT_DEBUGMSG(("Justify-1 Prev geom width %d additional %d \n",RI.m_pGlyphs->glyphs[i].geometry.width,RI.m_pJustify[i]));

				RI.m_pGlyphs->glyphs[i].geometry.width += RI.m_pJustify[i];
			
				if(!iPoints)
					break;
			}


			// skip over any glyphs that belong to the current character
			// LTR run, the glyphs and the text are in the same order,
			// and logical offsets are increasing
			UT_sint32 iOffset = RI.m_pLogOffsets[i++];
		
			while (RI.m_pLogOffsets[i] == iOffset && i < iGlyphCount)
				++i;

			if (i >= iGlyphCount)
				break;
		
			// if the glyph cluster represents more characters than its
			// length, we have to advance the iterator accordingly
			UT_sint32 iDiff = RI.m_pLogOffsets[i] - iOffset;
		
			text += iDiff;
			j += iDiff;
		}
	}
	else
	{
		i = iGlyphCount - 1;
		j = 0;
		
		while (text.getStatus() == UTIter_OK &&
			   i >= 0 &&
			   j < RI.m_iLength)
		{
			UT_UCS4Char c = text.getChar();
		
			if(c == UCS_SPACE)
			{
			
				iPoints--;

				// iSpace is in layout units. Convert to pango units

				RI.m_pJustify[i] = ltpunz(iSpace);

				// add this amount the pango metrics

				xxx_UT_DEBUGMSG(("Justify-2 Prev geom width %d additional %d \n",RI.m_pGlyphs->glyphs[i].geometry.width,RI.m_pJustify[i]));
				RI.m_pGlyphs->glyphs[i].geometry.width += RI.m_pJustify[i];
			
				if(!iPoints)
					break;
			}


			// skip over any glyphs that belong to the current character
			// RTL run, so the glyphs are in reversed order of the text,
			// and logical offsets are decreesing
			UT_sint32 iOffset = RI.m_pLogOffsets[i--];
		
			while (RI.m_pLogOffsets[i] == iOffset && i >= 0)
				--i;

			if (i < 0)
				break;
		
			// if the glyph cluster represents more characters than its
			// length, we have to advance the iterator accordingly
			UT_sint32 iDiff = iOffset - RI.m_pLogOffsets[i];
		
			text += iDiff;
			j += iDiff;
		}
	}
	//
	// Now scale the metrics for the drawing glyphs
	//
	_scaleCharacterMetrics(RI);
}

/*!
 * This function takes (x,y) in layout units and determines the location in the
 * pango string.
 */
UT_uint32 GR_UnixPangoGraphics::XYToPosition(const GR_RenderInfo & ri, UT_sint32 x, UT_sint32 y) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_UNIX_PANGO, 0);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &) ri;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
	UT_return_val_if_fail(pItem, 0);

	// TODO: this is very inefficient: to cache or not to cache ?
	UT_UTF8String utf8;
	
	UT_sint32 i;
	for(i = 0; i < RI.m_iLength; ++i, ++(*(RI.m_pText)))
	{
		UT_return_val_if_fail(RI.m_pText->getStatus() == UTIter_OK, 0);
		utf8 += RI.m_pText->getChar();
	}
	
	// Since the glyphs are measured in pango units
	// we need to convert from layout units

	int x_pos = ltpunz(x);
	int len = utf8.byteLength();
	int iPos = len;
	int iTrailing;
	const char * pUtf8 = utf8.utf8_str();

	/* Another jolly pango function:
	 * if x is greater than the width of the string, it will happily read
	 * pass the end of it.
	 */
	pango_glyph_string_x_to_index(RI.m_pGlyphs,
								  (char*)pUtf8, // do not like this ...
								  len,
								  &(pItem->m_pi->analysis), 
								  x_pos,
								  &iPos,
								  &iTrailing);

	/* if at the end (or pass) the end of the string, just return the length*/
	if (iPos >= len)
	{
		return RI.m_iLength;
	}
	
	i = g_utf8_pointer_to_offset(pUtf8, pUtf8 + iPos);
	
	if(iTrailing)
		i++;

	return i;
}

/*!
 * Return a location in layout units (x,y) of a pango glyph.
 */
void GR_UnixPangoGraphics::positionToXY(const GR_RenderInfo & ri,
										  UT_sint32& x, UT_sint32& y,
										  UT_sint32& x2, UT_sint32& y2,
										  UT_sint32& height, bool& bDirection) const
{
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &) ri;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
  
	if(!pItem)
		return;

	// TODO: this is very inefficient: to cache or not to cache ?
	UT_UTF8String utf8;

	UT_sint32 i;
	for(i = 0; i < RI.m_iLength; ++i, ++(*(RI.m_pText)))
	{
		UT_return_if_fail(RI.m_pText->getStatus() == UTIter_OK);
		utf8 += RI.m_pText->getChar();
	}

	UT_sint32 iByteOffset;
	gboolean  bTrailing = TRUE;
	const char * pUtf8 = utf8.utf8_str();
	const char * pOffset = NULL;
	
	if(RI.m_iOffset < 0)
	{
		// we translate negative offsets into leading edge of the first char
		iByteOffset = 0;
		bTrailing = FALSE;
	}
	else if(RI.m_iOffset == 0)
	{
		// trailing edge of the first char
		iByteOffset = 0;
	}
	else if( i > RI.m_iOffset)
	{
		// withing range of our string
		pOffset = g_utf8_offset_to_pointer (pUtf8, RI.m_iOffset);
	}
	else if(i > 1)
	{
		// this is the case where the requested offset is past the end
		// of our string; we will use the last char; as we have more than one
		// character we can use g_utf8_prev_char (); 
		pOffset = g_utf8_prev_char (pUtf8 + utf8.byteLength());
	}
	else
	{
		// something utterly wrong ...
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		iByteOffset = 0;
	}

	if (pOffset)
		iByteOffset = pOffset - pUtf8;
	
	pango_glyph_string_index_to_x (RI.m_pGlyphs,
								   (char*)pUtf8, // do not like this ...
								   utf8.byteLength(),
								   &(pItem->m_pi->analysis), 
								   iByteOffset,
								   bTrailing,
								   &x);

	//
	// Since the glyphs are measured in pango units we need to convert to layout
	//
	x = ptlunz(x);
	x2 = x;
}

void GR_UnixPangoGraphics::drawChars(const UT_UCSChar* pChars,
									int iCharOffset, int iLength,
									UT_sint32 xoff, UT_sint32 yoff,
									int * pCharWidth)
{
	UT_return_if_fail(m_pXftDraw);

	UT_UTF8String utf8;

	if(isSymbol())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeToUnicode(pChars[i]);
		}
	}
	else if(isDingbat())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeDingbatsToUnicode(pChars[i]);
		}
	}
	else
	{
		utf8.appendUCS4(pChars + iCharOffset, iLength);
	}

	// this function expect indexes in bytes !!! (stupid)
	GList * pItems = pango_itemize(getContext(),
								   utf8.utf8_str(),
								   0, utf8.byteLength(),
								   NULL, NULL);
	
	int iItemCount = g_list_length(pItems);
	PangoGlyphString * pGstring = pango_glyph_string_new();

	UT_sint32 xoffD = _tduX(xoff);
	UT_sint32 yoffD = _tduY(yoff+getFontAscent());

	PangoFont * pf = m_pPFont->getPangoFont();
	PangoRectangle LR;
	
	for(int i = 0; i < iItemCount; ++i)
	{
		PangoItem *pItem = (PangoItem *)g_list_nth(pItems, i)->data;

		if(!pItem)
		{
			UT_ASSERT(pItem);
			if(pGstring)
				pango_glyph_string_free(pGstring);
			_pango_item_list_free(pItems);
			return;
		}

		g_object_unref(pItem->analysis.font);
		pItem->analysis.font = (PangoFont*)g_object_ref((GObject*)pf);

		pango_shape(utf8.utf8_str()+ pItem->offset,
					pItem->length,
					&(pItem->analysis),
					pGstring);
		if(pCharWidth)
		{
			for(int j=0; j<pGstring->num_glyphs; j++)
			{

				pGstring->glyphs[j].geometry.width = _tduX(pCharWidth[j]*PANGO_SCALE);
			}
		}
		pango_xft_render(m_pXftDraw, &m_XftColor, pf, pGstring, xoffD, yoffD);

		// now advance xoff
		pango_glyph_string_extents(pGstring, pf, NULL, &LR);
		xoffD += PANGO_PIXELS(LR.width);
	}

	if(pGstring)
		pango_glyph_string_free(pGstring);
	_pango_item_list_free(pItems);
}

UT_uint32 GR_UnixPangoGraphics::measureString(const UT_UCSChar * pChars,
											  int iCharOffset,
											  int iLength,
											  UT_GrowBufElement* pWidths,
											  UT_uint32 * height)
{
	UT_UTF8String utf8;
	UT_uint32 iWidth = 0;

	if (!iLength || iLength <= iCharOffset)
		return 0;
	
	if(isSymbol())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeToUnicode(pChars[i]);
		}
	}
	else if(isDingbat())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeDingbatsToUnicode(pChars[i]);
		}
	}
	else
	{
		utf8.appendUCS4(pChars + iCharOffset, iLength);
	}

	// this function expect indexes in bytes !!! (stupid)
	GList * pItems = pango_itemize(getLayoutContext(),
								   utf8.utf8_str(),
								   0, utf8.byteLength(),
								   NULL, NULL);
	
	PangoGlyphString * pGstring = pango_glyph_string_new();

	PangoFont * pf = m_pPFont->getPangoLayoutFont();
	PangoRectangle LR;
	UT_uint32 iOffset = 0;
	GList * l = pItems;

	if (height)
		*height = 0;
	
	while (l)
	{
		PangoItem *pItem = (PangoItem*)l->data;

		if(!pItem)
		{
			UT_ASSERT(pItem);
			iWidth = 0;
			goto cleanup;
		}

		// the PangoItem has to take ownership of that.
		g_object_unref(pItem->analysis.font);
		pItem->analysis.font = (PangoFont*)g_object_ref((GObject*)pf);

		pango_shape(utf8.utf8_str()+ pItem->offset,
					pItem->length,
					&(pItem->analysis),
					pGstring);

		pango_glyph_string_extents(pGstring, pf, NULL, &LR);
		iWidth += ((double) LR.width + (double)LR.x)/PANGO_SCALE;
		UT_uint32 h = LR.height/PANGO_SCALE;
		xxx_UT_DEBUGMSG(("measure string iWidth %d height %d \n",iWidth,h));
		if (height && *height < h)
			*height = h;

		int * pLogOffsets = NULL;

		/* this is rather involved, fortunately the width array is not
		 * needed most of the time we use this function in abi
		 */
		if (pWidths)
		{
			int charLength =
				UT_MIN(g_utf8_strlen(utf8.utf8_str() + pItem->offset, -1),
					   pItem->num_chars);

			xxx_UT_DEBUGMSG(("*** strlen %d, num-chars %d ***\n",
						 g_utf8_strlen(utf8.utf8_str() + pItem->offset, -1),
						 pItem->num_chars));
			
			for (int j = 0; j < charLength; /*increment manually in loop*/)
			{
				UT_sint32 iStart = j;
				UT_sint32 iEnd = j + 1;
				UT_BidiCharType iDir = pItem->analysis.level % 2 ?
					UT_BIDI_RTL : UT_BIDI_LTR;
				
				UT_uint32 iMyWidth =
					_measureExtent (pGstring, pf, iDir,
									utf8.utf8_str()+pItem->offset,
									pLogOffsets, iStart, iEnd);

				if (iEnd == j + 1)
				{
					/* this should be the case most of the time */
					pWidths[iOffset++] = iMyWidth;
				}
				else if (iEnd > j+1)
				{
					for (UT_uint32 k = iOffset;
						 k < iOffset + (iEnd - (j + 1)) + 1;
						 ++k)
					{
						pWidths[k] = iMyWidth / (iEnd - (j + 1) + 1);
					}
					iOffset += iEnd - (j + 1) + 1;
				}
				else
				{
					// iEnd < j+1 -- something badly wrong
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
					pWidths[iOffset++] = 0;
					++j;
					continue;
				}
				
				j = iEnd;
			}
		}

		delete [] pLogOffsets;

		l = l->next;
	}

	if (pWidths)
	{
		/* This is a bit weird, possibly a Pango bug, but it is better
		 * to set any dangling widths to 0 than leave them at randomn values
		 */
		while (iOffset < (UT_uint32)iLength)
		{
			pWidths[iOffset++] = 0;
		}
	}
	
	xxx_UT_DEBUGMSG(("Length %d, Offset %d\n", iLength, iOffset));
	
cleanup:
	if(pGstring)
		pango_glyph_string_free(pGstring);

	_pango_item_list_free(pItems);

	return iWidth;
}

void GR_UnixPangoGraphics::saveRectangle(UT_Rect & r, UT_uint32 iIndx)
{
	UT_Rect* oldR = NULL;	

	m_vSaveRect.setNthItem(iIndx, new UT_Rect(r),&oldR);
	if(oldR) {
		delete oldR;
	}

	GdkPixbuf * oldC = NULL;
	UT_sint32 idx = _tduX(r.left);
	UT_sint32 idy = _tduY(r.top);
	UT_sint32 idw = _tduR(r.width);
	UT_sint32 idh = _tduR(r.height);

	GdkPixbuf * pix = gdk_pixbuf_get_from_drawable(NULL,
												   _getDrawable(),
												   NULL,
												   idx, idy, 0, 0,
												   idw, idh);
	m_vSaveRectBuf.setNthItem(iIndx, pix, &oldC);

	if(oldC)
		g_object_unref (G_OBJECT (oldC));
}

void GR_UnixPangoGraphics::restoreRectangle(UT_uint32 iIndx)
{
	UT_Rect * r = m_vSaveRect.getNthItem(iIndx);
	GdkPixbuf *p = m_vSaveRectBuf.getNthItem(iIndx);
	UT_sint32 idx = _tduX(r->left);
	UT_sint32 idy = _tduY(r->top);


	if (p && r)
		gdk_draw_pixbuf (_getDrawable(), NULL, p, 0, 0,
						 idx, idy,
						 -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
}

/*!
 * Take a screenshot of the graphics and convert it to an image.
 */
GR_Image * GR_UnixPangoGraphics::genImageFromRectangle(const UT_Rect &rec)
{
	UT_sint32 idx = _tduX(rec.left);
	UT_sint32 idy = _tduY(rec.top);
	UT_sint32 idw = _tduR(rec.width);
	UT_sint32 idh = _tduR(rec.height);
	UT_return_val_if_fail (idw > 0 && idh > 0 && idx >= 0 && idy >= 0, NULL);
	GdkColormap* cmp = gdk_colormap_get_system();
	GdkPixbuf * pix = gdk_pixbuf_get_from_drawable(NULL,
												   _getDrawable(),
												   cmp,
												   idx, idy, 0, 0,
												   idw, idh);
	
	UT_return_val_if_fail(pix, NULL);

	GR_UnixImage * pImg = new GR_UnixImage("ScreenShot");
	pImg->m_image = pix;
	pImg->setDisplaySize(idw,idh);
	return static_cast<GR_Image *>(pImg);
}

/*!
 * Create a new image from the Raster rgba byte buffer defined by pBB.
 * The dimensions of iWidth and iHeight are in logical units but the image
 * doesn't scale if the resolution or zoom changes. Instead you must create
 * a new image.
 */
GR_Image* GR_UnixPangoGraphics::createNewImage (const char* pszName,
											    const UT_ByteBuf* pBB,
												UT_sint32 iWidth,
												UT_sint32 iHeight,
												GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;

	pImg = new GR_UnixImage(pszName);
	pImg->convertFromBuffer(pBB, tdu(iWidth), tdu(iHeight));
   	return pImg;
}


/*!
 * Draw the specified image at the location specified in local units 
 * (xDest,yDest). xDest and yDest are in logical units.
 */
void GR_UnixPangoGraphics::drawImage(GR_Image* pImg,
									 UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

   	GR_UnixImage * pUnixImage = static_cast<GR_UnixImage *>(pImg);

	GdkPixbuf * image = pUnixImage->getData();
	UT_return_if_fail(image);

   	UT_sint32 iImageWidth = pUnixImage->getDisplayWidth();
   	UT_sint32 iImageHeight = pUnixImage->getDisplayHeight();

	xxx_UT_DEBUGMSG(("Drawing image %d x %d\n", iImageWidth, iImageHeight));
	UT_sint32 idx = _tduX(xDest);
	UT_sint32 idy = _tduY(yDest);

	xDest = idx; yDest = idy;

	if (gdk_pixbuf_get_has_alpha (image))
		gdk_draw_pixbuf (_getDrawable(), NULL, image,
						 0, 0, xDest, yDest,
						 iImageWidth, iImageHeight,
						 GDK_RGB_DITHER_NORMAL,
						 0, 0);
	else
		gdk_draw_pixbuf (_getDrawable(), m_pGC, image,
						 0, 0, xDest, yDest,
						 iImageWidth, iImageHeight,
						 GDK_RGB_DITHER_NORMAL,
						 0, 0);
}

void GR_UnixPangoGraphics::setFont(const GR_Font * pFont)
{
	UT_return_if_fail( pFont && pFont->getType() == GR_FONT_UNIX_PANGO);

	//PangoFont * pf = (PangoFont*) pFont;
	m_pPFont = const_cast<GR_UnixPangoFont*>(static_cast<const GR_UnixPangoFont*>(pFont));

	_setIsSymbol(false);
	_setIsDingbat(false);

	char * szLCFontName = g_utf8_strdown (m_pPFont->getFamily(), -1);

	if (szLCFontName)
	{
		xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::setFont: %s\n", szLCFontName));
		if(strstr(szLCFontName,"symbol") != NULL)
		{
			/*
			 * I am not too happy about this, and do not want to see the exception
			 * list to grow much more, but cannot think of another simple solution.
			 */
			if(!strstr(szLCFontName,"starsymbol") &&
			   !strstr(szLCFontName,"opensymbol") &&
			   !strstr(szLCFontName,"symbolnerve"))
				_setIsSymbol(true);
		}
		
		if(strstr(szLCFontName,"dingbat"))
			_setIsDingbat(true);
	}
	g_free (szLCFontName); szLCFontName = NULL;
	
	if(!m_pPFont->isGuiFont() && m_pPFont->getZoom() != getZoomPercentage())
	{
		m_pPFont->reloadFont(this);
	}
}

void GR_UnixPangoGraphics::setZoomPercentage(UT_uint32 iZoom)
{
	// not sure if we should not call GR_UnixGraphics::setZoomPercentage() here
	// instead
	GR_Graphics::setZoomPercentage (iZoom); // chain up

	if(m_pPFont && !m_pPFont->isGuiFont() && m_pPFont->getZoom() != iZoom)
	{
		m_pPFont->reloadFont(this);
	}
}

GR_Font* GR_UnixPangoGraphics::getDefaultFont(UT_String& fontFamily, const char * pLang)
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, NULL );
}

UT_uint32 GR_UnixPangoGraphics::getFontAscent()
{
	return getFontAscent(m_pPFont);
}

UT_uint32 GR_UnixPangoGraphics::getFontDescent()
{
	return getFontDescent(m_pPFont);
}

UT_uint32 GR_UnixPangoGraphics::getFontHeight()
{
	return getFontHeight(m_pPFont);
}

UT_uint32 GR_UnixPangoGraphics::getFontAscent(const GR_Font * pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	const GR_UnixPangoFont * pFP = static_cast<const GR_UnixPangoFont*>(pFont);
	return pFP->getAscent();
}

UT_uint32 GR_UnixPangoGraphics::getFontDescent(const GR_Font *pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	const GR_UnixPangoFont * pFP = static_cast<const GR_UnixPangoFont*>(pFont);
	return pFP->getDescent();
}

UT_uint32 GR_UnixPangoGraphics::getFontHeight(const GR_Font *pFont)
{
	UT_return_val_if_fail( pFont, 0 );

	const GR_UnixPangoFont * pFP = static_cast<const GR_UnixPangoFont*>(pFont);
	xxx_UT_DEBUGMSG(("Font Height Pango %d \n",pFP->getAscent() + pFP->getDescent()));
	return pFP->getAscent() + pFP->getDescent();
}

/* Static 'virtual' function declared in gr_Graphics.h */
const char* GR_Graphics::findNearestFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize,
										 const char* pszFontLang)
{
	static UT_UTF8String s = pszFontFamily;
	
	double dSize = UT_convertToPoints(pszFontSize);
	int iWeight = 400;
	if (pszFontWeight)
	{
		if (!strcmp (pszFontWeight, "normal"))
			iWeight = 400;
		else if (!strcmp (pszFontWeight, "bold"))
			iWeight = 700;
		else if (!strcmp (pszFontWeight, "heavy"))
			iWeight = 900;
		else if (!strcmp (pszFontWeight, "semibold"))
			iWeight = 600;
		else if (!strcmp (pszFontWeight, "light"))
			iWeight = 300;
		else if (!strcmp (pszFontWeight, "ultralight"))
			iWeight = 200;
	}
	
	FcPattern *p = FcPatternCreate();
	if (p)
	{
		FcValue v;
		v.type = FcTypeString;
		v.u.s  = (const FcChar8*) pszFontFamily;
		FcPatternAdd(p, FC_FAMILY, v, FcFalse);

		v.u.s = (const FcChar8*) pszFontStyle;
		FcPatternAdd(p, FC_STYLE, v, FcFalse);

		v.u.s = (const FcChar8*) pszFontLang;
		FcPatternAdd(p, FC_LANG, v, FcFalse);

		v.type = FcTypeInteger;
		v.u.i  = iWeight;
		FcPatternAdd(p, FC_WEIGHT, v, FcFalse);

		v.type = FcTypeDouble;
		v.u.d  = dSize;
		FcPatternAdd(p, FC_SIZE, v, FcFalse);
		
		FcDefaultSubstitute (p);
		FcConfigSubstitute (FcConfigGetCurrent(), p, FcMatchPattern);

		// must initalize this, as FcFontMatch will only modify it if the
		// call does not succeed !!!
		FcResult r = FcResultMatch;
		FcPattern * p2 = FcFontMatch(FcConfigGetCurrent(), p, &r);

		if (r == FcResultMatch)
		{
			FcChar8 * family;
			if (FcResultMatch == FcPatternGetString(p2, FC_FAMILY, 0, &family))
			{
				s = (char*)family;
			}

			FcPatternDestroy (p2);
		}
		else
			UT_DEBUGMSG(("@@@@ ===== No match for %s, %s, %s, %s, %s, %s, %s "
						 "(%d)!!!\n",
						 pszFontFamily,
						 pszFontStyle,
						 pszFontVariant,
						 pszFontWeight,
						 pszFontStretch,
						 pszFontSize,
						 pszFontLang,
						 r));
		
		FcPatternDestroy (p);
	}

	xxx_UT_DEBUGMSG(("@@@@ ===== Requested [%s], found [%s]\n",
				 pszFontFamily, s.utf8_str()));
	
	return s.utf8_str();
}


GR_Font* GR_UnixPangoGraphics::_findFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize,
										 const char* pszLang)
{
	double dPointSize = UT_convertToPoints(pszFontSize);
	UT_String s;

	// Pango is picky about the string we pass to it -- it cannot handle any
	// 'normal' values, and it will stop parsing when it encounters one.
	const char * pStyle = pszFontStyle;
	const char * pVariant = pszFontVariant;
	const char * pWeight = pszFontWeight;
	const char * pStretch = pszFontStretch;

	if(pszFontFamily && !strcmp(pszFontFamily, "Symbol"))
	{
		pszFontFamily = "Standard Symbols L";
	}
	   
	if(pszFontStyle && *pszFontStyle == 'n')
		pStyle = "";
	else if(pszFontStyle == NULL)
	        pStyle = "";

	if(pszFontVariant && *pszFontVariant == 'n')
		pVariant = "";
	else if(pszFontVariant == NULL)
	        pVariant = "";

	if(pszFontWeight && *pszFontWeight == 'n')
		pWeight = "";
	else if(pszFontWeight == NULL)
	        pWeight = "";

	if(pszFontStretch && *pszFontStretch == 'n')
		pStretch = "";
	else if(pszFontStretch == NULL)
	        pStretch = "";

	if(!pszLang || !*pszLang)
		pszLang = "en-US";
	
	UT_String_sprintf(s, "%s, %s %s %s %s",
					  pszFontFamily,
					  pStyle,
					  pVariant,
					  pWeight,
					  pStretch);
	
	return new GR_UnixPangoFont(s.c_str(), dPointSize, this, pszLang);
}

/*!
 *  This is a very ugly hack, but Pango gives me no public API to find out the
 *  upper extent of the font coverage, so I either have to do this, or have to
 *  iterate over the entire UCS-4 space (which is a non-starter)
 *
 *  The struct definition below must match _PangoCoverage !!!
 *
 *  each block represents 256 characters, so the maximum possible character
 *  value is n_blocks * 256;
 */

struct _MyPangoCoverage
{
  guint ref_count;
  int n_blocks;
  int data_size;
  
  void *blocks;
};

typedef _MyPangoCoverage MyPangoCoverage;

void GR_UnixPangoGraphics::getCoverage(UT_NumberVector& coverage)
{
	coverage.clear();

	UT_return_if_fail(m_pPFont);

	PangoCoverage * pc = m_pPFont->getPangoCoverage();
	
	if(!pc)
		return;

	MyPangoCoverage * mpc = (MyPangoCoverage*) pc;
	UT_uint32 iMaxChar = mpc->n_blocks * 256;

	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::getCoverage: iMaxChar %d\n", iMaxChar));
	
	bool bInRange = false;
	UT_uint32 iRangeStart = 0;
	
	// Skip the coverage for character 0 as pango doesn't seem to be able to
	// handle it properly.
	// Note that for almost all fonts pango reports that it has no coverage for
	// character 0, so this is a non-issue there. However, for some (broken?) fonts 
	// like 'Fixedsys Excelsior 2.00' pango reports it *has* coverage for character 0. 
	// This will lead to crashes when attempting to shape and/or draw it, like 
	// the crash in bug 11731 - MARCM
	for(UT_uint32 i = 1; i < iMaxChar; ++i)
	{
		PangoCoverageLevel pl = pango_coverage_get(pc, i);
		
		if(PANGO_COVERAGE_NONE == pl || PANGO_COVERAGE_FALLBACK == pl)
		{
			if(bInRange)
			{
				// according to the code in XAP_UnixFont::getCoverage(), the
				// range is of type <x,y)
				coverage.push_back(i - iRangeStart);
				bInRange = false;
			}
		}
		else
		{
			if(!bInRange)
			{
				coverage.push_back(i);
				iRangeStart = i;
				bInRange = true;
			}
		}
	}
}

const std::vector<const char *> & GR_UnixPangoGraphics::getAllFontNames(void)
{
	XAP_Prefs * pPrefs = XAP_App::getApp()->getPrefs();
	bool bExclude = false;
	bool bInclude = false;

	/*
	 * Do this only once
	 */
	static std::vector<const char *> Vec;

	if (Vec.size())
		return Vec;
	
	if (pPrefs)
	{
		XAP_FontSettings & Fonts = pPrefs->getFontSettings();
		bExclude = Fonts.haveFontsToExclude();
		bInclude = Fonts.haveFontsToInclude();
		
		if (bInclude)
		{
			for (UT_uint32 k = 0; k < Fonts.getFonts().size(); ++k)
				Vec.push_back (Fonts.getFonts()[k].utf8_str());

			return Vec;
		}
	}

	UT_DEBUGMSG(("@@@@ ===== Loading system fonts =====\n"));
	FcFontSet* fs = FcConfigGetFonts(FcConfigGetCurrent(), FcSetSystem);
	
	for(UT_sint32 i = 0; i < fs->nfont; ++i)
	{
		unsigned char *family;
		FcPatternGetString(fs->fonts[i], FC_FAMILY, 0, &family);

		if (bExclude)
		{
			XAP_FontSettings & Fonts = pPrefs->getFontSettings();
			if (Fonts.isOnExcludeList((const char *)family))
			{
				UT_DEBUGMSG(("@@@@ ===== Excluding font [%s] =====\n",
							 family));
				continue;
			}
		}
		
		Vec.push_back((const char*)family);
	}

	return Vec;
}

UT_uint32 GR_UnixPangoGraphics::getAllFontCount()
{
	FcFontSet* fs;
	fs = FcConfigGetFonts(FcConfigGetCurrent(), FcSetSystem);
	return fs->nfont;
}

GR_Font * GR_UnixPangoGraphics::getDefaultFont(GR_Font::FontFamilyEnum f,
											   const char * pszLang)
{
	const char* pszFontFamily = NULL;
	const char* pszFontStyle = "normal";
	const char* pszFontVariant = "normal";
	const char* pszFontWeight = "normal";
	const char* pszFontStretch = "normal";
	const char* pszFontSize = "12pt";

	if(!pszLang)
		pszLang = "en-US";

	switch (f)
	{
		case GR_Font::FF_Roman:
			pszFontFamily = "Times";
			break;

		case GR_Font::FF_Swiss:
			pszFontFamily = "Helvetica";
			break;

		case GR_Font::FF_Modern:
			pszFontFamily = "Courier";
			break;

		case GR_Font::FF_Script:
			pszFontFamily = "Cursive";
			break;

		case GR_Font::FF_Decorative:
			pszFontFamily = "Old English";
			break;

		case GR_Font::FF_Technical:
		case GR_Font::FF_BiDi:
			pszFontFamily = "Arial";
			break;
			
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	return findFont(pszFontFamily,
					pszFontStyle,
					pszFontVariant,
					pszFontWeight,
					pszFontStretch,
					pszFontSize,
					pszLang);
}

void GR_UnixPangoGraphics::getColor(UT_RGBColor& clr)
{
	clr = m_curColor;
}

void GR_UnixPangoGraphics::setColor(const UT_RGBColor& clr)
{
	UT_ASSERT(m_pGC);
	GdkColor c;

	if (m_curColor == clr)
		return;

	m_curColor = clr;
	c.red = clr.m_red << 8;
	c.blue = clr.m_blu << 8;
	c.green = clr.m_grn << 8;

	_setColor(c);
}

void GR_UnixPangoGraphics::_setColor(GdkColor & c)
{
	gint ret = gdk_colormap_alloc_color(m_pColormap, &c, FALSE, TRUE);

	UT_ASSERT(ret == TRUE);
	if(ret)
	{
		gdk_gc_set_foreground(m_pGC, &c);
		
		m_XftColor.color.red = c.red;
		m_XftColor.color.green = c.green;
		m_XftColor.color.blue = c.blue;
		m_XftColor.color.alpha = 0xffff;
		m_XftColor.pixel = c.pixel;
		
		/* Set up the XOR gc */
		gdk_gc_set_foreground(m_pXORGC, &c);
		gdk_gc_set_function(m_pXORGC, GDK_XOR);
	}
	else 
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "gdk_colormap_alloc_color() "
			  "failed in %s", __PRETTY_FUNCTION__);
	}
}

void GR_UnixPangoGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
							   UT_sint32 x2, UT_sint32 y2)
{
	GdkGCValues gcV;
	gdk_gc_get_values(m_pGC, &gcV);
	
	UT_sint32 idx1 = _tduX(x1);
	UT_sint32 idx2 = _tduX(x2);

	UT_sint32 idy1 = _tduY(y1);
	UT_sint32 idy2 = _tduY(y2);
	
	gdk_draw_line(_getDrawable(), m_pGC, idx1, idy1, idx2, idy2);
}

void GR_UnixPangoGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = tdu(iLineWidth);

	// Get the current values of the line attributes

	GdkGCValues cur_line_att;
	gdk_gc_get_values(m_pGC, &cur_line_att);
	GdkLineStyle cur_line_style = cur_line_att.line_style;
	GdkCapStyle   cur_cap_style = cur_line_att.cap_style;
	GdkJoinStyle  cur_join_style = cur_line_att.join_style;

	// Set the new line width
	gdk_gc_set_line_attributes(m_pGC,m_iLineWidth,cur_line_style,cur_cap_style,cur_join_style);

}

static GdkCapStyle mapCapStyle ( GR_Graphics::CapStyle in )
{
	switch ( in )
    {
		case GR_Graphics::CAP_ROUND :
			return GDK_CAP_ROUND ;
		case GR_Graphics::CAP_PROJECTING :
			return GDK_CAP_PROJECTING ;
		case GR_Graphics::CAP_BUTT :
		default:
			return GDK_CAP_BUTT ;
    }
}

static GdkLineStyle mapLineStyle ( GdkGC				  * pGC, 
								   GR_Graphics::LineStyle 	in, 
								   gint 					iWidth )
{
	iWidth = iWidth == 0 ? 1 : iWidth;
	switch ( in )
    {
		case GR_Graphics::LINE_ON_OFF_DASH :
			{
				gint8 dash_list[2] = { 4*iWidth, 4*iWidth };
				gdk_gc_set_dashes(pGC, 0, dash_list, 2);
			}
			return GDK_LINE_ON_OFF_DASH ;
		case GR_Graphics::LINE_DOUBLE_DASH :
			{
				gint8 dash_list[2] = { 4*iWidth, 4*iWidth };
				gdk_gc_set_dashes(pGC, 0, dash_list, 2);
			}
			return GDK_LINE_DOUBLE_DASH ;
		case GR_Graphics::LINE_DOTTED:
			{
				/* strange but 1/3 ratio looks dotted */
				gint8 dash_list[2] = { iWidth, 3*iWidth };
				gdk_gc_set_dashes(pGC, 0, dash_list, 2);
			}
			return GDK_LINE_ON_OFF_DASH;
		case GR_Graphics::LINE_SOLID :
		default:
			return GDK_LINE_SOLID ;
    }
}

static GdkJoinStyle mapJoinStyle ( GR_Graphics::JoinStyle in )
{
	switch ( in )
    {
		case GR_Graphics::JOIN_ROUND :
			return GDK_JOIN_ROUND ;
		case GR_Graphics::JOIN_BEVEL :
			return GDK_JOIN_BEVEL ;
		case GR_Graphics::JOIN_MITER :
		default:
			return GDK_JOIN_MITER ;
    }
}


void GR_UnixPangoGraphics::setLineProperties ( double inWidth, 
										  GR_Graphics::JoinStyle inJoinStyle,
										  GR_Graphics::CapStyle inCapStyle,
										  GR_Graphics::LineStyle inLineStyle )
{
	gint iWidth = static_cast<gint>(tduD(inWidth));
	gdk_gc_set_line_attributes ( m_pGC, iWidth,
								 mapLineStyle ( m_pGC, inLineStyle, iWidth ),
								 mapCapStyle ( inCapStyle ),
								 mapJoinStyle ( inJoinStyle ) ) ;
	gdk_gc_set_line_attributes ( m_pXORGC, iWidth,
								 mapLineStyle ( m_pXORGC, inLineStyle, iWidth ), /* this was m_pGC before */
								 mapCapStyle ( inCapStyle ),
								 mapJoinStyle ( inJoinStyle ) ) ;
}

void GR_UnixPangoGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
							  UT_sint32 y2)
{
	UT_sint32 idx1 = _tduX(x1);
	UT_sint32 idx2 = _tduX(x2);

	UT_sint32 idy1 = _tduY(y1);
	UT_sint32 idy2 = _tduY(y2);

	gdk_draw_line(_getDrawable(), m_pXORGC, idx1, idy1, idx2, idy2);
}

void GR_UnixPangoGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	// see bug #303 for what this is about

	GdkPoint * points = static_cast<GdkPoint *>(UT_calloc(nPoints, sizeof(GdkPoint)));
	UT_ASSERT(points);

	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		UT_sint32 idx = _tduX(pts[i].x);
		points[i].x = idx;
		// It seems that Windows draws each pixel along the the Y axis
		// one pixel beyond where GDK draws it (even though both coordinate
		// systems start at 0,0 (?)).  Subtracting one clears this up so
		// that the poly line is in the correct place relative to where
		// the rest of GR_UnixPangoGraphics:: does things (drawing text, clearing
		// areas, etc.).
		UT_sint32 idy1 = _tduY(pts[i].y);

		points[i].y = idy1 - 1;
	}

	gdk_draw_lines(_getDrawable(), m_pGC, points, nPoints);

	FREEP(points);
}

void GR_UnixPangoGraphics::invertRect(const UT_Rect* pRect)
{
	UT_ASSERT(pRect);

	UT_sint32 idy = _tduY(pRect->top);
	UT_sint32 idx = _tduX(pRect->left);
	UT_sint32 idw = _tduR(pRect->width);
	UT_sint32 idh = _tduR(pRect->height);

	gdk_draw_rectangle(_getDrawable(), m_pXORGC, 1, idx, idy,
			   idw, idh);
}

void GR_UnixPangoGraphics::setClipRect(const UT_Rect* pRect)
{
	m_pRect = pRect;
	if (pRect)
	{
		GdkRectangle r;
		UT_sint32 idy = _tduY(pRect->top);
		UT_sint32 idx = _tduX(pRect->left);
		r.x = idx;
		r.y = idy;
		r.width = _tduR(pRect->width);
		r.height = _tduR(pRect->height);
		gdk_gc_set_clip_rectangle(m_pGC, &r);
		gdk_gc_set_clip_rectangle(m_pXORGC, &r);
		XRectangle xRect;
		xRect.x = r.x;
		xRect.y = r.y;
		xRect.width = r.width;
		xRect.height = r.height;
		XftDrawSetClipRectangles (m_pXftDraw,0,0,&xRect,1);
	}
	else
	{
		gdk_gc_set_clip_rectangle(m_pGC, NULL);
		gdk_gc_set_clip_rectangle(m_pXORGC, NULL);

		xxx_UT_DEBUGMSG(("Setting clipping rectangle NULL\n"));
		XftDrawSetClip(m_pXftDraw, 0);
	}
}

void GR_UnixPangoGraphics::fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
							   UT_sint32 w, UT_sint32 h)
{
	// save away the current color, and restore it after we fill the rect
	GdkGCValues gcValues;
	GdkColor oColor;

	memset(&oColor, 0, sizeof(GdkColor));

	gdk_gc_get_values(m_pGC, &gcValues);

	oColor.pixel = gcValues.foreground.pixel;

	// get the new color
	GdkColor nColor;

	nColor.red = c.m_red << 8;
	nColor.blue = c.m_blu << 8;
	nColor.green = c.m_grn << 8;

	gdk_colormap_alloc_color(m_pColormap, &nColor, FALSE, TRUE);

	gdk_gc_set_foreground(m_pGC, &nColor);
	UT_sint32 idx = _tduX(x);
	UT_sint32 idy = _tduY(y);
	UT_sint32 idw = _tduR(w);
	UT_sint32 idh = _tduR(h);
 	gdk_draw_rectangle(_getDrawable(), m_pGC, 1, idx, idy, idw, idh);

	gdk_gc_set_foreground(m_pGC, &oColor);
}


GR_Font * GR_UnixPangoGraphics::getGUIFont(void)
{
	if (!m_pPFontGUI)
	{
		// get the font resource
		GtkStyle *tempStyle = gtk_style_new();
		const char *guiFontName = pango_font_description_get_family(tempStyle->font_desc);
		if (!guiFontName)
			guiFontName = "'Times New Roman'";

		UT_UTF8String s = XAP_EncodingManager::get_instance()->getLanguageISOName();

		const char * pCountry
			= XAP_EncodingManager::get_instance()->getLanguageISOTerritory();
		
		if(pCountry)
		{
			s += "-";
			s += pCountry;
		}
		
		m_pPFontGUI = new GR_UnixPangoFont(guiFontName, 11.0, this, s.utf8_str(), true);

		g_object_unref(G_OBJECT(tempStyle));
		
		UT_ASSERT(m_pPFontGUI);
	}

	return m_pPFontGUI;
}

/*!
    Convert device units to pango units
*/
inline int GR_UnixPangoGraphics::dtpu(int d) const
{
	return d * PANGO_SCALE;
}

/*!
    Convert pango units to device units
*/
inline int GR_UnixPangoGraphics::ptdu(int p) const
{
	return PANGO_PIXELS(p);
}

/*!
    Convert pango units to layout units
*/
inline int GR_UnixPangoGraphics::ptlu(int p) const
{
	double d = (double)p * (double) getResolution() * 100.0 /
		((double)getDeviceResolution()*(double)getZoomPercentage()*(double) PANGO_SCALE) + .5;

	return (int) d;
}


/*!
    Convert pango units to layout units without zoom
*/
inline int GR_UnixPangoGraphics::ptlunz(int p) const
{
	double d = ((double)p / ((double) PANGO_SCALE)) + .5; //getDeviceResolution

	return (int) d;
}

/*!
    Convert layout units to pango units
*/
inline int GR_UnixPangoGraphics::ltpu(int l) const
{
	double d = (double)l *
		(double)getDeviceResolution() * (double)PANGO_SCALE * (double)getZoomPercentage()/
		(100.0 * (double) getResolution()) + .5; 
	
	return (int) d;
}


/*!
    Convert layout units to pango units without zoom
*/
inline int GR_UnixPangoGraphics::ltpunz(int l) const
{
	double d = (double)l * PANGO_SCALE  + .5; //getDeviceResolution()
	
	return (int) d;
}
	

/*!
    Convert pango font units to layout units

    (Pango font units == point size * PANGO_SCALE, hence at zoom of 100% there
    are 20/PANGO_SCALE layout units to each pango font unit.)
*/
inline int GR_UnixPangoGraphics::pftlu(int pf) const
{
	double d = (double)pf * 2000.0 / ((double)getZoomPercentage() * (double)PANGO_SCALE);
	return (int) d;
}

void GR_UnixPangoGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c,r.left,r.top,r.width,r.height);
}

void GR_UnixPangoGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	gdk_gc_set_foreground(m_pGC, &m_3dColors[c]);
	gdk_draw_rectangle(_getDrawable(), m_pGC, 1, tdu(x), tdu(y), tdu(w), tdu(h));
}

void GR_UnixPangoGraphics::polygon(UT_RGBColor& c, UT_Point *pts,
								   UT_uint32 nPoints)
{
	// save away the current color, and restore it after we draw the polygon
	GdkGCValues gcValues;
	GdkColor oColor;

	memset(&oColor, 0, sizeof(GdkColor));

	gdk_gc_get_values(m_pGC, &gcValues);

	oColor.pixel = gcValues.foreground.pixel;

	// get the new color
	GdkColor nColor;

	nColor.red = c.m_red << 8;
	nColor.blue = c.m_blu << 8;
	nColor.green = c.m_grn << 8;

	gdk_colormap_alloc_color(m_pColormap, &nColor, FALSE, TRUE);

	gdk_gc_set_foreground(m_pGC, &nColor);

	GdkPoint* points = new GdkPoint[nPoints];
    UT_ASSERT(points);

    for (UT_uint32 i = 0;i < nPoints;i++){
		UT_sint32 idx = _tduX(pts[i].x);
        points[i].x = idx;
		UT_sint32 idy = _tduY(pts[i].y);
        points[i].y = idy;
    }
	gdk_draw_polygon(_getDrawable(), m_pGC, 1, points, nPoints);
	delete[] points;

	gdk_gc_set_foreground(m_pGC, &oColor);
}

void GR_UnixPangoGraphics::clearArea(UT_sint32 x, UT_sint32 y,
									 UT_sint32 width, UT_sint32 height)
{
	if (width > 0)
	{
		static const UT_RGBColor clrWhite(255,255,255);
		fillRect(clrWhite, x, y, width, height);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GR_UnixPangFont implementation
//
GR_UnixPangoFont::GR_UnixPangoFont(const char * pDesc, double dSize,
								   GR_UnixPangoGraphics * pG,
								   const char * pLang,
								   bool bGuiFont):
	m_dPointSize(dSize),
	m_iZoom(0), // forces creation of font by reloadFont()
	m_pf(NULL),
	m_bGuiFont(bGuiFont),
	m_pCover(NULL),
	m_pfdDev(NULL),
	m_pfdLay(NULL),
	m_pPLang(NULL),
	m_iAscent(0),
	m_iDescent(0),
	m_pLayoutF(NULL)
{
	m_eType = GR_FONT_UNIX_PANGO;
	UT_return_if_fail( pDesc && pG && pLang);

	m_sLayoutDesc = pDesc;
	m_sDesc = pDesc;
	setLanguage(pLang);
	reloadFont(pG);
}

GR_UnixPangoFont::~GR_UnixPangoFont()
{
	if(m_pCover)
		pango_coverage_unref(m_pCover);
	if (m_pf) 
	{
		g_object_unref(m_pf);
	}
	if (m_pLayoutF) 
	{
		g_object_unref(m_pLayoutF);
	}
	pango_font_description_free(m_pfdDev);
	pango_font_description_free(m_pfdLay);
}

void GR_UnixPangoFont::setLanguage(const char * pLang)
{
	UT_return_if_fail( pLang );

	m_pPLang = pango_language_from_string(pLang); 
}

/*!
    Reloads the Pango font associated with this font, taking into account the
    current level of zoom
*/
void GR_UnixPangoFont::reloadFont(GR_UnixPangoGraphics * pG)
{
	UT_return_if_fail( pG );

	UT_uint32 iZoom = pG->getZoomPercentage();
	if(m_pf && (m_bGuiFont || m_iZoom == iZoom))
		return;
	
	m_iZoom = iZoom;
	
	UT_LocaleTransactor t(LC_NUMERIC, "C");
	UT_String sLay;
	UT_String sDev;
	if(!m_bGuiFont && pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_String_sprintf(sDev, "%s %f", m_sDesc.c_str(), m_dPointSize * (double)m_iZoom / 100.0);
		UT_String_sprintf(sLay, "%s %f", m_sLayoutDesc.c_str(), m_dPointSize);
	}
	else
	{
		UT_String_sprintf(sDev, "%s %f", m_sDesc.c_str(), m_dPointSize);
		UT_String_sprintf(sLay, "%s %f", m_sLayoutDesc.c_str(), m_dPointSize);
	}		

	if(m_pfdLay)
	{
		pango_font_description_free(m_pfdLay);
		m_pfdLay = NULL;
	}


	if(m_pfdDev)
	{
		pango_font_description_free(m_pfdDev);
		m_pfdDev = NULL;
	}
	
	m_pfdLay = pango_font_description_from_string(sLay.c_str());
	UT_return_if_fail(m_pfdLay);

	m_pfdDev = pango_font_description_from_string(sDev.c_str());
	UT_return_if_fail(m_pfdLay);

	if (m_pf) {
		g_object_unref(m_pf);
	}
	m_pf = pango_context_load_font(pG->getContext(), m_pfdDev);
	if(m_pLayoutF) {
		g_object_unref(m_pLayoutF);
	}
	m_pLayoutF = pango_context_load_font(pG->getLayoutContext(), m_pfdLay);

	UT_return_if_fail( m_pf );
	UT_return_if_fail( m_pLayoutF );
	// FIXME: we probably want the real language from somewhere
	PangoFontMetrics * pfm = pango_font_get_metrics(m_pLayoutF, m_pPLang);
	UT_return_if_fail( pfm);

	// pango_metrics_ functions return in points * PANGO_SCALE (points * 1024)
	m_iAscent = (UT_uint32) pango_font_metrics_get_ascent(pfm)/PANGO_SCALE;
	m_iDescent = (UT_uint32) pango_font_metrics_get_descent(pfm)/PANGO_SCALE;
	xxx_UT_DEBUGMSG(("Layout Font Ascent %d point size %f zoom %d \n",m_iAscent, m_dPointSize, m_iZoom));
	pango_font_metrics_unref(pfm);

	UT_return_if_fail( pfm);
}


/*!
	Measure the unremapped char to be put into the cache.
	That means measuring it for a font size of 120
*/
UT_sint32 GR_UnixPangoFont::measureUnremappedCharForCache(UT_UCS4Char cChar) const
{
	// this is not implemented because we do not use the width cache (when
	// shaping, it is not possible to measure characters, only glyphs)
	UT_ASSERT_HARMLESS( UT_NOT_IMPLEMENTED );
	return 0;
}

PangoCoverage * GR_UnixPangoFont::getPangoCoverage() const
{
	if(!m_pCover)
	{
		m_pCover = pango_font_get_coverage(m_pf, m_pPLang);
		UT_return_val_if_fail(m_pCover, NULL);
	}

	return m_pCover;
}


/*!
    Determine if character g exists in this font.  We assume here that coverage
    is not affected by font size -- since we only operate with single fonts and
    assume scalable fonts, this should be OK.
    
    NB: it is essential that this function is fast
*/
bool GR_UnixPangoFont::doesGlyphExist(UT_UCS4Char g)
{
	UT_return_val_if_fail( m_pf, false );

	PangoCoverage * pc = getPangoCoverage();
	UT_return_val_if_fail(pc, false);
	
	PangoCoverageLevel eLevel = pango_coverage_get(pc, g);

	if(PANGO_COVERAGE_NONE == eLevel || PANGO_COVERAGE_FALLBACK == eLevel)
		return false;

	return true;
}

bool GR_UnixPangoFont::glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG)
{
	UT_return_val_if_fail( m_pf, false );
	
	guint iGlyphIndx = pango_fc_font_get_glyph (PANGO_FC_FONT(m_pLayoutF), g);
	FT_Face pFace = pango_fc_font_lock_face(PANGO_FC_FONT(m_pLayoutF));

	double resRatio = 1.0;
	
#ifdef ENABLE_PRINT
	if(pG->canQuickPrint())
	{
		/* cast this safely */
		GR_UnixPangoPrintGraphics * pPGP = dynamic_cast<GR_UnixPangoPrintGraphics *>(pG);

		if (pPGP)
	  		resRatio = pPGP->_getResolutionRatio();

	}
#endif
	FT_Error error = FT_Load_Glyph(pFace, iGlyphIndx,
								   FT_LOAD_LINEAR_DESIGN |
								   FT_LOAD_IGNORE_TRANSFORM |
								   FT_LOAD_NO_BITMAP |
								   FT_LOAD_NO_SCALE);

	
	if (error)
	{
		pango_fc_font_unlock_face(PANGO_FC_FONT(m_pf));
		return false;
	}

	UT_uint32 iSize = (UT_uint32)(0.5 + m_dPointSize * resRatio *(double)pG->getResolution() / (double)pG->getDeviceResolution());
	
	rec.left   = static_cast<UT_sint32>(0.5 + fontPoints2float(iSize, pFace,
														 pFace->glyph->metrics.horiBearingX));
	
	rec.width  = static_cast<UT_sint32>(0.5 + fontPoints2float(iSize, pFace,
														 pFace->glyph->metrics.width));
	
	rec.top    = static_cast<UT_sint32>(0.5 + fontPoints2float(iSize, pFace,
														 pFace->glyph->metrics.horiBearingY));
	
	rec.height = static_cast<UT_sint32>(0.5 + fontPoints2float(iSize, pFace,
														 pFace->glyph->metrics.height));
	
	UT_DEBUGMSG(("GlyphBox: %c [l:%d, w:%d, t:%d, h:%d\n",
				 (char)g, rec.left,rec.width,rec.top,rec.height));

	pango_fc_font_unlock_face(PANGO_FC_FONT(m_pf));
	
	return true;
}

const char* GR_UnixPangoFont::getFamily() const
{
	UT_return_val_if_fail( m_pfdLay, NULL );
	
	return pango_font_description_get_family(m_pfdLay);
}


//////////////////////////////////////////////////////////////////////////////
//
// GR_UnixPangoRenderInfo Implementation
//

bool GR_UnixPangoRenderInfo::canAppend(GR_RenderInfo &ri) const
{
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	GR_UnixPangoItem * pItem1 = (GR_UnixPangoItem *)m_pItem;
	GR_UnixPangoItem * pItem2 = (GR_UnixPangoItem *)RI.m_pItem;

	/* Do not merger runs that have not been shapped yet */
	if (!pItem1 || !pItem2)
		return false;

	/* If the shapping resulted in font substitution we cannot merge */
	if (pItem1->m_pi->analysis.font == pItem2->m_pi->analysis.font)
		return true;

	return false;
}


bool GR_UnixPangoRenderInfo::append(GR_RenderInfo &ri, bool bReverse)
{
	if(s_pOwnerUTF8 == this)
		s_pOwnerUTF8 = NULL;

	if(s_pOwnerLogAttrs == this)
		s_pOwnerLogAttrs = NULL;
	
	delete [] m_pLogOffsets; m_pLogOffsets = NULL;

	// will be set when shaping
	m_iCharCount = 0;
	return false;
}

bool GR_UnixPangoRenderInfo::split (GR_RenderInfo *&pri, bool bReverse)
{
	UT_return_val_if_fail(m_pGraphics && m_pFont, false);

	UT_ASSERT_HARMLESS(!pri);

	// create a new RI and make a copy of item into
	if(!pri)
	{
		pri = new GR_UnixPangoRenderInfo(m_eScriptType);
		UT_return_val_if_fail(pri,false);
	}

	pri->m_pItem = m_pItem->makeCopy();
	UT_return_val_if_fail(pri->m_pItem, false);

	if(s_pOwnerUTF8 == this)
		s_pOwnerUTF8 = NULL;

	if(s_pOwnerLogAttrs == this)
		s_pOwnerLogAttrs = NULL;
	
	delete [] m_pLogOffsets; m_pLogOffsets = NULL;

	// will be set when shaping
	m_iCharCount = 0;
	
	return false;
}

bool GR_UnixPangoRenderInfo::cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse)
{

	if(s_pOwnerUTF8 == this)
		s_pOwnerUTF8 = NULL;

	if(s_pOwnerLogAttrs == this)
		s_pOwnerLogAttrs = NULL;
	
	delete [] m_pLogOffsets; m_pLogOffsets = NULL;
	
	// will be set when shaping
	m_iCharCount = 0;
	return false;
}


bool GR_UnixPangoRenderInfo::isJustified() const
{
    return (m_pJustify != NULL);
}

#ifdef ENABLE_PRINT
GR_UnixPangoPrintGraphics::GR_UnixPangoPrintGraphics(GnomePrintJob *gpm,
													 bool isPreview):
	GR_UnixPangoGraphics(),
	m_pGPFontMap(NULL),
	m_pGPContext(NULL),
	m_dResRatio(1.0),
	m_bIsPreview (isPreview),
	m_bStartPrint (false),
	m_bStartPage (false),
	m_gpm (gpm),
	m_gpc (NULL),
	m_bPdfLandscapeWorkaround(false)
{
	_constructorCommon();

	m_gpc = gnome_print_job_get_context(gpm);
	
	GnomePrintConfig * cfg = gnome_print_job_get_config (gpm);

	const GnomePrintUnit *from;
	const GnomePrintUnit *to = gnome_print_unit_get_by_abbreviation (reinterpret_cast<const guchar*>("Pt"));
	double ww,hh;
	gnome_print_config_get_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_WIDTH), &ww, &from);
	gnome_print_config_get_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_HEIGHT), &hh, &from);
	UT_DEBUGMSG(("Print construct cfg %x set Width set %f Height %f \n",cfg,ww,hh));

	gnome_print_config_get_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_WIDTH), &m_width, &from);
	gnome_print_convert_distance (&m_width, from, to);

	gnome_print_config_get_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_HEIGHT), &m_height, &from);
	gnome_print_convert_distance (&m_height, from, to);
	m_height = getDeviceResolution()*m_height/72.;
	m_width = getDeviceResolution()*m_width/72.;
	UT_DEBUGMSG(("Constructor 1 width %f height %f \n",m_width,m_height));
}

GR_UnixPangoPrintGraphics::GR_UnixPangoPrintGraphics(GnomePrintContext *ctx,
													 double inWidthDevice,
													 double inHeightDevice):
	GR_UnixPangoGraphics(),
	m_pGPFontMap(NULL),
	m_pGPContext(NULL),
	m_dResRatio(1.0),
	m_bIsPreview(false),
	m_bStartPrint(false),
	m_bStartPage(false),
	m_gpm(NULL),
	m_gpc(ctx),
	m_width(inWidthDevice),
	m_height(inHeightDevice),
	m_bPdfLandscapeWorkaround(false)
{
	_constructorCommon();
	UT_DEBUGMSG(("Constructor 2 width %d height %f \n",m_width,m_height));
}

void GR_UnixPangoPrintGraphics::_constructorCommon()
{
	setColorSpace(GR_Graphics::GR_COLORSPACE_COLOR);
	
	/* ascertain the real dpi that xft will be using, so we can match that
	 * for our
	 * gnome-print font map
	 */
	GdkScreen *  gScreen  = gdk_screen_get_default();

	// the parent class' device resolution is the screen's resolution
	m_iScreenResolution = m_iDeviceResolution;
	m_iDeviceResolution = 72; // hardcoded in GnomePrint
	m_dResRatio = static_cast<double>(m_iDeviceResolution)/
		static_cast<double>(m_iScreenResolution);
	UT_DEBUGMSG(("@@@@@@ Screen %d dpi printer %d dpi \n",
				 m_iScreenResolution, m_iDeviceResolution));

	if (gScreen)
		{
			int iScreen = gdk_x11_screen_get_screen_number(gScreen);
			Display * disp = GDK_SCREEN_XDISPLAY (gScreen);

			m_pContext = pango_xft_get_context(disp, iScreen);
			m_pFontMap = pango_xft_get_font_map(disp, iScreen);
		}
	else
		{
#ifdef HAVE_PANGOFT2
			m_bOwnsFontMap = true;
#else
			UT_DEBUGMSG(("No screen, no display, and no PangoFT2. We're screwed.\n"));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
#endif
		}
 	
	m_pGPFontMap = gnome_print_pango_get_default_font_map ();
	m_pGPContext = gnome_print_pango_create_context(m_pGPFontMap);
}

void GR_UnixPangoPrintGraphics::s_setup_config (GnomePrintConfig *cfg,
												double mrgnTop,
												double mrgnBottom,
												double mrgnLeft,
												double mrgnRight,
												double width,
												double height,
												int copies,
												bool portrait)
{
	
	const GnomePrintUnit *unit =
		gnome_print_unit_get_by_abbreviation (reinterpret_cast<const guchar*>("mm"));
	
	gnome_print_config_set (cfg, reinterpret_cast<const guchar *>(GNOME_PRINT_KEY_PAPER_SIZE), reinterpret_cast<const guchar *>("Custom"));
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAGE_MARGIN_TOP), mrgnTop, unit);
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM), mrgnBottom, unit);
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAGE_MARGIN_LEFT), mrgnLeft, unit);
	gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT), mrgnRight, unit);
	gnome_print_config_set_int (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_NUM_COPIES), copies);	

	if (portrait)
	{
		gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_WIDTH), width, unit);
		gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_HEIGHT), height, unit);
		UT_DEBUGMSG(("Portrait Setup Width set %f Height %f \n",width,height));

		gnome_print_config_set (cfg, reinterpret_cast<const guchar *>(GNOME_PRINT_KEY_PAGE_ORIENTATION) , reinterpret_cast<const guchar *>("R0"));
		gnome_print_config_set (cfg, reinterpret_cast<const guchar *>(GNOME_PRINT_KEY_PAPER_ORIENTATION) , reinterpret_cast<const guchar *>("R0"));
	}
	else
	{
		gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_WIDTH), height, unit);

		gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_HEIGHT), width, unit);
		//		gnome_print_config_set (cfg, reinterpret_cast<const guchar *>(GNOME_PRINT_KEY_ORIENTATION) , reinterpret_cast<const guchar *>("R90"));
		
		gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_LAYOUT_WIDTH),height,unit);
		gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_LAYOUT_HEIGHT),width,unit);
		gnome_print_config_set (cfg, reinterpret_cast<const guchar *>(GNOME_PRINT_KEY_PAGE_ORIENTATION) , reinterpret_cast<const guchar *>("R90"));
		gnome_print_config_set (cfg, reinterpret_cast<const guchar *>(GNOME_PRINT_KEY_PAPER_ORIENTATION) , reinterpret_cast<const guchar *>("R90"));

	}
}

GnomePrintConfig * GR_UnixPangoPrintGraphics::s_setup_config (double mrgnTop,
															  double mrgnBottom,
															  double mrgnLeft,
															  double mrgnRight,
															  double width,
															  double height,
															  int copies,
															  bool portrait)
{
	GnomePrintConfig * cfg = gnome_print_config_default();
	s_setup_config (cfg, mrgnTop, mrgnBottom, mrgnLeft, mrgnRight, width, height, copies, portrait);
	return cfg;
}

GR_UnixPangoPrintGraphics::~GR_UnixPangoPrintGraphics()
{
}

GR_Graphics * GR_UnixPangoPrintGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_UNIX, NULL);
	xxx_UT_DEBUGMSG(("GR_UnixPangoGraphics::graphicsAllocator\n"));

	UT_return_val_if_fail(info.isPrinterGraphics(), NULL);
	GR_UnixAllocInfo &AI = (GR_UnixAllocInfo&)info;

	return new GR_UnixPangoPrintGraphics(AI.m_gpm, AI.m_bPreview);
}

GnomePrintContext * GR_UnixPangoPrintGraphics::getGnomePrintContext() const
{
	return m_gpc;
}


UT_uint32 GR_UnixPangoPrintGraphics::getFontAscent()
{
	return static_cast<UT_uint32>(static_cast<double>(GR_UnixPangoGraphics::getFontAscent()));
}

UT_uint32 GR_UnixPangoPrintGraphics::getFontDescent()
{
	return static_cast<UT_uint32>(static_cast<double>(GR_UnixPangoGraphics::getFontDescent()));
}

UT_uint32 GR_UnixPangoPrintGraphics::getFontHeight()
{
	return static_cast<UT_uint32>(static_cast<double>(GR_UnixPangoGraphics::getFontHeight()));
}

UT_uint32 GR_UnixPangoPrintGraphics::getFontAscent(const GR_Font * fnt)
{
	return static_cast<UT_uint32>(static_cast<double>(GR_UnixPangoGraphics::getFontAscent(fnt)));
}

UT_uint32 GR_UnixPangoPrintGraphics::getFontDescent(const GR_Font * fnt )
{
	return static_cast<UT_uint32>(static_cast<double>(GR_UnixPangoGraphics::getFontDescent(fnt)));
}

UT_uint32 GR_UnixPangoPrintGraphics::getFontHeight(const GR_Font * fnt)
{
	return static_cast<UT_uint32>(static_cast<double>(GR_UnixPangoGraphics::getFontHeight(fnt)));
}

UT_sint32 GR_UnixPangoPrintGraphics::scale_ydir (UT_sint32 in) const
{
	double height;

	if (isPortrait() || m_bPdfLandscapeWorkaround )
		height = m_height;
	else
		height = m_width;

	return static_cast<UT_sint32>(height - static_cast<double>(in));
}

UT_sint32 GR_UnixPangoPrintGraphics::scale_xdir (UT_sint32 in) const
{
	return in;
}

void GR_UnixPangoPrintGraphics::drawChars(const UT_UCSChar* pChars, 
										   int iCharOffset, int iLength,
										   UT_sint32 xoff, UT_sint32 yoff,
										   int * pCharWidths)
{
	UT_UTF8String utf8;

	if(isSymbol())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeToUnicode(pChars[i]);
		}
	}
	else if(isDingbat())
	{
		for(int i = iCharOffset; i < iCharOffset + iLength; ++i)
		{
			utf8 += adobeDingbatsToUnicode(pChars[i]);
		}
	}
	else
	{
		utf8.appendUCS4(pChars + iCharOffset, iLength);
	}
	
	GList * pLogItems = pango_itemize(m_pContext, utf8.utf8_str(),
									  0, utf8.byteLength(),
									  NULL, NULL);

	GList * pItems = pango_reorder_items(pLogItems);
	g_list_free(pLogItems);
	
	xoff = _tduX(xoff);
	yoff = scale_ydir(_tduY(yoff + getFontAscent(m_pPFont)));
	// yoff = scale_ydir(_tduY(yoff));

	UT_return_if_fail( m_gpc );

	gnome_print_gsave(m_gpc);
	gnome_print_moveto(m_gpc, xoff, yoff);

	PangoFontDescription * pdf = pango_font_describe (m_pPFont->getPangoFont());
	PangoFont * pf = pango_context_load_font (m_pGPContext, pdf);
	pango_font_description_free (pdf);
	UT_return_if_fail( pf );
	
	for(unsigned int i = 0; i < g_list_length(pItems); ++i)
	{
		PangoGlyphString * pGlyphs = pango_glyph_string_new();
		PangoItem *pItem = (PangoItem *)g_list_nth(pItems, i)->data;
		g_object_unref(pItem->analysis.font);
		pItem->analysis.font = (PangoFont*)g_object_ref((GObject*)pf);

		pango_shape(utf8.utf8_str() + pItem->offset, pItem->length,
					& pItem->analysis, pGlyphs);

		gnome_print_pango_glyph_string(m_gpc, pf, pGlyphs);

		if(pGlyphs)
			pango_glyph_string_free(pGlyphs);
	}

	gnome_print_grestore (m_gpc);
 	_pango_item_list_free(pItems);
}

bool GR_UnixPangoPrintGraphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& ri)
{
	
	if (!GR_UnixPangoGraphics::shape(si,ri))
		return false;

	UT_return_val_if_fail( ri, false );

	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)*ri;
#if 0
	for(int i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		RI.m_pGlyphs->glyphs[i].geometry.x_offset =
			(int)((double)RI.m_pGlyphs->glyphs[i].geometry.x_offset *
				  (double)m_iDeviceResolution / (double)m_iScreenResolution + 0.5) ;

		RI.m_pGlyphs->glyphs[i].geometry.y_offset =
			(int)((double)RI.m_pGlyphs->glyphs[i].geometry.y_offset *
				  (double)m_iDeviceResolution / (double)m_iScreenResolution + 0.5);

		RI.m_pGlyphs->glyphs[i].geometry.width =
			(int)((double)RI.m_pGlyphs->glyphs[i].geometry.width *
				  (double)m_iDeviceResolution / (double)m_iScreenResolution + 0.5);
	}
#endif
	return true;
}

void GR_UnixPangoPrintGraphics::renderChars(GR_RenderInfo & ri)
{
	xxx_UT_DEBUGMSG(("GR_UnixPangoPrintGraphics::renderChars\n"));
	UT_return_if_fail(ri.getType() == GRRI_UNIX_PANGO);
	GR_UnixPangoRenderInfo & RI = (GR_UnixPangoRenderInfo &)ri;
	GR_UnixPangoFont * pFont = (GR_UnixPangoFont *)RI.m_pFont;
	GR_UnixPangoItem * pItem = (GR_UnixPangoItem *)RI.m_pItem;
	UT_return_if_fail(pItem && pFont && pFont->getPangoFont());

	if(RI.m_iLength == 0)
		return;

	xxx_UT_DEBUGMSG(("PangoPrint renderChars: xoff %d yoff %d\n", RI.m_xoff, RI.m_yoff));
	UT_sint32 xoff = _tduX(RI.m_xoff);
	UT_sint32 yoff = scale_ydir(_tduY((RI.m_yoff + getFontAscent(pFont))));

	xxx_UT_DEBUGMSG(("about to gnome_print_pango_gplyph_string render xoff %d yoff %d\n",
				 xoff, yoff));

	UT_return_if_fail(m_gpc);

	gnome_print_gsave(m_gpc);
	gnome_print_moveto(m_gpc, xoff, yoff);

	PangoFont * pf1 = _adjustedPangoFont(pFont, pItem->m_pi->analysis.font);
	
	PangoFontDescription * pfd = pango_font_describe (pf1);
	PangoFont * pf = pango_context_load_font (m_pContext, pfd);
#ifdef DEBUG
	char * psz = pango_font_description_to_string (pfd);
	xxx_UT_DEBUGMSG(("XXXX Loaded GP font [%s] XXXX\n", psz));
	g_free (psz);
#endif
	
	pango_font_description_free (pfd);

#define _N 1440
	xxx_UT_DEBUGMSG(("@@@@ tdu(%d)== %d, _tduX(%d) == %d, _tduY(%d) == %d\n",
				 _N, tdu(_N), _N, _tduX(_N), _N, _tduY(_N)));
#undef _N

	for(int i = 0; i < RI.m_pGlyphs->num_glyphs; ++i)
	{
		RI.m_pScaledGlyphs->glyphs[i].geometry.x_offset =
			_tduX(RI.m_pGlyphs->glyphs[i].geometry.x_offset);


		RI.m_pScaledGlyphs->glyphs[i].geometry.y_offset = 
			_tduY(RI.m_pGlyphs->glyphs[i].geometry.y_offset);

		RI.m_pScaledGlyphs->glyphs[i].geometry.width =
			_tduX(RI.m_pGlyphs->glyphs[i].geometry.width );
	}

	gnome_print_pango_glyph_string(m_gpc, pf, RI.m_pScaledGlyphs);

	gnome_print_grestore (m_gpc);
}

void GR_UnixPangoPrintGraphics::drawLine (UT_sint32 x1, UT_sint32 y1,
										   UT_sint32 x2, UT_sint32 y2)
{
	if (!m_bStartPage)
		return;
	gnome_print_moveto (m_gpc, scale_xdir (tdu(x1)), scale_ydir (tdu(y1)));
	gnome_print_lineto (m_gpc, scale_xdir (tdu(x2)), scale_ydir (tdu(y2)));
	gnome_print_stroke (m_gpc);
}

bool GR_UnixPangoPrintGraphics::queryProperties(GR_Graphics::Properties gp) const
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

void GR_UnixPangoPrintGraphics::getColor(UT_RGBColor& clr)
{
	clr = m_curColor;
}

void GR_UnixPangoPrintGraphics::setColor(const UT_RGBColor& clr)
{
	if (!m_bStartPrint || m_curColor == clr)
		return;

	m_curColor = clr;

	double red   = static_cast<double>(m_curColor.m_red / 255.0);
	double green = static_cast<double>(m_curColor.m_grn / 255.0);
	double blue  = static_cast<double>(m_curColor.m_blu / 255.0);
	gnome_print_setrgbcolor(m_gpc,red,green,blue);
}

void GR_UnixPangoPrintGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	if (!m_bStartPage)
		return;

 	m_dLineWidth = tduD(static_cast<double>(iLineWidth));
	gnome_print_setlinewidth (m_gpc, m_dLineWidth); 
}

bool GR_UnixPangoPrintGraphics::_startDocument(void)
{
	return true;
}

bool GR_UnixPangoPrintGraphics::_startPage(const char * szPageLabel)
{
	if (!m_gpc)
		return true;
	
	gnome_print_beginpage(m_gpc, reinterpret_cast<const guchar *>(szPageLabel));

	return true;
}

bool GR_UnixPangoPrintGraphics::_endPage(void)
{
	if(m_bNeedStroked)
		gnome_print_stroke(m_gpc);
	
	if (!m_gpc)
		return true;

	gnome_print_showpage(m_gpc);
	return true;
}

bool GR_UnixPangoPrintGraphics::_endDocument(void)
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

UT_uint32 GR_UnixPangoPrintGraphics::_getResolution(void) const
{
	return 72; // was 72
}

bool GR_UnixPangoPrintGraphics::startPrint(void)
{
	UT_ASSERT(!m_bStartPrint || !m_gpm);
	m_bStartPrint = true;
	return _startDocument();
}

bool GR_UnixPangoPrintGraphics::startPage (const char *szPageLabel,
											UT_uint32 pageNo, bool portrait, 
											UT_uint32 width, UT_uint32 height)
{
	if (m_bStartPage)
		_endPage();
	m_bStartPage = true;
	m_bNeedStroked = false;
	return _startPage(szPageLabel);
}

bool GR_UnixPangoPrintGraphics::endPrint()
{
	if (m_bStartPage)
		_endPage();
	return _endDocument();
}

void GR_UnixPangoPrintGraphics::setColorSpace(GR_Graphics::ColorSpace c)
{
	m_cs = c;
}

GR_Graphics::ColorSpace GR_UnixPangoPrintGraphics::getColorSpace(void) const
{
	return m_cs;
}

UT_uint32 GR_UnixPangoPrintGraphics::getDeviceResolution(void) const
{
	return _getResolution();
}

void GR_UnixPangoPrintGraphics::_drawAnyImage (GR_Image* pImg, 
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

void GR_UnixPangoPrintGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, 
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
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
	}
}

GR_Image* GR_UnixPangoPrintGraphics::createNewImage(const char* pszName, 
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

void GR_UnixPangoPrintGraphics::fillRect(const UT_RGBColor& c, 
										  UT_sint32 x, UT_sint32 y, 
										  UT_sint32 w, UT_sint32 h)
{
	if (!m_bStartPage)
		return;

	// set the bgcolor
	UT_RGBColor old (m_curColor);
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

void GR_UnixPangoPrintGraphics::setCursor(GR_Graphics::Cursor c)
{
	UT_ASSERT_NOT_REACHED ();
}

GR_Graphics::Cursor GR_UnixPangoPrintGraphics::getCursor(void) const
{
	UT_ASSERT_NOT_REACHED ();
	return GR_CURSOR_INVALID;
}

void GR_UnixPangoPrintGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, 
										 UT_sint32 y2)
{
	UT_ASSERT_NOT_REACHED ();
}

void GR_UnixPangoPrintGraphics::polyLine(UT_Point * pts, 
										  UT_uint32 nPoints)
{
	UT_ASSERT_NOT_REACHED ();
}

void GR_UnixPangoPrintGraphics::invertRect(const UT_Rect* pRect)
{
	UT_ASSERT_NOT_REACHED ();
}

void GR_UnixPangoPrintGraphics::clearArea(UT_sint32 x, UT_sint32 y,
										   UT_sint32 width, UT_sint32 height)
{
	UT_ASSERT_NOT_REACHED ();
}

void GR_UnixPangoPrintGraphics::scroll(UT_sint32 x, UT_sint32 y)
{
	UT_ASSERT_NOT_REACHED ();
}

void GR_UnixPangoPrintGraphics::scroll(UT_sint32 x_dest,
										UT_sint32 y_dest,
										UT_sint32 x_src,
										UT_sint32 y_src,
										UT_sint32 width,
										UT_sint32 height)
{
	UT_ASSERT_NOT_REACHED ();
}

UT_RGBColor * GR_UnixPangoPrintGraphics::getColor3D(GR_Color3D c)
{
	UT_ASSERT_NOT_REACHED ();
	return NULL;
}

void GR_UnixPangoPrintGraphics::setColor3D(GR_Color3D c)
{
	UT_ASSERT_NOT_REACHED ();
}

GR_Font* GR_UnixPangoPrintGraphics::getGUIFont()
{
	UT_ASSERT_NOT_REACHED ();
	return NULL;
}

void GR_UnixPangoPrintGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y,
										  UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT_NOT_REACHED ();
}

void GR_UnixPangoPrintGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT_NOT_REACHED ();
}

void GR_UnixPangoPrintGraphics::setPageSize(char* pageSizeName,
											UT_uint32 iwidth, UT_uint32 iheight)
{
	UT_ASSERT_NOT_REACHED ();
}

void GR_UnixPangoPrintGraphics::setClipRect(const UT_Rect* pRect)
{
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
			UT_ASSERT_HARMLESS(UT_TODO);
			offset = 0.; n_values = 0; return NULL;
		}
	
	n_values = 0; offset = 0.; return NULL;
}

void GR_UnixPangoPrintGraphics::setLineProperties (double inWidth,
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

#endif


static const UT_uint32 adobeDUni[/*202*/][2] =
	{
		{0x0020,0x0020},
		{0x0021,0x2701},
		{0x0022,0x2702},
		{0x0023,0x2703},
		{0x0024,0x2704},
		{0x0025,0x260E},
		{0x0026,0x2706},
		{0x0027,0x2707},
		{0x0028,0x2708},
		{0x0029,0x2709},
		{0x002A,0x261B},
		{0x002B,0x261E},
		{0x002C,0x270C},
		{0x002D,0x270D},
		{0x002E,0x270E},
		{0x002F,0x270F},
		{0x0030,0x2710},
		{0x0031,0x2711},
		{0x0032,0x2712},
		{0x0033,0x2713},
		{0x0034,0x2714},
		{0x0035,0x2715},
		{0x0036,0x2716},
		{0x0037,0x2717},
		{0x0038,0x2718},
		{0x0039,0x2719},
		{0x003A,0x271A},
		{0x003B,0x271B},
		{0x003C,0x271C},
		{0x003D,0x271D},
		{0x003E,0x271E},
		{0x003F,0x271F},
		{0x0040,0x2720},
		{0x0041,0x2721},
		{0x0042,0x2722},
		{0x0043,0x2723},
		{0x0044,0x2724},
		{0x0045,0x2725},
		{0x0046,0x2726},
		{0x0047,0x2727},
		{0x0048,0x2605},
		{0x0049,0x2729},
		{0x004A,0x272A},
		{0x004B,0x272B},
		{0x004C,0x272C},
		{0x004D,0x272D},
		{0x004E,0x272E},
		{0x004F,0x272F},
		{0x0050,0x2730},
		{0x0051,0x2731},
		{0x0052,0x2732},
		{0x0053,0x2733},
		{0x0054,0x2734},
		{0x0055,0x2735},
		{0x0056,0x2736},
		{0x0057,0x2737},
		{0x0058,0x2738},
		{0x0059,0x2739},
		{0x005A,0x273A},
		{0x005B,0x273B},
		{0x005C,0x273C},
		{0x005D,0x273D},
		{0x005E,0x273E},
		{0x005F,0x273F},
		{0x0060,0x2740},
		{0x0061,0x2741},
		{0x0062,0x2742},
		{0x0063,0x2743},
		{0x0064,0x2744},
		{0x0065,0x2745},
		{0x0066,0x2746},
		{0x0067,0x2747},
		{0x0068,0x2748},
		{0x0069,0x2749},
		{0x006A,0x274A},
		{0x006B,0x274B},
		{0x006C,0x25CF},
		{0x006D,0x274D},
		{0x006E,0x25A0},
		{0x006F,0x274F},
		{0x0070,0x2750},
		{0x0071,0x2751},
		{0x0072,0x2752},
		{0x0073,0x25B2},
		{0x0074,0x25BC},
		{0x0075,0x25C6},
		{0x0076,0x2756},
		{0x0077,0x25D7},
		{0x0078,0x2758},
		{0x0079,0x2759},
		{0x007A,0x275A},
		{0x007B,0x275B},
		{0x007C,0x275C},
		{0x007D,0x275D},
		{0x007E,0x275E},
		{0x0080,0xF8D7},
		{0x0081,0xF8D8},
		{0x0082,0xF8D9},
		{0x0083,0xF8DA},
		{0x0084,0xF8DB},
		{0x0085,0xF8DC},
		{0x0086,0xF8DD},
		{0x0087,0xF8DE},
		{0x0088,0xF8DF},
		{0x0089,0xF8E0},
		{0x008A,0xF8E1},
		{0x008B,0xF8E2},
		{0x008C,0xF8E3},
		{0x008D,0xF8E4},
		{0x00A1,0x2761},
		{0x00A2,0x2762},
		{0x00A3,0x2763},
		{0x00A4,0x2764},
		{0x00A5,0x2765},
		{0x00A6,0x2766},
		{0x00A7,0x2767},
		{0x00A8,0x2663},
		{0x00A9,0x2666},
		{0x00AA,0x2665},
		{0x00AB,0x2660},
		{0x00AC,0x2460},
		{0x00AD,0x2461},
		{0x00AE,0x2462},
		{0x00AF,0x2463},
		{0x00B0,0x2464},
		{0x00B1,0x2465},
		{0x00B2,0x2466},
		{0x00B3,0x2467},
		{0x00B4,0x2468},
		{0x00B5,0x2469},
		{0x00B6,0x2776},
		{0x00B7,0x2777},
		{0x00B8,0x2778},
		{0x00B9,0x2779},
		{0x00BA,0x277A},
		{0x00BB,0x277B},
		{0x00BC,0x277C},
		{0x00BD,0x277D},
		{0x00BE,0x277E},
		{0x00BF,0x277F},
		{0x00C0,0x2780},
		{0x00C1,0x2781},
		{0x00C2,0x2782},
		{0x00C3,0x2783},
		{0x00C4,0x2784},
		{0x00C5,0x2785},
		{0x00C6,0x2786},
		{0x00C7,0x2787},
		{0x00C8,0x2788},
		{0x00C9,0x2789},
		{0x00CA,0x278A},
		{0x00CB,0x278B},
		{0x00CC,0x278C},
		{0x00CD,0x278D},
		{0x00CE,0x278E},
		{0x00CF,0x278F},
		{0x00D0,0x2790},
		{0x00D1,0x2791},
		{0x00D2,0x2792},
		{0x00D3,0x2793},
		{0x00D4,0x2794},
		{0x00D5,0x2192},
		{0x00D6,0x2194},
		{0x00D7,0x2195},
		{0x00D8,0x2798},
		{0x00D9,0x2799},
		{0x00DA,0x279A},
		{0x00DB,0x279B},
		{0x00DC,0x279C},
		{0x00DD,0x279D},
		{0x00DE,0x279E},
		{0x00DF,0x279F},
		{0x00E0,0x27A0},
		{0x00E1,0x27A1},
		{0x00E2,0x27A2},
		{0x00E3,0x27A3},
		{0x00E4,0x27A4},
		{0x00E5,0x27A5},
		{0x00E6,0x27A6},
		{0x00E7,0x27A7},
		{0x00E8,0x27A8},
		{0x00E9,0x27A9},
		{0x00EA,0x27AA},
		{0x00EB,0x27AB},
		{0x00EC,0x27AC},
		{0x00ED,0x27AD},
		{0x00EE,0x27AE},
		{0x00EF,0x27AF},
		{0x00F1,0x27B1},
		{0x00F2,0x27B2},
		{0x00F3,0x27B3},
		{0x00F4,0x27B4},
		{0x00F5,0x27B5},
		{0x00F6,0x27B6},
		{0x00F7,0x27B7},
		{0x00F8,0x27B8},
		{0x00F9,0x27B9},
		{0x00FA,0x27BA},
		{0x00FB,0x27BB},
		{0x00FC,0x27BC},
		{0x00FD,0x27BD},
		{0x00FE,0x27BE},
		{255,100000}
	};

// see e.g. http://www.unicode.org/Public/MAPPINGS/VENDORS/ADOBE/symbol.txt
static const UT_uint32 adobeSUni[/*185*/][2] =
	{
		{32,32},
		{33,33},
		{34,8704},
		{35,35},
		{36,8707},
		{37,37},
		{38,38},
		{39,8715},
		{40,40},
		{41,41},
		{42,8727},
		{43,43},
		{44,44},
		{45,8722},
		{46,46},
		{47,47},
		{48,48},
		{49,49},
		{50,50},
		{51,51},
		{52,52},
		{53,53},
		{54,54},
		{55,55},
		{56,56},
		{57,57},
		{58,58},
		{59,59},
		{60,60},
		{61,61},
		{62,62},
		{63,63},
		{64,8773},
		{65,913},
		{66,914},
		{67,935},
		{68,8710},
		{69,917},
		{70,934},
		{71,915},
		{72,919},
		{73,921},
		{74,977},
		{75,922},
		{76,923},
		{77,924},
		{78,925},
		{79,927},
		{80,928},
		{81,920},
		{82,929},
		{83,931},
		{84,932},
		{85,933},
		{86,962},
		{87,8486},
		{88,926},
		{89,936},
		{90,918},
		{91,91},
		{92,8756},
		{93,93},
		{94,8869},
		{95,95},
		{96,63717},
		{97,945},
		{98,946},
		{99,967},
		{100,948},
		{101,949},
		{102,966},
		{103,947},
		{104,951},
		{105,953},
		{106,981},
		{107,954},
		{108,955},
		{109,181},
		{110,957},
		{111,959},
		{112,960},
		{113,952},
		{114,961},
		{115,963},
		{116,964},
		{117,965},
		{119,969},
		{120,958},
		{121,968},
		{122,950},
		{123,123},
		{124,124},
		{125,125},
		{126,8764},
		{163,8804},
		{164,8260},
		{165,8734},
		{166,402},
		{167,9827},
		{168,9830},
		{169,9829},
		{170,9824},
		{171,8596},
		{172,8592},
		{173,8593},
		{174,8594},
		{175,8595},
		{176,176},
		{177,177},
		{179,8805},
		{180,215},
		{181,8733},
		{182,8706},
		{183,8226},
		{184,247},
		{185,8800},
		{186,8801},
		{187,8776},
		{188,8230},
		{189,63718},
		{190,63719},
		{191,8629},
		{192,8501},
		{193,8465},
		{194,8476},
		{195,8472},
		{196,8855},
		{197,8853},
		{198,8709},
		{199,8745},
		{200,8746},
		{201,8835},
		{202,8839},
		{203,8836},
		{204,8834},
		{205,8838},
		{206,8712},
		{207,8713},
		{208,8736},
		{209,8711},
		{210,63194},
		{211,63193},
		{212,63195},
		{213,8719},
		{214,8730},
		{215,8901},
		{216,172},
		{217,8743},
		{218,8744},
		{219,8660},
		{220,8656},
		{221,8657},
		{222,8658},
		{223,8659},
		{224,9674},
		{225,9001},
		{226,63720},
		{227,63721},
		{228,63722},
		{229,8721},
		{230,63723},
		{231,63724},
		{232,63725},
		{233,63726},
		{234,63727},
		{235,63728},
		{236,63729},
		{237,63730},
		{238,63731},
		{239,63732},
		{241,9002},
		{242,8747},
		{243,8992},
		{244,63733},
		{245,8993},
		{246,63734},
		{247,63735},
		{248,63736},
		{249,63737},
		{250,63738},
		{251,63739},
		{252,63740},
		{253,63741},
		{254,63742},
		{255,100000}
	};

UT_uint32 adobeToUnicode(UT_uint32 iAdobe)
{
	UT_uint32 low = adobeSUni[0][0];
	UT_uint32 high = adobeSUni[183][0];
	if(iAdobe < low)
	{
		return iAdobe;
	}
	if(iAdobe > high)
	{
		return iAdobe;
	}
	UT_sint32 slow = static_cast<UT_sint32>(iAdobe) - 72;
	if(slow < 0)
	{ 
		slow = 0;
	}
	while(adobeSUni[slow][0] != iAdobe && slow < 255)
	{
		xxx_UT_DEBUGMSG(("char at %d is %d value %d \n",slow,adobeSUni[slow][0],adobeSUni[slow][1]));
		slow++;
	}
	xxx_UT_DEBUGMSG(("Input %d return %d \n",iAdobe,adobeSUni[slow][1]));
	if(slow > 255)
	{
		return iAdobe;
	}
	return adobeSUni[slow][1];
}

UT_uint32 adobeDingbatsToUnicode(UT_uint32 iAdobe)
{
	
#if 1
	UT_uint32 low = adobeDUni[0][0];
	UT_uint32 high = adobeDUni[202][0];
	if(iAdobe < low)
	{
		return iAdobe;
	}
	if(iAdobe > high)
	{
		return iAdobe;
	}
	UT_sint32 slow = static_cast<UT_sint32>(iAdobe) - 32;
	if(slow < 0)
	{ 
		slow = 0;
	}
	while(adobeDUni[slow][0] != iAdobe && slow < 255)
	{
		xxx_UT_DEBUGMSG(("char at %d is %d value %d \n",slow,adobeDUni[slow][0],adobeSUni[slow][1]));
		slow++;
	}
	xxx_UT_DEBUGMSG(("Input %d return %d \n",iAdobe,adobeDUni[slow][1]));
	if(slow > 255)
	{
		return iAdobe;
	}
	return adobeDUni[slow][1];
#endif
}

void GR_Font::s_getGenericFontProperties(const char * /*szFontName*/,
										 FontFamilyEnum * pff,
										 FontPitchEnum * pfp,
										 bool * pbTrueType)
{
	// describe in generic terms the named font.

	// Note: most of the unix font handling code is in abi/src/af/xap/unix
	// Note: rather than in the graphics class.  i'm not sure this matters,
	// Note: but it is just different....

	// TODO add code to map the given font name into one of the
	// TODO enums in GR_Font and set *pff and *pft.

	*pff = FF_Unknown;
	*pfp = FP_Unknown;
	*pbTrueType = true;
}
