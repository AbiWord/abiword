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

#ifndef AP_DIALOG_FONTCHOOSER_H
#define AP_DIALOG_FONTCHOOSER_H

#include "ut_types.h"
#include "xmlparse.h"
#include "ap_Dialog.h"
class DG_Graphics;

/*****************************************************************/

class AP_Dialog_FontChooser : public AP_Dialog_NonPersistent
{
public:
	AP_Dialog_FontChooser(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_FontChooser(void);

	virtual void					runModal(AP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL, a_YES, a_NO }	tAnswer;

	void							setGraphicsContext(DG_Graphics * pGraphics);
	void							setFontFamily(const XML_Char * pFontFamily);
	void							setFontSize(const XML_Char * pFontSize);
	void							setFontWeight(const XML_Char * pFontWeight);
	void							setFontStyle(const XML_Char * pFontStyle);
	void							setColor(const XML_Char * pColor);
	void							setFontDecoration(UT_Bool bUnderline, UT_Bool bStrikeOut);

	AP_Dialog_FontChooser::tAnswer	getAnswer(void) const;
	
	// the following return TRUE iff the user changed the value
	// of the control in the dialog.  they also return the value
	// of the field in the supplied argument w/o regard to whether
	// they changed it or not.  this value is a static string in
	// the dialog and must be valid only until the dialog is released.

	UT_Bool							getChangedFontFamily(const XML_Char ** pszFontFamily) const;
	UT_Bool							getChangedFontSize(const XML_Char ** pszFontSize) const;
	UT_Bool							getChangedFontWeight(const XML_Char ** pszFontWeight) const;
	UT_Bool							getChangedFontStyle(const XML_Char ** pszFontStyle) const;
	UT_Bool							getChangedColor(const XML_Char ** pszColor) const;
	UT_Bool							getChangedUnderline(UT_Bool * pbUnderline) const;
	UT_Bool							getChangedStrikeOut(UT_Bool * pbStrikeOut) const;

protected:
	AP_Dialog_FontChooser::tAnswer	m_answer;

	DG_Graphics *					m_pGraphics;			/* input */
	XML_Char *						m_pFontFamily;			/* input/output */
	XML_Char *						m_pFontSize;			/* input/output */
	XML_Char *						m_pFontWeight;			/* input/output */
	XML_Char *						m_pFontStyle;			/* input/output */
	XML_Char *						m_pColor;				/* input/output */
	UT_Bool							m_bUnderline;			/* input/output */
	UT_Bool							m_bStrikeOut;			/* input/output */

	UT_Bool							m_bChangedFontFamily;	/* output */
	UT_Bool							m_bChangedFontSize;		/* output */
	UT_Bool							m_bChangedFontWeight;	/* output */
	UT_Bool							m_bChangedFontStyle;	/* output */
	UT_Bool							m_bChangedColor;		/* output */
	UT_Bool							m_bChangedUnderline;	/* output */
	UT_Bool							m_bChangedStrikeOut;	/* output */
};

#endif /* AP_DIALOG_FONTCHOOSER_H */
