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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"
#include "gr_Graphics.h"

#include "xap_Draw_Symbol.h"

XAP_Draw_Symbol::XAP_Draw_Symbol(GR_Graphics * gc)
	: XAP_Preview(gc),
	  m_areagc(NULL),
	  m_pFont(NULL),
	  m_drawWidth(0),
	  m_drawHeight(0),
	  m_drawareaWidth(0),
	  m_drawareaHeight(0),
	  m_CurrentSymbol(UCS_SPACE),
	  m_PreviousSymbol(UCS_SPACE),
	  m_vCharSet(256),
	  m_stFont("Symbol")
{
}


XAP_Draw_Symbol::~XAP_Draw_Symbol(void)
{
  	DELETEP(m_pFont);
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
	setFontToGC(m_gc, m_drawWidth / 32, 14);	
}

void XAP_Draw_Symbol::setFontToGC(GR_Graphics *p_gc, UT_uint32 MaxWidthAllowable, UT_sint32 PointSize)
{
	UT_ASSERT(p_gc);
	UT_ASSERT(MaxWidthAllowable);

	GR_Font* font = NULL;

	int SizeOK = false;

#ifndef WITH_PANGO	
	UT_UCSChar p_buffer[224];
	for(int i = 0; i < 224; i++)
		p_buffer[i] = i + 32;
#endif

	char temp[10];
	while (!SizeOK)
	{
		sprintf(temp, "%ipt", PointSize);
		font = p_gc->findFont(m_stFont.c_str(), "normal", "", "normal", "", temp);
		/* findFont does a fuzzy match.  If the font found doesn't have the same family name
		 * that we asked for, we retrieve the new name and we use it */
		if (font->getFamily())
			m_stFont = font->getFamily();
		
		p_gc->setFont(font);
		
		UT_uint32 MaxWidth = p_gc->getMaxCharacterWidth(p_buffer, 224);
			
		if (MaxWidth < MaxWidthAllowable)
			SizeOK = true;
		else
		{
			PointSize--;
			UT_ASSERT(PointSize);
		}
	}

	p_gc->getCoverage(m_vCharSet);
}

const char* XAP_Draw_Symbol::getSelectedFont()
{
	return m_stFont.c_str();
}

void XAP_Draw_Symbol::setSelectedFont(const char *font)
{
	m_stFont = font;
	setFontString();
	setFontStringarea();

	m_CurrentSymbol = m_PreviousSymbol = UCS_SPACE;
}

void XAP_Draw_Symbol::setFontStringarea(void)
{
	setFontToGC(m_areagc, m_drawareaWidth, 32);
}


