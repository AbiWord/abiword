/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"
#include "gr_Graphics.h"
#include "gr_CharWidths.h"
#include "gr_Painter.h"

#include "xap_Draw_Symbol.h"


static UT_UCSChar MaxWidthChar = 0;
static UT_UCSChar MaxHeightChar = 0;

XAP_Draw_Symbol::XAP_Draw_Symbol(GR_Graphics * gc)
	: XAP_Preview(gc),
	  m_areagc(NULL),
	  m_pFont(NULL),
	  m_drawWidth(0),
	  m_drawHeight(0),
	  m_drawareaWidth(0),
	  m_drawareaHeight(0),
	  m_start_base(0),
	  m_start_nb_char(0),
	  m_CurrentSymbol(UCS_SPACE),
	  m_PreviousSymbol(UCS_SPACE),
	  m_vCharSet(256),
	  m_stFont("Symbol")
{
}

XAP_Draw_Symbol::~XAP_Draw_Symbol(void)
{
}

void XAP_Draw_Symbol::setWindowSize( UT_uint32 width, UT_uint32 height)
{
	m_drawWidth = m_gc->tlu(width);
	m_drawHeight = m_gc->tlu(height);
}

void XAP_Draw_Symbol::setAreaGc( GR_Graphics * gc)
{
	m_areagc = gc;   
}

void XAP_Draw_Symbol::setAreaSize( UT_uint32 width, UT_uint32 height)
{
	m_drawareaWidth = m_areagc->tlu(width);
	m_drawareaHeight = m_areagc->tlu(height);
	setFontStringarea();
}

void XAP_Draw_Symbol::setFontString( void )
{
	setFontToGC(m_gc, m_drawWidth / 32, m_drawHeight / 7);	
}

void XAP_Draw_Symbol::setFontToGC(GR_Graphics *p_gc, UT_uint32 MaxWidthAllowable, UT_uint32 MaxHeightAllowable)
{
	UT_ASSERT(p_gc);
	UT_ASSERT(MaxWidthAllowable);
	UT_ASSERT(MaxHeightAllowable);

	GR_Font* font = NULL;


	// Estimate Pointsize
	UT_sint32 PointSize = 32;
	UT_sint32 LastPointSize = -1;
	UT_sint32 UpperPointSize = -1;
	UT_sint32 LowerPointSize = 1;

	// We try to find the Pointsize which is a bit smaller than
	// MaxHeightAllowable and MaxWidthAllowable (W+HAllowable)
	// we archieve that in two steps:
	// 1. step: find a Pointsize which is bigger as W+HAllowable)
	// 2. step: Iterate by taking halfe PointSize steps until we find
	// the fitting one
	while (1)
	{
		char temp[16];

		snprintf(temp, 16, "%ipt", PointSize);
		font = p_gc->findFont(m_stFont.c_str(),
							  "normal", "",
							  "normal", "",
							  temp,
							  NULL);
		/* findFont does a fuzzy match.  If the font found doesn't
		 * have the same family name that we asked for, we retrieve
		 * the new name and we use it
		 */
		if (font->getFamily())
			m_stFont = font->getFamily();
		
		p_gc->setFont(font);

		p_gc->getCoverage(m_vCharSet);

		if (PointSize == LastPointSize)
			break;
		
		if (!MaxWidthChar)
		{
			UT_uint32 MaxWidth = 0;
			UT_uint32 MaxHeight = 0;

                        // this can take very long with large fonts
		 	for (UT_sint32 i = m_start_base; i < m_vCharSet.size(); i += 2)
			{
				UT_UCSChar base = static_cast<UT_UCSChar>(m_vCharSet[i]);
				UT_sint32 nb_chars = m_vCharSet[i + 1];

				for (UT_sint32 j = m_start_base == i? m_start_nb_char: 0; j < nb_chars; j++)
				{
					UT_UCSChar c = base + j;
				        UT_uint32 width, height;
		   			p_gc->getMaxCharacterDimension(&c, 1, width, height);
					if (width > MaxWidth) 
					{
						MaxWidth = width;
						MaxWidthChar = c;
					}
						
					if (height > MaxHeight) 
					{
						MaxHeight = height;
						MaxHeightChar = c;
					}
				}
			}
	        }


	        UT_uint32 width, height;
		p_gc->getMaxCharacterDimension(&MaxWidthChar, 1, width, height);
		UT_sint32 xdiff_temp = MaxWidthAllowable - width;
		p_gc->getMaxCharacterDimension(&MaxHeightChar, 1, width, height);
		UT_sint32 ydiff_temp = MaxHeightAllowable - height;

		if (UpperPointSize < 0)
		{
			if (xdiff_temp < 0 || ydiff_temp < 0)
				UpperPointSize = PointSize;
			else if (PointSize > 72)
				UpperPointSize = PointSize = 72;
			else
				PointSize *= 2;
		}
		if (UpperPointSize > 0)
		{	LastPointSize = PointSize;
		  	if (xdiff_temp < 0 || ydiff_temp < 0)
				UpperPointSize = PointSize;
			else
				LowerPointSize = PointSize;

			PointSize = LowerPointSize + (UpperPointSize - LowerPointSize) / 2;
		}
		if (PointSize < 10)
		{
			PointSize = 10; // don't allow a too small font, see #13723
	 	}

	}

}

