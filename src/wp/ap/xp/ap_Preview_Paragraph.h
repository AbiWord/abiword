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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef AP_PREVIEW_PARAGRAPH_H
#define AP_PREVIEW_PARAGRAPH_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"

#include "xap_Preview.h"
#include "ap_Dialog_Paragraph.h"

// fwd decl.
class AP_Dialog_Lists;

class GR_Font;

// a mini-class representing a block of text to be
// drawn in the preview
class ABI_EXPORT AP_Preview_Paragraph_Block
{
 public:
	AP_Preview_Paragraph_Block(UT_RGBColor & clr,
							   GR_Graphics * gc,
							   AP_Dialog_Paragraph::tAlignState m_align,
							   UT_uint32 fontHeight);
	virtual ~AP_Preview_Paragraph_Block();

	// sets the vectors full of words and lengths
	void setText(const UT_UCSChar * text);

	void setFormat(const gchar * pageLeftMargin,
				   const gchar * pageRightMargin,
				   AP_Dialog_Paragraph::tAlignState align,
				   const gchar * firstLineIndent,
				   AP_Dialog_Paragraph::tIndentState indent,
				   const gchar * leftIndent,
				   const gchar * rightIndent,
				   const gchar * beforeSpacing,
				   const gchar * afterSpacing,
				   const gchar * lineSpacing,
				   AP_Dialog_Paragraph::tSpacingState spacing);

	// absolute pixel positions (relative to respective sides)
	// at which we'll start/stop drawing.  The firstLine element
	// applies to the first line in a block only.  The others
	// apply to the rest of the block.
	UT_uint32 m_firstLineLeftStop;
	UT_uint32 m_leftStop;
	UT_uint32 m_rightStop;

	UT_uint32 m_beforeSpacing;
	UT_uint32 m_afterSpacing;
	UT_uint32 m_lineSpacing;

	AP_Dialog_Paragraph::tAlignState m_align;
	AP_Dialog_Paragraph::tIndentState m_indent;
	AP_Dialog_Paragraph::tSpacingState m_spacing;

	UT_uint32 m_fontHeight;
	UT_RGBColor m_clr;

	GR_Graphics * m_gc;

	// when a string is set, we break it into words for
	// easy layout, and store the word content (UT_UCSChar *)
	// and its measured length in pixels (UT_uint32)
	UT_GenericVector<UT_UCSChar *> m_words;
	UT_NumberVector m_widths;
};

class ABI_EXPORT AP_Preview_Paragraph : public XAP_Preview
{
 public:

	AP_Preview_Paragraph(GR_Graphics * gc, const UT_UCSChar * text,
			     AP_Dialog_Paragraph * parent, const char * fontname = NULL);

	AP_Preview_Paragraph(GR_Graphics * gc, const UT_UCSChar * text,
			     XAP_Dialog * parent);

	AP_Preview_Paragraph(GR_Graphics * gc,
			     const UT_UCSChar * text,
			     AP_Dialog_Lists * dlg);

	virtual ~AP_Preview_Paragraph(void);

	void setFormat(const gchar * pageLeftMargin,
				   const gchar * pageRightMargin,
				   AP_Dialog_Paragraph::tAlignState align,
				   const gchar * firstLineIndent,
				   AP_Dialog_Paragraph::tIndentState indent,
				   const gchar * leftIndent,
				   const gchar * rightIndent,
				   const gchar * beforeSpacing,
				   const gchar * afterSpacing,
				   const gchar * lineSpacing,
				   AP_Dialog_Paragraph::tSpacingState spacing,
				   UT_BidiCharType dir = UT_BIDI_LTR);

	virtual void drawImmediate(const UT_Rect *clip = NULL) override;

 protected:

	// standard colors to draw with
	UT_RGBColor * m_clrWhite;
	UT_RGBColor * m_clrBlack;
	UT_RGBColor * m_clrGray;

	// we flow using this as a current position
	UT_uint32 m_x;
	UT_uint32 m_y;

	virtual bool _loadDrawFont(const char *name = NULL);
	virtual void 	_drawPageBackground(void);
	virtual void	_drawPageBorder(void);
	virtual void 	_appendBlock(AP_Preview_Paragraph_Block * block);
	virtual UT_uint32 _appendLine(UT_GenericVector<UT_UCSChar *> * words,
								  UT_NumberVector * widths,
								  UT_uint32 startWithWord,
								  UT_uint32 left,
								  UT_uint32 right,
								  AP_Dialog_Paragraph::tAlignState align,
								  UT_uint32 y);

	// mini-classes to hold some general data about these three
	// blocks
	AP_Preview_Paragraph_Block * m_previousBlock;
	AP_Preview_Paragraph_Block * m_activeBlock;
	AP_Preview_Paragraph_Block * m_followingBlock;

	GR_Font * m_font;
	UT_uint32 m_fontHeight;
	UT_BidiCharType m_dir;
};

#endif /* AP_PREVIEW_PARAGRAPH_H */
