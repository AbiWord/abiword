/* AbiWord
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

#include <stdlib.h>
#include <stdio.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"

#include "xap_Preview.h"
#include "ap_Preview_Paragraph.h"

/************************************************************************/

AP_Preview_Paragraph_Block::AP_Preview_Paragraph_Block(UT_RGBColor & clr)
{
	m_clr.m_red = clr.m_red;
	m_clr.m_grn = clr.m_grn;
	m_clr.m_blu = clr.m_blu;	
}

AP_Preview_Paragraph_Block::~AP_Preview_Paragraph_Block()
{
	// clear out our vector by freeing ONLY the first
	// word.  All the pointers in the vector point
	// to different offsets of one piece of memory allocated
	// all at once.
	UT_UCSChar * word = (UT_UCSChar *) m_words.getNthItem(0);

	FREEP(word);
}

void AP_Preview_Paragraph_Block::setText(const UT_UCSChar * text)
{
	UT_ASSERT(text);

	// clear the words vector
	UT_VECTOR_FREEALL(UT_UCSChar *, m_words);
	m_words.clear();

	// dup the string for harmful chunkification
	UT_UCSChar * clone = NULL;
	UT_UCS_cloneString(&clone, text);
	
	UT_UCSChar * i = clone;

	while (*i != 0)
	{
		// for each space
		if (*i == ' ')
		{
			// terminate this word
			*i = 0;

			// add clone item
			m_words.addItem(clone);

			// advance clone pointer for new word
			clone = i + 1;
		}
		i++;
	}
	// add last word
	m_words.addItem(clone);
}

const UT_Vector * AP_Preview_Paragraph_Block::getWordsVector(void)
{
	return & m_words;
}

const UT_RGBColor & AP_Preview_Paragraph_Block::getColor(void)
{
	return m_clr;
}

/************************************************************************/

// TODO : LOCALIZE THESE!

#define STRING_PREVIOUS_BLOCK "Previous Paragraph Previous Paragraph Previous Paragraph Previous Paragraph Previous Paragraph Previous Paragraph Previous Paragraph"
#define STRING_FOLLOWING_BLOCK "Following Paragraph Following Paragraph Following Paragraph Following Paragraph Following Paragraph Following Paragraph Following Paragraph"

AP_Preview_Paragraph::AP_Preview_Paragraph(GR_Graphics * gc, const UT_UCSChar * text)
	: XAP_Preview(gc)
{
	UT_ASSERT(text);
	
	m_pFont = NULL;
	m_fontHeight = 0;

	m_defaultLeftMargin = 20;
	m_defaultRightMargin = 20;	
	m_defaultTopMargin = 5;
	
	m_x = m_defaultLeftMargin;
	m_y = m_defaultTopMargin;
		
	m_clrWhite = new UT_RGBColor(255,255,255);
	m_clrBlack = new UT_RGBColor(0,0,0);
	m_clrGray = new UT_RGBColor(192,192,192);

	m_previousBlock = new AP_Preview_Paragraph_Block(*m_clrGray);
	m_activeBlock = new AP_Preview_Paragraph_Block(*m_clrBlack);
	m_followingBlock = new AP_Preview_Paragraph_Block(*m_clrGray);

	// TODO : LOCALIZE THESE!

	UT_UCSChar * tmp = NULL;

	UT_UCS_cloneString_char(&tmp, STRING_PREVIOUS_BLOCK);
	m_previousBlock->setText(tmp);
	FREEP(tmp);
	
	m_activeBlock->setText(text);

	UT_UCS_cloneString_char(&tmp, STRING_FOLLOWING_BLOCK);	
	m_followingBlock->setText(tmp);
	FREEP(tmp);
}

AP_Preview_Paragraph::~AP_Preview_Paragraph()
{
	DELETEP(m_pFont);

	DELETEP(m_clrWhite);
	DELETEP(m_clrBlack);
	DELETEP(m_clrGray);

	DELETEP(m_previousBlock);
	DELETEP(m_activeBlock);
	DELETEP(m_followingBlock);	
}

void AP_Preview_Paragraph::draw(void)
{
	UT_ASSERT(m_gc);

	if (!m_pFont)
		_loadDrawFont();
	
	// paint white background (Word 97's background is always white, the
	// text is always black, and the font is always the same size, regardless
	// of the current document's real block formatting).
	_drawPageBackground();

	// draw the three paragraphs
	_appendBlock(m_previousBlock);
	_appendBlock(m_activeBlock);
	_appendBlock(m_followingBlock);

	m_x = m_defaultLeftMargin;
	m_y = m_defaultTopMargin;
}

UT_Bool AP_Preview_Paragraph::_loadDrawFont(void)
{
	// we draw at 5 points in this preview
	GR_Font * font = m_gc->findFont("Times New Roman", "normal", "", "normal", "", "6pt");
	
	if (font)
	{
		REPLACEP(m_pFont, font);
		m_gc->setFont(m_pFont);
		m_fontHeight = m_gc->getFontHeight();
		return UT_TRUE;
	}
	else
		return UT_FALSE;
}

void AP_Preview_Paragraph::_drawPageBackground(void)
{
	// clear area
	m_gc->fillRect(*m_clrWhite, 0, 0, getWindowWidth(), getWindowHeight());

	// draw a black one pixel border
	m_gc->setColor(*m_clrBlack);
	m_gc->drawLine(0, 0, getWindowWidth(), 0);
	m_gc->drawLine(getWindowWidth() - 1, 0, getWindowWidth() - 1, getWindowHeight());
	m_gc->drawLine(getWindowWidth() - 1, getWindowHeight() - 1, 0, getWindowHeight() - 1);
	m_gc->drawLine(0, getWindowHeight() - 1, 0, 0);
}

void AP_Preview_Paragraph::_appendBlock(AP_Preview_Paragraph_Block * block)
{
	UT_ASSERT(block);

	const UT_Vector * words = block->getWordsVector();
	UT_ASSERT(words);

	UT_uint32 wordIndex = 0;

	UT_uint32 wordLen = 0;
	UT_uint32 wordWidth = 0;

	UT_uint32 wordCount = words->getItemCount();
	
	// starting at m_x and m_y, we flow text until we hit the margins,
	// then wrap and start over again at m_y + lineheight
	while (wordIndex < wordCount)
	{
		UT_UCSChar * word = (UT_UCSChar *) words->getNthItem(wordIndex);
		UT_ASSERT(word);

		// TODO : maybe move this strlen to the block's setText() member to
		// TODO : speed things up
		wordLen = UT_UCS_strlen(word);

		wordWidth = m_gc->measureString(word, 0, wordLen, NULL);

		// if it won't fit, wrap the Y coordinate before plotting
		if ((wordWidth + m_x) >= (getWindowWidth() - m_defaultRightMargin))
		{
			m_y += m_fontHeight + 1;
			m_x = m_defaultLeftMargin;
		}

		// plot the text
		m_gc->setColor((UT_RGBColor &) block->getColor());
		m_gc->drawChars(word, 0, wordLen, m_x, m_y);
		m_x += wordWidth;

		// print a space as 3 pixels
		m_x += 3;
		
		// next word
		wordIndex++;
	}

	// always drop down one more line to start next paragraph
	m_y += m_fontHeight + 1;

	// start next at margin
	m_x = m_defaultLeftMargin;
}
			
		

		
		
	
