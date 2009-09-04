/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2006 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2009 Hubert Figuiere
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

#include "ut_bytebuf.h"

#include "gr_UnixCairoGraphics.h"
#include "gr_CairoImage.h"
#include "gr_Painter.h"
#include "gr_UnixImage.h"

#include "xap_App.h"
#include "xap_EncodingManager.h"

#define GTK_CHECK_VERSION(major,minor,micro) \
	(GTK_MAJOR_VERSION > (major) || \
	(GTK_MAJOR_VERSION == (major) && GTK_MINOR_VERSION > (minor)) || \
	(GTK_MAJOR_VERSION == (major) && GTK_MINOR_VERSION == (minor) && \
	GTK_MICRO_VERSION >= (micro)))

// remove this define and our own gdk_cairo_reset_clip function when gtk 2.18 is released
#define GTK_2_17_DEV_VERSION_WITHOUT_RESET_CLIP \
	(GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION == 17 && \
    (GTK_MICRO_VERSION >= 0 && GTK_MICRO_VERSION <= 10))

#if GTK_2_17_DEV_VERSION_WITHOUT_RESET_CLIP
void gdk_cairo_reset_clip (cairo_t *cr, GdkDrawable *drawable)
{
	cairo_reset_clip (cr);
	if (GDK_DRAWABLE_GET_CLASS (drawable)->set_cairo_clip)
		GDK_DRAWABLE_GET_CLASS (drawable)->set_cairo_clip (drawable, cr);
}
#endif

GR_UnixCairoGraphicsBase::~GR_UnixCairoGraphicsBase()
{
}

/*!
 * Create a new image from the Raster rgba byte buffer defined by pBB.
 * The dimensions of iWidth and iHeight are in logical units but the image
 * doesn't scale if the resolution or zoom changes. Instead you must create
 * a new image.
 */
GR_Image* GR_UnixCairoGraphicsBase::createNewImage (const char* pszName,
													const UT_ByteBuf* pBB,
                                                    const std::string& mimetype,
													UT_sint32 iWidth,
													UT_sint32 iHeight,
													GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;

	if (iType == GR_Image::GRT_Raster) {
		pImg = new GR_UnixImage(pszName);
		pImg->convertFromBuffer(pBB, mimetype, tdu(iWidth), tdu(iHeight));
	} else if (iType == GR_Image::GRT_Vector) {
		pImg = new GR_RSVGVectorImage(pszName);
		pImg->convertFromBuffer(pBB, mimetype, tdu(iWidth), tdu(iHeight));		
	} else {
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

   	return pImg;
}

GR_UnixCairoGraphicsBase::GR_UnixCairoGraphicsBase()
	: GR_CairoGraphics()
{
}

GR_UnixCairoGraphicsBase::GR_UnixCairoGraphicsBase(cairo_t *cr, UT_uint32 iDeviceResolution)
	: GR_CairoGraphics(cr, iDeviceResolution)
{
}

GR_UnixCairoGraphics::GR_UnixCairoGraphics(GdkDrawable * win)
	: GR_UnixCairoGraphicsBase()
	, m_pWin(win)
{
	if (_getDrawable())
	{
		m_cr = gdk_cairo_create (GDK_DRAWABLE (m_pWin));
		_initCairo();
		// Set GraphicsExposes so that XCopyArea() causes an expose on
		// obscured regions rather than just tiling in the default background.
		// TODO: is this still needed with cairo, and if yes can it be emulated
		// without having m_pGC any more?
		// gdk_gc_set_exposures(m_pGC, 1);
		setCursor(GR_CURSOR_DEFAULT);	
	}
}

GR_UnixCairoGraphics::~GR_UnixCairoGraphics()
{
	UT_VECTOR_SPARSEPURGEALL( UT_Rect*, m_vSaveRect);

	// purge saved pixbufs (SPARSE)
	for (UT_sint32 i = 0; i < m_vSaveRectBuf.size (); i++)
	{
		GdkPixbuf * pix = m_vSaveRectBuf.getNthItem(i);
		if(pix) {
			g_object_unref (G_OBJECT (pix));
		}
	}
}


GR_Graphics *   GR_UnixCairoGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_UNIX, NULL);
	xxx_UT_DEBUGMSG(("GR_CairoGraphics::graphicsAllocator\n"));

	UT_return_val_if_fail(!info.isPrinterGraphics(), NULL);
	GR_UnixCairoAllocInfo &AI = (GR_UnixCairoAllocInfo&)info;

	return new GR_UnixCairoGraphics(AI.m_win);
}

inline UT_RGBColor _convertGdkColor(const GdkColor &c)
{
	UT_RGBColor color;
	color.m_red = c.red >> 8;
	color.m_grn = c.green >> 8;
	color.m_blu = c.blue >> 8;
	return color;
}

void GR_UnixCairoGraphics::widget_size_allocate(GtkWidget* /*widget*/, GtkAllocation* /*allocation*/, GR_UnixCairoGraphics* me)
{
#if GTK_CHECK_VERSION(2,17,11) || GTK_2_17_DEV_VERSION_WITHOUT_RESET_CLIP
	UT_return_if_fail(me);
	me->m_clipRectDirty = TRUE;
#else
	UT_UNUSED(me);
#endif
}

void GR_UnixCairoGraphics::initWidget(GtkWidget* widget)
{
#if GTK_CHECK_VERSION(2,17,11) || GTK_2_17_DEV_VERSION_WITHOUT_RESET_CLIP
	UT_return_if_fail(widget);
	g_signal_connect_after(G_OBJECT(widget), "size_allocate", G_CALLBACK(widget_size_allocate), this);
#else
	UT_UNUSED(widget);
#endif
}

