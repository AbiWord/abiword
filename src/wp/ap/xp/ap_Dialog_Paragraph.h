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

#define SPIN_BUF_TEXT_SIZE	20

class AP_Dialog_Paragraph : public XAP_Dialog_NonPersistent
{
 public:

	AP_Dialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Paragraph(void);

	virtual void	runModal(XAP_Frame * pFrame) = 0;

	// answer from dialog
	typedef enum { a_OK, a_CANCEL, a_TABS } tAnswer;

	UT_Bool setDialogData(const XML_Char ** pProps);
 	UT_Bool getDialogData(const XML_Char **& pProps);

	AP_Dialog_Paragraph::tAnswer	getAnswer(void) const;
	
 protected:

	// enumerated types for drop-down lists (option menus)
	typedef enum { align_LEFT = 0, align_CENTERED, align_RIGHT, align_JUSTIFIED } tAlignState;
	typedef enum { indent_NONE = 0, indent_FIRSTLINE, indent_HANGING } tIndentState;
	typedef enum { spacing_SINGLE = 0, spacing_ONEANDHALF, spacing_DOUBLE,
				   spacing_ATLEAST, spacing_EXACTLY, spacing_MULTIPLE } tSpacingState;
	typedef enum { check_FALSE = 0, check_TRUE, check_INDETERMINATE } tCheckState;
	
	typedef enum { id_MENU_ALIGNMENT = 0, id_SPIN_LEFT_INDENT,
				   id_SPIN_RIGHT_INDENT, id_MENU_SPECIAL_INDENT,
				   id_SPIN_SPECIAL_INDENT, id_SPIN_BEFORE_SPACING,
				   id_SPIN_AFTER_SPACING, id_MENU_SPECIAL_SPACING,
				   id_SPIN_SPECIAL_SPACING, id_CHECK_WIDOW_ORPHAN,
				   id_CHECK_KEEP_LINES, id_CHECK_PAGE_BREAK,
				   id_CHECK_SUPPRESS, id_CHECK_NO_HYPHENATE,
				   id_CHECK_KEEP_NEXT } tControl;

	typedef enum { op_INIT = 0, op_UICHANGE, op_SYNC } tOperation;

	struct _sControlData
	{
		void * pData;
		UT_Bool bChanged;
	};
	typedef struct _sControlData sControlData;
	
	// handle the XP-job of attaching something to our m_paragraphPreview
	void _createPreviewFromGC(GR_Graphics * gc, UT_uint32 width, UT_uint32 height);

	void 			   	_setMenuItemValue(tControl item, UT_sint32 value, tOperation = op_UICHANGE);
	UT_uint32 			_getMenuItemValue(tControl item);
	void 				_setCheckItemValue(tControl item, tCheckState value, tOperation = op_UICHANGE);
	tCheckState 		_getCheckItemValue(tControl item);
	void 				_setSpinItemValue(tControl item, const XML_Char * value, tOperation = op_UICHANGE);
	const XML_Char * 	_getSpinItemValue(tControl item);

	void				_doSpin(tControl edit, UT_sint32 amt);
	virtual void		_syncControls(tControl changed, UT_Bool bAll = UT_FALSE);

	UT_Bool				_wasChanged(tControl item);
	
	// final dialog answer
	tAnswer					m_answer;

	// properties stored as a vector 
	UT_Vector				m_vecProperties;

	XML_Char				m_bufRightIndent[SPIN_BUF_TEXT_SIZE];
	XML_Char				m_bufLeftIndent[SPIN_BUF_TEXT_SIZE];
	XML_Char				m_bufSpecialIndent[SPIN_BUF_TEXT_SIZE];
	XML_Char				m_bufBeforeSpacing[SPIN_BUF_TEXT_SIZE];
	XML_Char				m_bufAfterSpacing[SPIN_BUF_TEXT_SIZE];
	XML_Char				m_bufSpecialSpacing[SPIN_BUF_TEXT_SIZE];
	
	// store a pointer to our preview control
	AP_Preview_Paragraph *	m_paragraphPreview;

	XAP_Frame *				m_pFrame;

	// which dimension system we're using as "native" for this document
	UT_Dimension			m_dim;
};

#endif /* AP_DIALOG_PARAGRAPH_H */
