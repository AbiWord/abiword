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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ut_bytebuf.h"

#include "gr_UnixCairoGraphics.h"
#include "gr_CairoImage.h"
#include "gr_Painter.h"
#include "gr_UnixImage.h"

#include "xap_App.h"
#include "xap_EncodingManager.h"

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

GR_UnixCairoGraphics::GR_UnixCairoGraphics(GdkWindow * win, bool double_buffered)
	: GR_UnixCairoGraphicsBase(),
	  m_pWin(win),
	  m_double_buffered(double_buffered),
	  m_CairoCreated(false),
	  m_Painting(false),
	  m_Signal(0),
 	  m_DestroySignal(0),
     m_Widget(NULL)
{
	m_cr = NULL;
	if (_getWindow())
	{
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
	if (m_Widget) {
		g_signal_handler_disconnect (m_Widget, m_Signal);
		g_signal_handler_disconnect (m_Widget, m_DestroySignal);
	}
}


GR_Graphics *   GR_UnixCairoGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_UNIX, NULL);
	xxx_UT_DEBUGMSG(("GR_CairoGraphics::graphicsAllocator\n"));

//	UT_return_val_if_fail(!info.isPrinterGraphics(), NULL);
	GR_UnixCairoAllocInfo &AI = (GR_UnixCairoAllocInfo&)info;

	return new GR_UnixCairoGraphics(AI.m_win, AI.m_double_buffered);
}

inline UT_RGBColor _convertGdkRGBA(const GdkRGBA &c)
{
	UT_RGBColor color;
	color.m_red = c.red * 255;
	color.m_grn = c.green * 255;
	color.m_blu = c.blue * 255;
	return color;
}

void GR_UnixCairoGraphics::widget_size_allocate(GtkWidget* /*widget*/, GtkAllocation* /*allocation*/, GR_UnixCairoGraphics* me)
{
	UT_return_if_fail(me);
	me->m_clipRectDirty = TRUE;
}

void GR_UnixCairoGraphics::widget_destroy(GtkWidget* widget, GR_UnixCairoGraphics* me)
{
	UT_return_if_fail(me && me->m_Widget == widget);
	me->m_Widget = NULL;
	me->m_Signal = 0;
	me->m_DestroySignal = 0;
}

void GR_UnixCairoGraphics::initWidget(GtkWidget* widget)
{
	UT_return_if_fail(widget && m_Widget == NULL);
	m_Widget = widget;
	m_Signal = g_signal_connect_after(G_OBJECT(widget), "size_allocate", G_CALLBACK(widget_size_allocate), this);
	m_DestroySignal = g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(widget_destroy), this);
}

#define COLOR_MIX 0.67   //COLOR_MIX should be between 0 and 1
#define SQUARE(A) (A)*(A)
void GR_UnixCairoGraphics::init3dColors(GtkWidget* /*w*/)
{
	GtkStyleContext* pCtxt = gtk_style_context_new();
	GtkWidgetPath *path = gtk_widget_path_new();
	gtk_widget_path_append_type(path, GTK_TYPE_TEXT_VIEW);
	gtk_style_context_set_path(pCtxt, path);
	gtk_widget_path_free(path);

	GdkRGBA rgba1, rgba2, rgba_;
	bool bUseThemeColors = true;

	gtk_style_context_save(pCtxt);
	gtk_style_context_add_class(pCtxt, GTK_STYLE_CLASS_BUTTON);

	gtk_style_context_get_color (pCtxt, GTK_STATE_FLAG_NORMAL, &rgba1);
	gtk_style_context_get_background_color (pCtxt, GTK_STATE_FLAG_NORMAL, &rgba2);
	if ((SQUARE(rgba1.red-rgba2.red) + SQUARE(rgba1.green-rgba2.green) + SQUARE(rgba1.blue - rgba2.blue)) < 0.01)
	{
		// There is very little contrast between the bg and fg colors
		// Use default values (black for fg and white for bg)
		// This happens for unknown reasons with some themes
		UT_DEBUGMSG(("No contrast between the theme foreground and background colors! Use black and white instead.\n"));
		bUseThemeColors = false;
		rgba1.red = 0.;
		rgba1.green = 0.;
		rgba1.blue = 0.;
		rgba2.red = 1.;
		rgba2.green = 1.;
		rgba2.blue = 1.;
	}
	m_3dColors[CLR3D_Foreground] = _convertGdkRGBA(rgba1);
	m_3dColors[CLR3D_Background] = _convertGdkRGBA(rgba2);
	rgba_.alpha = 1.;   // we don't really care, abiword does not use transparency
	rgba_.red = rgba1.red*COLOR_MIX + rgba2.red*(1.-COLOR_MIX);
	rgba_.green = rgba1.green*COLOR_MIX + rgba2.green*(1.-COLOR_MIX);
	rgba_.blue = rgba1.blue*COLOR_MIX + rgba2.blue*(1.-COLOR_MIX);
	m_3dColors[CLR3D_BevelUp]    = _convertGdkRGBA(rgba_);

	rgba_.red = rgba1.red*(1.-COLOR_MIX) + rgba2.red*COLOR_MIX;
	rgba_.green = rgba1.green*(1.-COLOR_MIX) + rgba2.green*COLOR_MIX;
	rgba_.blue = rgba1.blue*(1.-COLOR_MIX) + rgba2.blue*COLOR_MIX;
	m_3dColors[CLR3D_BevelDown]  = _convertGdkRGBA(rgba_);

	if (bUseThemeColors)
	{
		gtk_style_context_get_background_color (pCtxt, GTK_STATE_FLAG_PRELIGHT, &rgba2);
	}
	gtk_style_context_restore(pCtxt);
	m_3dColors[CLR3D_Highlight]	= _convertGdkRGBA(rgba2);
	m_bHave3DColors = true;

	g_object_unref(pCtxt);
}
#undef COLOR_MIX
#undef SQUARE