void XAP_Draw_Symbol::draw(void)
{
	UT_ASSERT(m_gc);
	UT_uint32 wwidth, wheight, yoff, xoff, x, y;
	size_t i;
	
	wwidth = m_drawWidth;
	wheight = m_drawHeight;
	UT_uint32 tmpw = wwidth / 32;
	UT_uint32 tmph = wheight / 7;
	yoff = wheight / 14;
	xoff = wwidth / 64;
	m_gc->clearArea(0, 0, wwidth, wheight);
	int pos = 0;
	
	for (i = 0; i < m_vCharSet.size(); i += 2)
	{
		UT_UCSChar base = static_cast<UT_UCSChar>(reinterpret_cast<UT_uint32>(m_vCharSet[i]));
		size_t nb_chars = reinterpret_cast<size_t>(m_vCharSet[i + 1]);

		for (UT_UCSChar j = base; j < base + nb_chars; ++j)
		{
			unsigned short w = m_gc->measureUnRemappedChar(j);
			UT_uint32 x = (pos % 32) * tmpw + (tmpw - w) / 2;
			UT_uint32 y = pos / 32 * tmph;
#ifndef WITH_PANGO
			m_gc->drawChars(&j, 0, 1, x, y);
#else
			PangoGlyphString * pGlyph = m_gc->getPangoGlyphString(c);
			m_gc->drawPangoGlyphString(pGlyph, x, y);
			pango_glyph_string_free(pGlyph);
#endif			
			++pos;
		}
	}

	y = 0;
	for(i = 0; i <= 6; i++)
	{
		m_gc->drawLine(0, y, wwidth - m_areagc->tlu(1), y);
		y += tmph;
	}

	x = 0;
	for(i = 0; i <= 31; i++)
	{
		m_gc->drawLine(x, 0, x, wheight - m_areagc->tlu(1));
		x += tmpw;
	}
}
UT_UCSChar XAP_Draw_Symbol::calcSymbolFromCoords(UT_uint32 ix, UT_uint32 iy)
{
	UT_uint32 index,count;
	index = iy * 32 + ix;
	count = 0;
	UT_DEBUGMSG(("calcSymbolFromCoords(x = [%u], y = [%u]) =", ix, iy));
	for (size_t i = 0; i < m_vCharSet.size(); i += 2)
	{
		count += reinterpret_cast<UT_uint32>(m_vCharSet[i + 1]);
		if (count > index)
		{
			UT_DEBUGMSG((" %u\n", static_cast<UT_uint32>(reinterpret_cast<UT_uint32>(m_vCharSet[i]) + index - count + reinterpret_cast<UT_uint32>(m_vCharSet[i + 1]))));
			return static_cast<UT_UCSChar>(reinterpret_cast<UT_uint32>(m_vCharSet[i]) + index - count + reinterpret_cast<UT_uint32>(m_vCharSet[i + 1]));
		}
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

	for (size_t i = 0; i < m_vCharSet.size(); i += 2)
	{
		UT_uint32 base = reinterpret_cast<UT_uint32>(m_vCharSet[i]);
		UT_uint32 size = reinterpret_cast<UT_uint32>(m_vCharSet[i + 1]);
		
		if (base + size > c)
		{
			index += c - base;
			break;
		}
		else
			index += size;
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
	UT_uint32 wwidth,wheight,x,y,cx,cy,px,py,swidth,sheight;
	UT_uint32 cx1,cy1,px1,py1;

	wwidth = m_drawareaWidth;
	wheight = m_drawareaHeight;

	// Center the character
	// Note: That's bogus.  measureString will give us the horizontal advance of "c",
	// but we need the bounding box of "c" instead.  To get it, we should use with FreeType face->bbox,
	// in windows we should use the (FIXME: find the right name, it was something as getCharABC(...)
	// NOTE: The Pango version has the same problem
#ifndef WITH_PANGO 	
	unsigned short w1 = m_areagc->measureUnRemappedChar(c);
#else	
	PangoGlyphString* pGlyph = m_areagc->getPangoGlyphString(c);
	PangoRectangle ink_rect;
	unsigned short w1;

	pango_glyph_string_extents(pGlyph, m_pFont, &ink_rect, NULL);
	w1 = ink_rect.width;
#endif	

	x = (m_drawareaWidth - w1) / 2;
	y = (m_drawareaHeight - m_areagc->getFontHeight()) / 2;

	m_areagc->clearArea(0, 0, wwidth, wheight);
#ifndef WITH_PANGO	
	m_areagc->drawChars(&c, 0, 1, x, y);
#else
	m_areagc->drawPangoGlyphString(pGlyph, x, y);
#endif
	//
	// Calculate the cordinates of the current and previous symbol
	// along with the widths of the appropriate boxes.
	swidth = m_drawWidth;
	sheight = m_drawHeight;
	UT_uint32 tmpw = m_drawWidth / 32;
	UT_uint32 tmph = m_drawHeight / 7;

	calculatePosition(c, cx, cy);
	unsigned short wc = m_gc->measureUnRemappedChar(c);

	cx *= tmpw;
	cy *= tmph;
	
	cx1 = cx + tmpw;
	cy1 = cy + tmph;

	calculatePosition(p, px, py);
	unsigned short wp = m_gc->measureUnRemappedChar(p);

	px *= tmpw;
	py *= tmph;
	
	px1 = px + tmpw;
	py1 = py + tmph;

	// Redraw the Previous Character in black on White
	m_gc->clearArea(px + m_areagc->tlu(1), py + m_areagc->tlu(1), tmpw - m_areagc->tlu(1), tmph - m_areagc->tlu(1));
#ifndef WITH_PANGO	
	m_gc->drawChars(&p, 0, 1, px + (tmpw - wp) / 2, py);
#else
	PangoGlyphString * pGlyph2 = m_gc->getPangoGlyphString(p);
	m_gc->drawPangoGlyphString(pGlyph2, px + m_areagc->tlu(2), py);
#endif

	// Redraw the Current Character in black on Blue
	UT_RGBColor colour(128, 128, 192);
	m_gc->fillRect(colour, cx + m_areagc->tlu(1), cy + m_areagc->tlu(1), tmpw - m_areagc->tlu(1), tmph - m_areagc->tlu(1));
#ifndef WITH_PANGO
	m_gc->drawChars(&c, 0, 1, cx + (tmpw - wc) / 2, cy);
#else
	m_gc->drawPangoGlyphString(pGlyph, cx + m_areagc->tlu(2), cy);

	pango_glyph_string_free(pGlyph);
	pango_glyph_string_free(pGlyph2);
#endif	
}

void XAP_Draw_Symbol::onLeftButtonDown(UT_sint32 x, UT_sint32 y)
{
	setCurrent(calcSymbol(x, y));
}
