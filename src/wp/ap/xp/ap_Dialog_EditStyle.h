/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2008 Ryan Pavlik <abiryan@ryand.net>
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

#ifndef AP_DIALOG_EDITSTYLE_H
#define AP_DIALOG_EDITSTYLE_H

#include <vector>
#include <string>

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

#include "pd_Style.h"
#include "pd_Document.h"
#include "ut_string.h"
#include "ut_vector.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_EditStyle : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_EditStyle(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_EditStyle(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;
	void							setDialogData(UT_UTF8String sName, PD_Style * pStyle, PD_Document * pDoc);

	typedef enum { a_OK, a_CANCEL } tAnswer;
	
	typedef enum { P_TEXT_ALIGN, P_TEXT_INDENT, P_MARGIN_LEFT, P_MARGIN_RIGHT,
		P_MARGIN_TOP, P_MARGIN_BOTTOM,
		P_LINE_HEIGHT, P_TABSTOPS, P_START_VALUE, P_LIST_DELIM, P_LIST_STYLE,
		P_LIST_DECIMAL, P_FIELD_FONT, P_FIELD_COLOR, P_KEEP_TOGETHER,
		P_KEEP_WITH_NEXT, P_ORPHANS, P_WIDOWS, P_DOM_DIR, P_LAST,
		C_BGCOLOR, C_COLOR, C_FONT_FAMILY, C_FONT_SIZE, C_FONT_STRETCH,
		C_FONT_STYLE, C_FONT_VARIANT, C_FONT_WEIGHT, C_TEXT_DECORATION,
		C_LANG } tProperty;

	AP_Dialog_EditStyle::tAnswer	getAnswer(void) const;
	
protected:
	// methods for constructing data structures
	bool							_deconstructStyle();
	bool							_reconstructStyle();

	// data we need
	UT_UTF8String m_sName;
	PD_Document * m_pDoc;
	PD_Style * m_pStyle;
	AP_Dialog_EditStyle::tAnswer		m_answer;
	
	// produced by _deconstructStyle
	std::vector<int>			m_vPropertyID;
	std::vector<std::string>	m_vPropertyValues;
	std::string					m_sUnrecognizedProps;
	std::vector<std::string>	m_vAllStyles;
	int							m_iBasedOn;
	int							m_iFollowedBy;
	bool						m_bIsCharStyle;
	std::string					m_sBasedOn;
	std::string					m_sType;
	std::string					m_sFollowedBy;

	// produced by _reconstructStyle
	std::string					m_sAllProperties;

};

#endif /* AP_DIALOG_EDITSTYLE_H */