const char* XAP_Draw_Symbol::getSelectedFont() const
{
	return m_stFont.c_str();
}

void XAP_Draw_Symbol::setSelectedFont(const char *font)
{
	m_stFont = font;
	m_CurrentSymbol = m_PreviousSymbol = UCS_SPACE;
	m_start_base = 0;
	m_start_nb_char = 0;
	MaxWidthChar = 0;
	MaxHeightChar = 0;

	setFontString();
	setFontStringarea();
}

void XAP_Draw_Symbol::setFontStringarea(void)
{
	setFontToGC(m_areagc, m_drawareaWidth, m_drawareaHeight);
}

void XAP_Draw_Symbol::setRow (UT_uint32 row)
{
        UT_uint32 chars_needed = row * 32;
	UT_uint32 chars = 0;
	for (UT_sint32 i = 0; i < m_vCharSet.size(); i += 2)
	{
		UT_sint32 numb = m_vCharSet[i + 1];
		if (chars + numb > chars_needed)
		{
			m_start_base = i;
			m_start_nb_char = chars_needed - chars;
			break;
		}
		chars += numb;
	}
	queueDraw();
}

UT_uint32 XAP_Draw_Symbol::getSymbolRows (void) const
{
	UT_uint32 chars = 0;
	for (UT_sint32 i = m_start_base; i < m_vCharSet.size(); i += 2)
	{
		chars += m_vCharSet[i + 1];
	}

	UT_uint32 rows = chars / 32 + (chars % 32? 1: 0);
	return rows;
}

void XAP_Draw_Symbol::drawImmediate(const UT_Rect *clip)
{
	UT_UNUSED(clip);
	UT_ASSERT(m_gc);
	UT_uint32 wwidth, wheight, /*yoff, xoff,*/ x, y;
	UT_sint32 i;

	GR_Painter painter(m_gc);
	
	wwidth = m_drawWidth;
	wheight = m_drawHeight;
	UT_uint32 tmpw = wwidth / 32;
	UT_uint32 tmph = wheight / 7;
	//	yoff = wheight / (2 * 7);
	//	xoff = wwidth / (2 * 32);
	painter.clearArea(0, 0, wwidth, wheight);
	int pos = 0;

	for (i = m_start_base; i < m_vCharSet.size(); i += 2)
	{
		UT_UCSChar base = static_cast<UT_UCSChar>(m_vCharSet[i]);
		UT_sint32 nb_chars = m_vCharSet[i + 1];

		for (UT_UCSChar j = base + (m_start_base == i? m_start_nb_char: 0); j < base + nb_chars; ++j)
		{
			UT_sint32 w = m_gc->measureUnRemappedChar(j);

			if(w != GR_CW_ABSENT)
			{
				if ((unsigned) w > tmpw)
					w = tmpw;
				x = (pos % 32) * tmpw + (tmpw - w) / 2;
				y = pos / 32 * tmph;
			
				painter.drawChars(&j, 0, 1, x, y);
			}
			
			++pos;
			
			if(pos > 32 * 7)
				break;
		}
		
		if(pos > 32 * 7)
			break;
	}

	y = 0;
	for(i = 0; i <= 7; i++)
	{
		painter.drawLine(0, y, wwidth - m_areagc->tlu(1), y);
		y += tmph;
	}

	x = 0;
	for(i = 0; i <= 32; i++)
	{
		painter.drawLine(x, 0, x, wheight - m_areagc->tlu(1));
		x += tmpw;
	}
}

UT_UCSChar XAP_Draw_Symbol::calcSymbolFromCoords(UT_uint32 ix, UT_uint32 iy)
{
	UT_sint32 index,count;
	index = iy * 32 + ix;
	count = 0;
	UT_DEBUGMSG(("calcSymbolFromCoords(x = [%u], y = [%u]) =", ix, iy));

	for (UT_sint32 i = m_start_base; i < m_vCharSet.size(); i += 2)
	{
		count = m_vCharSet[i + 1];
		if (i == m_start_base &&  count > m_start_nb_char)
			index += m_start_nb_char;
		if (count > index)
		{
			UT_DEBUGMSG((" %u\n", static_cast<UT_uint32>(m_vCharSet[i] + index - count + m_vCharSet[i + 1])));
			return static_cast<UT_UCSChar>(m_vCharSet[i] + index);
		}
		index -= count;
	}

	return static_cast<UT_UCSChar>(0);
}