void GR_UnixCairoGraphics::init3dColors(GtkStyle * pStyle)
{
	m_3dColors[CLR3D_Foreground] = _convertGdkColor(pStyle->text[GTK_STATE_NORMAL]);
	m_3dColors[CLR3D_Background] = _convertGdkColor(pStyle->bg[GTK_STATE_NORMAL]);
	m_3dColors[CLR3D_BevelUp]    = _convertGdkColor(pStyle->light[GTK_STATE_NORMAL]);
	m_3dColors[CLR3D_BevelDown]  = _convertGdkColor(pStyle->dark[GTK_STATE_NORMAL]);
	m_3dColors[CLR3D_Highlight]  = _convertGdkColor(pStyle->bg[GTK_STATE_PRELIGHT]);

	m_bHave3DColors = true;
}

GR_Font * GR_UnixCairoGraphics::getGUIFont(void)
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
		
		m_pPFontGUI = new GR_PangoFont(guiFontName, 11.0, this, s.utf8_str(), true);

		g_object_unref(G_OBJECT(tempStyle));
		
		UT_ASSERT(m_pPFontGUI);
	}

	return m_pPFontGUI;
}


void GR_UnixCairoGraphics::setCursor(GR_Graphics::Cursor c)
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


void GR_UnixCairoGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
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

void GR_UnixCairoGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	GdkGC *gc;

	GR_Painter caretDisablerPainter(this); // not an elegant way to disable all carets, but it works beautifully - MARCM

	gc = gdk_gc_new(_getDrawable());
   	gdk_draw_drawable(_getDrawable(), gc, _getDrawable(), tdu(x_src), tdu(y_src),
					  tdu(x_dest), tdu(y_dest), tdu(width), tdu(height));
	g_object_unref(G_OBJECT(gc)), gc = NULL;
}

void GR_UnixCairoGraphics::createPixmapFromXPM( char ** pXPM,GdkPixmap *source,
										   GdkBitmap * mask)
{
	source
		= gdk_pixmap_colormap_create_from_xpm_d(_getDrawable(),NULL,
							&mask, NULL,
							pXPM);
}

void GR_UnixCairoGraphics::_resetClip(void)
{
#if GTK_CHECK_VERSION(2,17,11) || GTK_2_17_DEV_VERSION_WITHOUT_RESET_CLIP
	gdk_cairo_reset_clip (m_cr, _getDrawable());
#else
	cairo_reset_clip (m_cr);
#endif
}

void GR_UnixCairoGraphics::saveRectangle(UT_Rect & r, UT_uint32 iIndx)
{
	UT_Rect* oldR = NULL;	
	cairo_save(m_cr);
	cairo_reset_clip(m_cr);
	m_vSaveRect.setNthItem(iIndx, new UT_Rect(r),&oldR);
	if(oldR) {
		delete oldR;
	}

	GdkPixbuf * oldC = NULL;
	UT_sint32 idx = _tduX(r.left);
	UT_sint32 idy = _tduY(r.top);
	UT_sint32 idw = _tduR(r.width);
	UT_sint32 idh = _tduR(r.height);
	cairo_surface_flush ( cairo_get_target(m_cr));

	GdkPixbuf * pix = gdk_pixbuf_get_from_drawable(NULL,
												   _getDrawable(),
												   NULL,
												   idx, idy, 0, 0,
												   idw, idh);
	m_vSaveRectBuf.setNthItem(iIndx, pix, &oldC);

	if(oldC)
		g_object_unref (G_OBJECT (oldC));
	cairo_restore(m_cr);
}	

void GR_UnixCairoGraphics::restoreRectangle(UT_uint32 iIndx)
{
	cairo_save(m_cr);
	cairo_reset_clip(m_cr);
	UT_Rect * r = m_vSaveRect.getNthItem(iIndx);
	GdkPixbuf *p = m_vSaveRectBuf.getNthItem(iIndx);
	UT_sint32 idx = _tduX(r->left);
	UT_sint32 idy = _tduY(r->top);
	cairo_surface_flush ( cairo_get_target(m_cr));

	if (p && r)
		gdk_draw_pixbuf (_getDrawable(), NULL, p, 0, 0,
						 idx, idy,
						 -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
	cairo_restore(m_cr);
}

/*!
 * Take a screenshot of the graphics and convert it to an image.
 */
GR_Image * GR_UnixCairoGraphics::genImageFromRectangle(const UT_Rect &rec)
{
	UT_sint32 idx = _tduX(rec.left);
	UT_sint32 idy = _tduY(rec.top);
	UT_sint32 idw = _tduR(rec.width);
	UT_sint32 idh = _tduR(rec.height);
	UT_return_val_if_fail (idw > 0 && idh > 0 && idx >= 0, NULL);
	cairo_surface_flush ( cairo_get_target(m_cr));
	GdkColormap* cmp = gdk_colormap_get_system();
	GdkPixbuf * pix = gdk_pixbuf_get_from_drawable(NULL,
												   _getDrawable(),
												   cmp,
												   idx, idy, 0, 0,
												   idw, idh);
	
	UT_return_val_if_fail(pix, NULL);

	GR_UnixImage * pImg = new GR_UnixImage("ScreenShot");
	pImg->setData(pix);
	pImg->setDisplaySize(idw,idh);
	return pImg;
}


cairo_t *GR_UnixCairoAllocInfo::createCairo()
{
	if(m_win) {
		return gdk_cairo_create (GDK_DRAWABLE (m_win));
	}
	return NULL;
}

