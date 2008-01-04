/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2007 Tomas Frydrych
 * Copyright (C) 2007 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "gr_UnixPangoPixmapGraphics.h"

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

#ifdef HAVE_PANGOFT2
#include <pango/pangoft2.h>
#endif

#include <math.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

/*!
 * The GR_UnixPangoPixmapGraphics is used to draw onto an offscreen 
 * buffer.
 */

GR_UnixPangoPixmapGraphics::GR_UnixPangoPixmapGraphics(GdkPixmap * pix): 
	GR_UnixPangoGraphics(),
	m_pPixmap(pix)
{
	init();
}			

GR_UnixPangoPixmapGraphics::~GR_UnixPangoPixmapGraphics(void)
{
	gdk_pixmap_unref (m_pPixmap);	
}

GR_Graphics *   GR_UnixPangoPixmapGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_UNIX_PANGO_PIXMAP,NULL);
	xxx_UT_DEBUGMSG(("GR_UnixPangoPixmapGraphics::graphicsAllocator\n"));

	GR_UnixPixmapAllocInfo &AI = (GR_UnixPixmapAllocInfo&)info;

	return new GR_UnixPangoPixmapGraphics(AI.m_pix);
}


void GR_UnixPangoPixmapGraphics::init(void)
{
	UT_DEBUGMSG(("Initializing UnixPangoPixmapGraphics %x \n",this));
	GdkDisplay * gDisplay = NULL;
	GdkScreen *  gScreen = NULL;
	UT_return_if_fail(_getDrawable());

	m_pColormap = gdk_rgb_get_colormap();
	m_Colormap = GDK_COLORMAP_XCOLORMAP(m_pColormap);

	gDisplay = gdk_drawable_get_display(_getDrawable());
	gScreen = gdk_drawable_get_screen(_getDrawable());

	if(!gDisplay)
		gDisplay = gdk_display_get_default();
	if(!gScreen)
		gScreen = gdk_screen_get_default();

	GdkDrawable * realDraw;
	realDraw = _getDrawable();
	m_iXoff = 0;
	m_iYoff = 0;
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
	
	m_bIsSymbol = false;
	m_bIsDingbat = false;

	bool bGotResolution = false;
	
	if (gScreen && gDisplay)
		{
			int iScreen = gdk_x11_screen_get_screen_number(gScreen);
			Display * disp = GDK_DISPLAY_XDISPLAY(gDisplay);
			UT_DEBUGMSG(("PangoPixmap init gScreen %x gDisplay %x Screen %d Display %x \n",gScreen,gDisplay,iScreen,disp));
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
		m_iDeviceResolution = 72;
		m_pFontMap = pango_ft2_font_map_new ();
		pango_ft2_font_map_set_resolution(reinterpret_cast<PangoFT2FontMap*>(m_pFontMap), 
										  m_iDeviceResolution,
										  m_iDeviceResolution);	
		m_pContext = pango_ft2_font_map_create_context(reinterpret_cast<PangoFT2FontMap*>(m_pFontMap));
		m_bOwnsFontMap = true;
	}
#endif
}

bool GR_UnixPangoPixmapGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
		case DGP_SCREEN:
			return false;
		case DGP_OPAQUEOVERLAY:
			return true;
		case DGP_PAPER:
			return true;
		default:
			UT_ASSERT(0);
			return false;
	}
}

GR_Graphics::Cursor GR_UnixPangoPixmapGraphics::getCursor(void) const
{
	return GR_CURSOR_DEFAULT;
}