UT_UCSChar XAP_Draw_Symbol::calcSymbol(UT_uint32 x, UT_uint32 y)
{
	UT_uint32 width = m_drawWidth;
	UT_uint32 height = m_drawHeight;
	UT_uint32 ix;
	UT_uint32 iy;
	
	if (x > width || y > height)
		return static_cast<UT_UCSChar>(0);

	iy = m_gc->tlu(y) / (height / 7);
	ix = m_gc->tlu(x) / (width / 32);
	return calcSymbolFromCoords(ix, iy);
}

void XAP_Draw_Symbol::calculatePosition(UT_UCSChar c, UT_uint32 &x, UT_uint32 &y)
{
	UT_uint32 index = 0;

	for (UT_sint32 i = m_start_base; i < m_vCharSet.size(); i += 2)
	{
		UT_uint32 base = static_cast<UT_uint32>(m_vCharSet[i]);
		UT_uint32 size =  static_cast<UT_uint32>(m_vCharSet[i + 1]);
		
		if (base + size > c)
		{
			index += c - (i == m_start_base? base + m_start_nb_char: base);
			break;
		}
		else
			index += size - (m_start_base == i? m_start_nb_char: 0);
	}

	x = index % 32;
	y = index / 32;
	UT_DEBUGMSG(("[%d] -> (%d, %d)\n", c, x, y));
}

void XAP_Draw_Symbol::setCurrent(UT_UCSChar c)
{
	m_PreviousSymbol = m_CurrentSymbol;
	m_CurrentSymbol = c;
	drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

void XAP_Draw_Symbol::drawarea(UT_UCSChar c, UT_UCSChar p)
	//
	// This function displays the symbol c into the Selected Area.
	// It also highlights the selected symbol in the Symbol Table.
	//
{
	UT_ASSERT(m_areagc);
	UT_uint32 wwidth,wheight,x,y,cx,cy,px,py /*,swidth,sheight*/;
	UT_uint32 /*cx1,cy1,*/px1,py1;

	GR_Painter areaPainter(m_areagc);
	GR_Painter painter(m_gc);

	wwidth = m_drawareaWidth;
	wheight = m_drawareaHeight;

	// Center the character
	// Note: That's bogus.  measureString will give us the horizontal advance of "c",
	// but we need the bounding box of "c" instead.  To get it, we should use with FreeType face->bbox,
	// in windows we should use the (FIXME: find the right name, it was something as getCharABC(...)
	UT_uint32 h1 = 0;
	UT_sint32 w1 = m_areagc->measureUnRemappedChar(c, &h1);

	areaPainter.clearArea(0, 0, wwidth, wheight);

	if(w1 != GR_CW_ABSENT)
	{
		x = (m_drawareaWidth > (unsigned) w1)? (m_drawareaWidth - w1) / 2: 0;
		y = (m_drawareaHeight - h1) / 2;
		areaPainter.drawChars(&c, 0, 1, x, y);
	}
			
	//
	// Calculate the cordinates of the current and previous symbol
	// along with the widths of the appropriate boxes.
	//	swidth = m_drawWidth;
	//	sheight = m_drawHeight;
	UT_uint32 tmpw = m_drawWidth / 32;
	UT_uint32 tmph = m_drawHeight / 7;

	calculatePosition(c, cx, cy);
	UT_sint32 wc = m_gc->measureUnRemappedChar(c);

	cx *= tmpw;
	cy *= tmph;
	
	//	cx1 = cx + tmpw;
	//	cy1 = cy + tmph;

	calculatePosition(p, px, py);
	UT_sint32 wp = m_gc->measureUnRemappedChar(p);

	px *= tmpw;
	py *= tmph;
	
	px1 = px + tmpw;
	py1 = py + tmph;

	// Redraw the Previous Character in black on White
	painter.clearArea(px + m_areagc->tlu(1), py + m_areagc->tlu(1), tmpw - m_areagc->tlu(1), tmph - m_areagc->tlu(1));

	if(wp != GR_CW_ABSENT)
	{
		painter.drawChars(&p, 0, 1, px + ((tmpw > (unsigned) wp)? (tmpw - wp) / 2: 0), py);
	}
	
	// Redraw only the white box boundaries
	// you do not notice the missing of the
	// selected box
	painter.drawLine(px, py, px1, py);
	painter.drawLine(px, py1, px1, py1);
	painter.drawLine(px, py, px, py1);
	painter.drawLine(px1, py, px1, py1);

	// Redraw the Current Character in black on Blue
	UT_RGBColor colour(128, 128, 192);
	painter.fillRect(colour, cx + m_areagc->tlu(1), cy + m_areagc->tlu(1), tmpw - m_areagc->tlu(1), tmph - m_areagc->tlu(1));
	if(wc != GR_CW_ABSENT)
	{
		painter.drawChars(&c, 0, 1, cx + ((tmpw > (unsigned) wc)? (tmpw - wc) / 2: 0), cy);
	}
}

void XAP_Draw_Symbol::onLeftButtonDown(UT_sint32 x, UT_sint32 y)
{
	setCurrent(calcSymbol(x, y));
}
