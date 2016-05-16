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

#ifndef GR_PAINTER_H
#define GR_PAINTER_H

#include "xap_Features.h"
#include "gr_Graphics.h"

class ABI_EXPORT GR_Painter
{
public:

	GR_Painter (GR_Graphics * pGr, bool bDisableCarets = true);
	~GR_Painter ();

	void drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2);
#if XAP_DONTUSE_XOR
#else
	void xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2);
	void xorRect(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	void xorRect(const UT_Rect& r);
#endif
	void invertRect(const UT_Rect* pRect);

	void fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
				  UT_sint32 w, UT_sint32 h);
	void fillRect(GR_Image *pImg, const UT_Rect &src, const UT_Rect & dest);
	void fillRect(const UT_RGBColor& c, const UT_Rect &r);
	void fillRect(GR_Graphics::GR_Color3D c,
				  UT_sint32 x,
				  UT_sint32 y,
				  UT_sint32 w,
				  UT_sint32 h);

	void fillRect(GR_Graphics::GR_Color3D c, const UT_Rect &r);

	void clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h);
	void drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest);
	void polygon(const UT_RGBColor& c, const UT_Point *pts, UT_uint32 nPoints);
	void polyLine(const UT_Point * pts, UT_uint32 nPoints);
	void drawGlyph(UT_uint32 glyph_idx, UT_sint32 xoff, UT_sint32 yoff);
	void drawChars(const UT_UCSChar* pChars,
				   int iCharOffset,
				   int iLength,
				   UT_sint32 xoff,
				   UT_sint32 yoff,
				   int* pCharWidths = NULL);

	void drawCharsRelativeToBaseline(const UT_UCSChar* pChars,
				   int iCharOffset,
				   int iLength,
				   UT_sint32 xoff,
				   UT_sint32 yoff,
				   int* pCharWidths = NULL);

	void renderChars(GR_RenderInfo & ri);


	GR_Image * genImageFromRectangle(const UT_Rect & r);

	// These just call the functions with the same name in GR_Graphics.
	void beginDoubleBuffering();
	void endDoubleBuffering();

	void suspendDrawing();
	void resumeDrawing();

private:

	GR_Painter ();
	GR_Painter (const GR_Painter & rhs);
	GR_Painter& operator=(const GR_Painter & rhs);

	GR_Graphics * m_pGr;
	bool m_bCaretsDisabled;

	bool m_bDoubleBufferingToken;
	bool m_bSuspendDrawingToken;
};

#endif // GR_PAINTER_H