GR_Font * GR_UnixCairoGraphics::getGUIFont(void)
{
	if (!m_pPFontGUI)
	{
		// get the font resource
		GtkStyleContext *tempCtxt = gtk_style_context_new();
		GtkWidgetPath *path = gtk_widget_path_new();
		gtk_widget_path_append_type (path, GTK_TYPE_WINDOW);
		gtk_style_context_set_path(tempCtxt, path);
		gtk_widget_path_free(path);
		PangoFontDescription* fontDesc;
		gtk_style_context_get(tempCtxt, GTK_STATE_FLAG_NORMAL, "font", &fontDesc, NULL);
		const char *guiFontName = pango_font_description_get_family(fontDesc);

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

		pango_font_description_free(fontDesc);
		g_object_unref(G_OBJECT(tempCtxt));

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
	g_object_unref(cursor);
}


void GR_UnixCairoGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
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

	disableAllCarets();

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
	enableAllCarets();
}

void GR_UnixCairoGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  G_GNUC_UNUSED UT_sint32 width, G_GNUC_UNUSED UT_sint32 height)
{
	scroll(x_src - x_dest, y_src - y_dest);
}

void GR_UnixCairoGraphics::_resetClip(void)
{
	
	cairo_reset_clip (m_cr);
	xxx_UT_DEBUGMSG(("Reset clip in gtk cairo \n"));
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
	GdkPixbuf * pix = gdk_pixbuf_get_from_window(getWindow(),
	                                             idx, idy,
	                                             idw, idh);
	UT_return_val_if_fail(pix, NULL);

	GR_UnixImage * pImg = new GR_UnixImage("ScreenShot");
	pImg->setData(pix);
	pImg->setDisplaySize(idw,idh);
	return pImg;
}

void GR_UnixCairoGraphics::_beginPaint()
{
	UT_ASSERT(m_Painting == false);
	GR_CairoGraphics::_beginPaint();

	if (m_cr == NULL)
	{
		UT_ASSERT(m_pWin);
		m_cr = gdk_cairo_create (m_pWin);
		m_CairoCreated = true;
	}

#ifndef NDEBUG
	/* should only be called inside an expose event, messes up
	 * double-buffering and all sorts of other GTK assumptions otherwise
	 * we make this extra effort here to track down old wrong code
	 */
	/* for the time being, ignore it for non-double-buffered widgets that
	 * might be very hard to migrate */
	if (m_double_buffered)
	{
		GdkEvent *ev = gtk_get_current_event();
		UT_ASSERT(ev);
		if (ev)
		{
			UT_ASSERT(ev->type == GDK_EXPOSE || ev->type == GDK_DAMAGE);
			if (ev->type == GDK_EXPOSE || ev->type == GDK_DAMAGE)
				UT_ASSERT(ev->expose.window == m_pWin || ev->expose.window == gdk_window_get_effective_parent (m_pWin));
		}
	}
#endif

	UT_ASSERT(m_cr);
	m_Painting = true;
	_initCairo();
}

void GR_UnixCairoGraphics::_endPaint()
{
	if (m_CairoCreated)
		cairo_destroy (m_cr);
	m_cr = NULL;

	m_Painting = false;
	m_CairoCreated = false;

	GR_CairoGraphics::_endPaint();
}

bool GR_UnixCairoGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
		case DGP_SCREEN:
		case DGP_OPAQUEOVERLAY:
			return m_pWin != NULL;
		case DGP_PAPER:
			return false;
		default:
			UT_ASSERT(0);
			return false;
	}
}

