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

#ifndef AP_DIALOG_PARAGRAPH_H
#define AP_DIALOG_PARAGRAPH_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

class XAP_Frame;
class AP_Preview_Paragraph;

typedef struct _paragraphData paragraphData;

class AP_Dialog_Paragraph : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Paragraph(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	// answer from dialog
	typedef enum { a_OK, a_CANCEL, a_TABS } tAnswer;

	// Types for lists in spacing tab
	typedef enum
	{
		align_LEFT,
		align_CENTERED,
		align_RIGHT,
		align_JUSTIFIED
	} tAlignment;
		
	typedef enum
	{
		indent_NONE,
		indent_FIRSTLINE,
		indent_HANGING
	} tSpecialIndent;
		
	typedef enum
	{
		spacing_SINGLE,
		spacing_ONEANDHALF,
		spacing_DOUBLE,
		spacing_ATLEAST,
		spacing_EXACTLY,
		spacing_MULTIPLE
	} tLineSpacing;
	
	struct sParagraphData
	{
		// final dialog answer
		AP_Dialog_Paragraph::tAnswer	m_answer;

		// data for spacing tab
		AP_Dialog_Paragraph::tAlignment		m_alignmentType;
		AP_Dialog_Paragraph::tSpecialIndent	m_specialIndentType;
		AP_Dialog_Paragraph::tLineSpacing	m_lineSpacingType;

		UT_UCSChar * m_leftIndent;
		UT_UCSChar * m_rightIndent;
		UT_UCSChar * m_specialIndent;

		UT_UCSChar * m_beforeSpacing;
		UT_UCSChar * m_afterSpacing;
		UT_UCSChar * m_specialSpacing;

		// data for breaks tab
		UT_Bool		m_widowOrphanControl;
		UT_Bool		m_keepLinesTogether;
		UT_Bool		m_keepWithNext;
		UT_Bool		m_pageBreakBefore;

		UT_Bool		m_supressLineNumbers;
		UT_Bool		m_noHyphenate;

	};

	// This dialog has so much data to it, I've stuck it all in one
	// big structure to be set/read in one pass.
	void									setDialogData(AP_Dialog_Paragraph::sParagraphData d);
	AP_Dialog_Paragraph::sParagraphData * 	getDialogData(void) const;
	
protected:

	AP_Dialog_Paragraph::sParagraphData	m_paragraphData;

	// handle the XP-job of attaching something to our m_paragraphPreview
	void _createPreviewFromGC(GR_Graphics * gc, UT_uint32 width, UT_uint32 height);

	// data 
	AP_Preview_Paragraph *	m_paragraphPreview;
};

#endif /* AP_DIALOG_PARAGRAPH_H */
