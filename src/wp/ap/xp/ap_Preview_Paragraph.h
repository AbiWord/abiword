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


#ifndef AP_PREVIEW_PARAGRAPH_H
#define AP_PREVIEW_PARAGRAPH_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"

#include "xap_Preview.h"

class GR_Font;

// a mini-class representing a block of text to be
// drawn in the preview
class AP_Preview_Paragraph_Block
{
 public:
	AP_Preview_Paragraph_Block(UT_RGBColor & clr);
	virtual ~AP_Preview_Paragraph_Block();

	void setText(const UT_UCSChar * text);
	const UT_Vector * getWordsVector(void);
	const UT_RGBColor & getColor(void);
	
 protected:

	UT_RGBColor m_clr;

	// when a string is set, we break it into words for
	// easy layout
	UT_Vector m_words;
	
	// margins stored in pixels, since they don't
	// need to change depending on unit system/zoom, etc.
	UT_uint32 m_leftMargin;
	UT_uint32 m_rightMargin;
};
		
class AP_Preview_Paragraph : public XAP_Preview
{
 public:

	AP_Preview_Paragraph(GR_Graphics * gc, const UT_UCSChar * text);
	virtual ~AP_Preview_Paragraph(void);

	virtual void draw(void);
	
 protected:
	
	// standard colors to draw with
	UT_RGBColor * m_clrWhite;
	UT_RGBColor * m_clrBlack;
	UT_RGBColor * m_clrGray;

	// measurements in pixels for default left and right margins
	// (used to plot all paragraphs)
	UT_uint32 m_defaultLeftMargin;
	UT_uint32 m_defaultRightMargin;	
	UT_uint32 m_defaultTopMargin;
	
	// we flow using this as a current position
	UT_uint32 m_x;
	UT_uint32 m_y;	
	
	virtual UT_Bool _loadDrawFont(void);
	virtual void 	_drawPageBackground(void);
	virtual void 	_appendBlock(AP_Preview_Paragraph_Block * block);
	
	// mini-classes to hold some general data about these three
	// blocks
	AP_Preview_Paragraph_Block * m_previousBlock;
	AP_Preview_Paragraph_Block * m_activeBlock;
	AP_Preview_Paragraph_Block * m_followingBlock;
		
	GR_Font * m_pFont;
	UT_uint32 m_fontHeight;
};

#endif /* AP_PREVIEW_PARAGRAPH_H */
