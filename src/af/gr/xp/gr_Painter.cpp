/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz <cinamod@hotmail.com>
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

#include "gr_Painter.h"
#include "gr_Graphics.h"

GR_Painter::GR_Painter (GR_Graphics * pGr)
	: m_pGr (pGr)
{
	UT_ASSERT (m_pGr);
	m_pGr->beginPaint ();
}

GR_Painter::~GR_Painter ()
{
	m_pGr->endPaint ();
}

#if 0

void GR_Painter::drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff)
{
	m_pGr->drawGlyph (glyph_idx, xoff, yoff);
}

void GR_Painter::drawChars(const UT_UCSChar* pChars,
						   int iCharOffset,
						   int iLength,
						   UT_sint32 xoff,
						   UT_sint32 yoff,
						   int* pCharWidths = NULL)
{
	m_pGr->drawChars (pChars, iCharOffset, iLength, xoff, yoff, pCharWidths);
}

void GR_Painter::setFont(GR_Font* pFont)
{
	m_pGr->setFont (pFont);
}

void GR_Painter::clearFont(void)
{
	m_pGr->clearFont ();
}

void GR_Painter::setColor(const UT_RGBColor& clr)
{
	m_pGr->setColor (clr);
}

void GR_Painter::getColor(UT_RGBColor& clr)
{
	m_pGr->getColor (clr);
}

void GR_Painter::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	m_pGr->drawImage (pImg, xDest, yDest);
}

void GR_Painter::drawLine(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	m_pGr->drawLine (x, y, w, h);
}

void GR_Painter::xorLine(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	m_pGr->xorLine (x, y, w, h);
}

void GR_Painter::setLineWidth(UT_sint32 w)
{
	m_pGr->setLineWidth (w);
}

void polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	m_pGr->polyLine (ptx, nPoints);
}

void GR_Painter::fillRect(const UT_RGBColor& c,
						  UT_sint32 x,
						  UT_sint32 y,
						  UT_sint32 w,
						  UT_sint32 h)
{
	m_pGr->fillRect (c, x, y, w, h);
}

void GR_Painter::fillRect(GR_Image *pImg, const UT_Rect &src, const UT_Rect & dest)
{
	m_pGr->fillRect (pImg, src, dest);
}

void GR_Painter::fillRect(const UT_RGBColor& c, const UT_Rect &r)
{
	m_pGr->fillRect (c, r);
}

void GR_Painter::invertRect(const UT_Rect* pRect)
{
	m_pGr->invertRect (pRect);
}

void GR_Painter::setClipRect(const UT_Rect* pRect)
{
	m_pGr->setClipRect (pRect);
}

void GR_Painter::clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	m_pGr->clearArea (x, y, w, h);
}

void GR_Painter::setLineProperties (double inWidthPixels,
									GR_Graphics::JoinStyle inJoinStyle = GR_Graphics::JOIN_MITER,
									GR_Graphics::CapStyle inCapStyle   = GR_Graphics::CAP_BUTT,
									GR_Graphics::LineStyle inLineStyle = GR_Graphics::LINE_SOLID)
{
	m_pGr->setLineProperties (inWidthPixels, inJoinStyle, inCapStyle, inLineStyle);
}

void GR_Painter::flush(void)
{
	m_pGr->flush ();
}

void GR_Painter::setColor3D(GR_Color3D c)
{
	m_pGr->setColor3D ();
}

void GR_Painter::fillRect(GR_Color3D c,
						  UT_sint32 x,
						  UT_sint32 y,
						  UT_sint32 w,
						  UT_sint32 h)
{
	m_pGr->fillRect (c, x, y, w, h);
}

void GR_Painter::fillRect(GR_Color3D c, UT_Rect &r)
{
	m_pGr->fillRect (c, r);
}

void GR_Painter::polygon(UT_RGBColor& c, UT_Point *pts, UT_uint32 nPoints)
{
	m_pGr->polygone (c, pts, nPoints);
}

#endif
