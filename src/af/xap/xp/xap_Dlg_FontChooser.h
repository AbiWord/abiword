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

#ifndef XAP_DIALOG_FONTCHOOSER_H
#define XAP_DIALOG_FONTCHOOSER_H

#include "ut_types.h"
#include "ut_xml.h"
#include "ut_string.h"
#include "xap_Dialog.h"
#include "xap_Preview.h"

class GR_Graphics;

class XAP_Preview_FontPreview  : public XAP_Preview
{
public:
	XAP_Preview_FontPreview(GR_Graphics * gc, const XML_Char * pszClrBackgound);
	virtual ~XAP_Preview_FontPreview(void);
	void setVecProperties( const UT_Vector * vFontProps);
    const XML_Char * getVal(const XML_Char * szProp);
	void draw(void);
	void setDrawString( const UT_UCSChar * pszChars) {m_pszChars = pszChars;}
	void clearScreen(void);


protected:
	UT_Vector * m_vecProps;
	UT_RGBColor m_clrBackground;
	const UT_UCSChar * m_pszChars;
};

/*****************************************************************/

class XAP_Dialog_FontChooser : public XAP_Dialog_NonPersistent
{
public:
	XAP_Dialog_FontChooser(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_FontChooser(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL, a_YES, a_NO }	tAnswer;

	void                            addOrReplaceVecProp(const XML_Char * pszProp, 
														const XML_Char * pszVal);
	void                            event_previewExposed(const UT_UCSChar * pszChars);
	void                            event_previewClear(void);
	const XML_Char *                getVal(const XML_Char * szProp) const;
	void                            setAllPropsFromVec(UT_Vector * vProps);

	void							setGraphicsContext(GR_Graphics * pGraphics);
	void							setFontFamily(const XML_Char * pFontFamily);
	void							setFontSize(const XML_Char * pFontSize);
	void							setFontWeight(const XML_Char * pFontWeight);
	void							setFontStyle(const XML_Char * pFontStyle);
	void							setColor(const XML_Char * pColor);
	void							setBGColor(const XML_Char * pBGColor);
	void							setFontDecoration(bool bUnderline, bool bOverline, bool bStrikeOut);
	void                            setBackGroundColor (const XML_Char * pBackGroundColor);

	
/*
#ifdef BIDI_ENABLED
	void                            setDirection(bool bDirection);
#endif
*/


	XAP_Dialog_FontChooser::tAnswer	getAnswer(void) const;
	
	// the following return TRUE iff the user changed the value
	// of the control in the dialog.  they also return the value
	// of the field in the supplied argument w/o regard to whether
	// they changed it or not.  this value is a static string in
	// the dialog and must be valid only until the dialog is released.
	bool                            didPropChange(const XML_Char * v1, const XML_Char * v2) const;
	bool							getChangedFontFamily(const XML_Char ** pszFontFamily) const;
	bool							getChangedFontSize(const XML_Char ** pszFontSize) const;
	bool							getChangedFontWeight(const XML_Char ** pszFontWeight) const;
	bool							getChangedFontStyle(const XML_Char ** pszFontStyle) const;
	bool							getChangedColor(const XML_Char ** pszColor) const;
	bool							getChangedBGColor(const XML_Char ** pszColor) const;
	bool							getChangedUnderline(bool * pbUnderline) const;
	bool							getChangedOverline(bool * pbOverline) const;
	bool							getChangedStrikeOut(bool * pbStrikeOut) const;
/*
#ifdef BIDI_ENABLED
	bool                         	getChangedDirection(bool * pbDirection) const;
#endif
*/

protected:
	void                            _createFontPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);

	XAP_Dialog_FontChooser::tAnswer	m_answer;
/*
#ifdef BIDI_ENABLED
	virtual void _enableDirectionCheck(bool b) = 0;
	void _initEnableControls();
#endif
*/
	GR_Graphics *					m_pGraphics;			/* input */
	const XML_Char *                m_pColorBackground;
	XML_Char *						m_pFontFamily;			/* input/output */
	XML_Char *						m_pFontSize;			/* input/output */
	XML_Char *						m_pFontWeight;			/* input/output */
	XML_Char *						m_pFontStyle;			/* input/output */
	XML_Char *						m_pColor;				/* input/output */
	XML_Char *						m_pBGColor;				/* input/output */
	bool							m_bUnderline;			/* input/output */
	bool							m_bOverline;			/* input/output */
	bool							m_bStrikeout;			/* input/output */

	UT_Vector                       m_vecProps; // Holds the current
/*
#ifdef BIDI_ENABLED
	bool							m_bDirection;
#endif
*/
	XAP_Preview_FontPreview *          m_pFontPreview;

	bool							m_bChangedFontFamily;	/* output */
	bool							m_bChangedFontSize;		/* output */
	bool							m_bChangedFontWeight;	/* output */
	bool							m_bChangedFontStyle;	/* output */
	bool							m_bChangedColor;		/* output */
	bool							m_bChangedBGColor;		/* output */
	bool							m_bChangedUnderline;	/* output */
	bool							m_bChangedOverline;	/* output */
	bool							m_bChangedStrikeOut;	/* output */
/*
#ifdef BIDI_ENABLED
	bool							m_bChangedDirection;
#endif
*/
};

#endif /* XAP_DIALOG_FONTCHOOSER_H */

