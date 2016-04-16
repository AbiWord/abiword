/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifndef AP_DIALOG_PARAGRAPH_H
#define AP_DIALOG_PARAGRAPH_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "xap_CustomWidget.h"

#include "pp_Property.h"

class XAP_Frame;
class AP_Preview_Paragraph;

#define SPIN_BUF_TEXT_SIZE	20

class ABI_EXPORT AP_Dialog_Paragraph : public XAP_Dialog_NonPersistent
{
	// the preview's block classes want to use some of our protected enums
	// below for similar alignment, etc.
	friend class AP_Preview_Paragraph;
	friend class AP_Preview_Paragraph_Block;

 public:

	typedef enum { align_UNDEF = 0, align_LEFT, align_CENTERED, align_RIGHT, align_JUSTIFIED } tAlignState;
	typedef enum { indent_UNDEF = 0, indent_NONE, indent_FIRSTLINE, indent_HANGING } tIndentState;
	typedef enum { spacing_UNDEF = 0, spacing_SINGLE, spacing_ONEANDHALF, spacing_DOUBLE,
				   spacing_ATLEAST, spacing_EXACTLY, spacing_MULTIPLE } tSpacingState;
	/* these are public because we must be able to identify widgets out this class (NSWindowController)*/
	typedef enum { id_MENU_ALIGNMENT = 0, id_SPIN_LEFT_INDENT,
				   id_SPIN_RIGHT_INDENT, id_MENU_SPECIAL_INDENT,
				   id_SPIN_SPECIAL_INDENT, id_SPIN_BEFORE_SPACING,
				   id_SPIN_AFTER_SPACING, id_MENU_SPECIAL_SPACING,
				   id_SPIN_SPECIAL_SPACING, id_CHECK_WIDOW_ORPHAN,
				   id_CHECK_KEEP_LINES, id_CHECK_PAGE_BREAK,
				   id_CHECK_SUPPRESS, id_CHECK_NO_HYPHENATE,
				   id_CHECK_KEEP_NEXT,
				   id_CHECK_DOMDIRECTION
				   } tControl;

	AP_Dialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Paragraph(void);

	virtual void	runModal(XAP_Frame * pFrame) = 0;

	// answer from dialog
	typedef enum { a_OK, a_CANCEL, a_TABS } tAnswer;

	bool setDialogData(const PP_PropertyVector & pProps);
	bool getDialogData(PP_PropertyVector & pProps);

	// expects a width in paper units.
	void setMaxWidth(UT_sint32 width) { m_iMaxWidth = UT_inchesFromPaperUnits(width); }

	AP_Dialog_Paragraph::tAnswer	getAnswer(void) const;

	// must be public, otherwise MSVC bails out
	// enumerated types for drop-down lists (option menus)
	typedef enum { check_FALSE = 0, check_TRUE, check_INDETERMINATE } tCheckState;

	typedef enum { op_INIT = 0, op_UICHANGE, op_SYNC } tOperation;

 protected:

	// handle the XP-job of attaching something to our m_paragraphPreview
	void _createPreviewFromGC(GR_Graphics * gc, UT_uint32 width, UT_uint32 height);

	void 			   	_setMenuItemValue(tControl item, UT_sint32 value, tOperation = op_UICHANGE);
	UT_sint32 			_getMenuItemValue(tControl item);
	void 				_setCheckItemValue(tControl item, tCheckState value, tOperation = op_UICHANGE);
	tCheckState 		_getCheckItemValue(tControl item);
	void 				_setSpinItemValue(tControl item, const gchar * value, tOperation = op_UICHANGE);
	const gchar * 	_getSpinItemValue(tControl item);
	const gchar *	_makeAbsolute(const gchar * value);

	void				_doSpin(tControl edit, UT_sint32 amt);
	virtual void		_syncControls(tControl changed, bool bAll = false);

	bool				_wasChanged(tControl item);

	// final dialog answer
	tAnswer					m_answer;

	std::string			m_pageLeftMargin;
	std::string			m_pageRightMargin;

	// store a pointer to our preview control
	AP_Preview_Paragraph *	m_paragraphPreview;

	XAP_Frame *				m_pFrame;

private:
	class ABI_EXPORT sControlData
	{
	private:
		UT_sint32		m_siData;
		tCheckState		m_csData;
		gchar *		m_szData;

		bool			m_bChanged;

	public:
		sControlData (UT_sint32 data);
		sControlData (tCheckState data);
		sControlData (gchar * data = 0); // default is empty string

		sControlData (const sControlData & rhs);

		~sControlData ();

		sControlData & operator= (const sControlData & rhs);

		inline bool getData (UT_sint32 & data) const
		{
			data = m_siData;
			return true;
		}
		inline bool getData (tCheckState & data) const
		{
			data = m_csData;
			return true;
		}
		inline bool getData (const gchar *& data) const
		{
			data = m_szData;
			return (data != NULL);
		}

		inline bool setData (UT_sint32 data)
		{
			m_siData = data;
			return true;
		}
		inline bool setData (tCheckState data)
		{
			m_csData = data;
			return true;
		}
		bool setData (const gchar * data);

		inline bool changed () const { return m_bChanged; }
		inline void changed (bool c) { m_bChanged = c; }
	};

	UT_GenericVector<sControlData *> m_vecProperties; // properties stored as a vector

	void					_addPropertyItem (tControl index, const sControlData & control_data);
	inline sControlData *	_getPropertyItem (tControl index) const
	{
		return m_vecProperties.getNthItem (static_cast<UT_uint32>(index));
	}

	// which dimension system we're using as "native" for this document
	UT_Dimension			m_dim;

	// current column width, in inches.
	double				m_iMaxWidth;
};

#endif /* AP_DIALOG_PARAGRAPH_H */
