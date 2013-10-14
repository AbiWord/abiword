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


#ifndef __GR_QTGRAPHICS_H__
#define __GR_QTGRAPHICS_H__

#include "gr_CairoGraphics.h"

class ABI_EXPORT GR_QtAllocInfo
	: public GR_AllocInfo
{
public:
	GR_QtAllocInfo(bool bPreview, bool bPrinter, bool double_buffered)
		: m_bPreview(bPreview),
		  m_bPrinter(bPrinter),
		  m_double_buffered(double_buffered)
	{
	}
	virtual GR_GraphicsId getType() const {return GRID_QT;}
	virtual bool isPrinterGraphics() const {return m_bPrinter;}

	bool m_bPreview;
	bool m_bPrinter;
	bool m_double_buffered;
};

class ABI_EXPORT GR_QtGraphics
	: public GR_CairoGraphics
{
public:
	~GR_QtGraphics();
	static UT_uint32        s_getClassId() {return GRID_QT;}
	virtual UT_uint32       getClassId() {return s_getClassId();}
 
	static const char *     graphicsDescriptor(){return "Unix Qt";}
	static GR_Graphics *    graphicsAllocator(GR_AllocInfo&);

	virtual GR_Font*        getGUIFont(void);
	virtual void			scroll(UT_sint32, UT_sint32);
	virtual void			scroll(UT_sint32 x_dest, UT_sint32 y_dest,
								UT_sint32 x_src, UT_sint32 y_src,
								UT_sint32 width, UT_sint32 height);
	virtual void			setCursor(Cursor c);
	virtual GR_Image *      genImageFromRectangle(const UT_Rect & r);

protected:
	GR_QtGraphics();

private:
};

#endif
