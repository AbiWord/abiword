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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gr_Painter.h"
#include "gr_Graphics.h"

GR_Painter::GR_Painter (GR_Graphics * pGr, bool bDisableCarets)
	: m_pGr (pGr),
	m_bCaretsDisabled(bDisableCarets),
	m_bDoubleBufferingToken(false),
	m_bSuspendDrawingToken(false)
{
	UT_ASSERT (m_pGr);

	if (m_pGr) {
		if (m_bCaretsDisabled) {
			m_pGr->disableAllCarets();
		}
		m_pGr->beginPaint ();
	}
}

GR_Painter::~GR_Painter ()
{
	endDoubleBuffering();

	m_pGr->endPaint ();

	if (m_bCaretsDisabled)
		m_pGr->enableAllCarets();
}

void GR_Painter::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	m_pGr->drawLine (x1, y1, x2, y2);
}

#if XAP_DONTUSE_XOR
#else
void GR_Painter::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	m_pGr->xorLine (x1, y1, x2, y2);
}

void GR_Painter::xorRect(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	m_pGr->xorRect (x, y, w, h);
}

void GR_Painter::xorRect(const UT_Rect& r)
{
	m_pGr->xorRect (r);
}
#endif

void GR_Painter::invertRect(const UT_Rect* pRect)
{
	m_pGr->invertRect (pRect);
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

void GR_Painter::clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	m_pGr->clearArea (x, y, w, h);
}

void GR_Painter::fillRect(const UT_RGBColor& c, const UT_Rect &r)
{
	m_pGr->fillRect (c, r);
}

void GR_Painter::fillRect(GR_Graphics::GR_Color3D c, const UT_Rect &r)
{
	m_pGr->fillRect (c, r.left, r.top, r.width, r.height);
}

void GR_Painter::fillRect(GR_Graphics::GR_Color3D c,
						  UT_sint32 x,
						  UT_sint32 y,
						  UT_sint32 w,
						  UT_sint32 h)
{
	m_pGr->fillRect (c, x, y, w, h);
}

void GR_Painter::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	m_pGr->drawImage (pImg, xDest, yDest);
}

void GR_Painter::polygon(const UT_RGBColor& c, const UT_Point *pts, UT_uint32 nPoints)
{
	m_pGr->polygon (c, pts, nPoints);
}

void GR_Painter::polyLine(const UT_Point * pts, UT_uint32 nPoints)
{
	m_pGr->polyLine(pts, nPoints);
}

void GR_Painter::drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff)
{
	m_pGr->drawGlyph (glyph_idx, xoff, yoff);
}

void GR_Painter::drawChars(const UT_UCSChar* pChars,
						   int iCharOffset,
						   int iLength,
						   UT_sint32 xoff,
						   UT_sint32 yoff,
						   int* pCharWidths)
{
	m_pGr->drawChars (pChars, iCharOffset, iLength, xoff, yoff, pCharWidths);
}

void GR_Painter::drawCharsRelativeToBaseline(const UT_UCSChar* pChars,
						   int iCharOffset,
						   int iLength,
						   UT_sint32 xoff,
						   UT_sint32 yoff,
						   int* pCharWidths)
{
	m_pGr->drawCharsRelativeToBaseline (pChars, iCharOffset, iLength, xoff, yoff, pCharWidths);
}

void GR_Painter::renderChars(GR_RenderInfo & ri)
{
	m_pGr->renderChars(ri);
}


GR_Image * GR_Painter::genImageFromRectangle(const UT_Rect & r)
{
	return m_pGr->genImageFromRectangle(r);
}

void GR_Painter::beginDoubleBuffering()
{
	m_bDoubleBufferingToken = m_pGr -> beginDoubleBuffering();
}

void GR_Painter::endDoubleBuffering()
{
	m_pGr -> endDoubleBuffering(m_bDoubleBufferingToken);
	m_bDoubleBufferingToken = false;
}

void GR_Painter::suspendDrawing()
{
	m_bSuspendDrawingToken = m_pGr->suspendDrawing();
}

void GR_Painter::resumeDrawing()
{
	m_pGr->resumeDrawing(m_bSuspendDrawingToken);
	m_bSuspendDrawingToken = false;
}

