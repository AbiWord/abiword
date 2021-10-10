/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 2004-2006 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2009-2021 Hubert Figuière
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

#pragma once

#include "ut_compiler.h"

ABI_W_NO_CONST_QUAL
#include <gdk/gdk.h>
ABI_W_POP
#include "gr_CairoGraphics.h"

class ABI_EXPORT GR_UnixCairoAllocInfo : public GR_CairoAllocInfo
{
public:
	GR_UnixCairoAllocInfo(GtkWidget *widget)
		: GR_CairoAllocInfo(false, false, true),
		m_win(widget)
	{}

	GR_UnixCairoAllocInfo(bool bPreview)
		: GR_CairoAllocInfo(bPreview, true, false),
		  m_win(NULL){}

	cairo_t *createCairo() override {return NULL;} // we need this since otherwise the class would be abstract
	GtkWidget     * m_win;
};

class ABI_EXPORT GR_UnixCairoGraphicsBase
	: public GR_CairoGraphics
{
 public:
	~GR_UnixCairoGraphicsBase();

	virtual GR_Image*	createNewImage(const char* pszName,
									   const UT_ConstByteBufPtr& pBB,
		                               const std::string& mimetype,
									   UT_sint32 iDisplayWidth,
									   UT_sint32 iDisplayHeight,
									   GR_Image::GRType = GR_Image::GRT_Raster) override;
 protected:
	GR_UnixCairoGraphicsBase();
	GR_UnixCairoGraphicsBase(cairo_t *cr, UT_uint32 iDeviceResolution);

};


class ABI_EXPORT GR_UnixCairoGraphics
	: public GR_UnixCairoGraphicsBase
{
public:
	~GR_UnixCairoGraphics();
	static UT_uint32       s_getClassId() {return GRID_UNIX_PANGO;}
	virtual UT_uint32      getClassId() override {return s_getClassId();}

	static const char *    graphicsDescriptor(){return "Unix Cairo Pango";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);
	GdkWindow *  getWindow () {return m_pWin;}

	virtual GR_Font * getGUIFont(void) override;

	virtual void		setCursor(GR_Graphics::Cursor c) override;
	virtual void		scroll(UT_sint32, UT_sint32) override;
	virtual void		scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						   UT_sint32 x_src, UT_sint32 y_src,
						   UT_sint32 width, UT_sint32 height) override;
	virtual GR_Image *  genImageFromRectangle(const UT_Rect & r) override;

	void				init3dColors(GtkWidget* w);
	virtual bool		queryProperties(GR_Graphics::Properties gp) const override;

	/** In the UnixCairoGraphics, color3D are mostly invalid. */
	virtual bool        getColor3D(GR_Color3D name, UT_RGBColor &color) override;

	virtual void		fillRect(GR_Color3D c,
								 UT_sint32 x, UT_sint32 y,
								 UT_sint32 w, UT_sint32 h) override;
	virtual void queueDraw(const UT_Rect* pRect) override;
	virtual void      flush(void) override;

	// Return the cursor name.
	static const char* _getCursor(GR_Graphics::Cursor c);
protected:
	void _initWidget();
	virtual void		_resetClip(void) override;
	static void		widget_size_allocate (GtkWidget        *widget,
									  GtkAllocation    *allocation,
									  GR_UnixCairoGraphics *me);
	static void		widget_destroy (GtkWidget        *widget,
									  GR_UnixCairoGraphics *me);
	GR_UnixCairoGraphics(GtkWidget* win = nullptr);
	virtual GdkWindow * _getWindow(void)
	{  return m_pWin;}

	virtual void _beginPaint() override;
	virtual void _endPaint() override;

private:
	GdkWindow *m_pWin;
	GdkDrawingContext* m_context;
	bool m_CairoCreated;
	bool m_Painting;
	gulong m_Signal, m_DestroySignal;
	GtkWidget *m_Widget;
	GtkStyleContext* m_styleBg;
	GtkStyleContext* m_styleHighlight;
};
