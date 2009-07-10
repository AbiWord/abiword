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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gr_Painter.h"
#include "gr_Graphics.h"

GR_Painter::GR_Painter (GR_Graphics * pGr)
	: m_pGr (pGr),
		m_pCaretDisabler(NULL)
{
	UT_ASSERT (m_pGr);
	m_pCaretDisabler = new GR_CaretDisabler(m_pGr->getCaret());
	UT_sint32 i = 0;
	GR_Caret * pCaret = pGr->getNthCaret(i);
	while(pCaret)
	{
	    GR_CaretDisabler * pCaretDisabler = new GR_CaretDisabler(pCaret);
	    m_vecDisablers.addItem(pCaretDisabler);
	    i++;
	    pCaret = pGr->getNthCaret(i);
	}
	m_pGr->beginPaint ();
}


GR_Painter::GR_Painter (GR_Graphics * pGr, bool bCaret)
	: m_pGr (pGr),
		m_pCaretDisabler(NULL)
{
	UT_ASSERT (m_pGr);
	if(bCaret)
	  m_pCaretDisabler = NULL;
	m_pGr->beginPaint ();
}

GR_Painter::~GR_Painter ()
{
	m_pGr->endPaint ();
	DELETEP(m_pCaretDisabler);
	UT_VECTOR_PURGEALL(GR_CaretDisabler *, m_vecDisablers);
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

void GR_Painter::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	m_pGr->drawImage (pImg, xDest, yDest);
}

void GR_Painter::fillRect(const UT_RGBColor& c, const UT_Rect &r)
{
	m_pGr->fillRect (c, r);
}

void GR_Painter::polygon(UT_RGBColor& c, UT_Point *pts, UT_uint32 nPoints)
{
	m_pGr->polygon (c, pts, nPoints);
}

void GR_Painter::polyLine(UT_Point * pts, UT_uint32 nPoints)
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


void GR_Painter::fillRect(GR_Graphics::GR_Color3D c, UT_Rect &r)
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


GR_Image * GR_Painter::genImageFromRectangle(const UT_Rect & r)
{
	return m_pGr->genImageFromRectangle(r);
}
