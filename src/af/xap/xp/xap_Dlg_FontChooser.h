/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef XAP_DIALOG_FONTCHOOSER_H
#define XAP_DIALOG_FONTCHOOSER_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include <map>
#include <string>

#include "ut_string.h"

#include "xap_Dialog.h"
#include "xap_Preview.h"
#include "gr_Graphics.h"

class GR_Graphics;
class XAP_Preview_FontPreview; //forward, see below

/*****************************************************************/

class ABI_EXPORT XAP_Dialog_FontChooser : public XAP_Dialog_NonPersistent
{
public:
	typedef std::map<std::string,std::string> PropMap;

	XAP_Dialog_FontChooser(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_FontChooser(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL, a_YES, a_NO }	tAnswer;

	void setDrawString (const UT_UCSChar * str);

	const UT_UCSChar * getDrawString ()
	  {
	    return m_drawString;
	  }

	void                            addOrReplaceVecProp(const std::string & pszProp,
														const std::string & pszVal);
	void                            event_previewExposed(const UT_UCSChar * pszChars);
	void                            event_previewClear(void);
	const std::string&              getVal(const std::string & sProp) const;
	void                            setAllPropsFromVec(const UT_Vector & vProps);

	void							setGraphicsContext(GR_Graphics * pGraphics);
	void							setFontFamily(const std::string& sFontFamily);
	void							setFontSize(const std::string& sFontSize);
	void							setFontWeight(const std::string& sFontWeight);
	void							setFontStyle(const std::string& sFontStyle);
	void							setColor(const std::string& sColor);
	void							setBGColor(const std::string& sBGColor);
	void							setFontDecoration(bool bUnderline, bool bOverline, bool bStrikeOut, bool bTopline, bool bBottomline);
	void                            setHidden(bool bHidden);
	void                            setSuperScript(bool bSuperScript);
	void                            setSubScript(bool bSubScript);
	void                            setBackGroundColor (const gchar * pBackGroundColor);
	void                            setTextTransform (const std::string& pTextTransform);

	XAP_Dialog_FontChooser::tAnswer	getAnswer(void) const;

	// the following return TRUE iff the user changed the value
	// of the control in the dialog.  they also return the value
	// of the field in the supplied argument w/o regard to whether
	// they changed it or not.  this value is a static string in
	// the dialog and must be valid only until the dialog is released.
	bool                            didPropChange(const std::string & v1, const std::string & v2) const;
	bool							getChangedTextTransform(const gchar ** pszTextTransform) const;
	bool							getChangedFontFamily(const gchar ** pszFontFamily) const;
	bool							getChangedFontSize(const gchar ** pszFontSize) const;
	bool							getChangedFontWeight(const gchar ** pszFontWeight) const;
	bool							getChangedFontStyle(const gchar ** pszFontStyle) const;
	bool							getChangedColor(const gchar ** pszColor) const;
	bool							getChangedBGColor(const gchar ** pszColor) const;
	bool							getChangedUnderline(bool * pbUnderline) const;
	bool							getChangedOverline(bool * pbOverline) const;
	bool							getChangedStrikeOut(bool * pbStrikeOut) const;
	bool							getChangedTopline(bool * pbTopline) const;
	bool							getChangedBottomline(bool * pbBottomline) const;
	bool							getChangedHidden(bool * pbHidden) const;
	bool							getChangedSuperScript(bool * pbSuperScript) const;
	bool							getChangedSubScript(bool * pbSubScript) const;

protected:
	void                            _createFontPreviewFromGC(GR_Graphics * gc,
															 UT_uint32 width,
															 UT_uint32 height);

	XAP_Dialog_FontChooser::tAnswer	m_answer;
	GR_Graphics *					m_pGraphics;			/* input */
	std::string						m_sColorBackground;
	std::string						m_sFontFamily;			/* input/output */
	std::string						m_sFontSize;			/* input/output */
	std::string						m_sFontWeight;			/* input/output */
	std::string						m_sFontStyle;			/* input/output */
	std::string						m_sColor;				/* input/output */
	std::string						m_sBGColor;				/* input/output */
	bool							m_bUnderline;			/* input/output */
	bool							m_bOverline;			/* input/output */
	bool							m_bStrikeout;			/* input/output */
	bool							m_bTopline;		 	    /* input/output */
	bool							m_bBottomline;			/* input/output */
	bool                            m_bHidden;				/* input/output */
	bool                            m_bSuperScript;			/* input/output */
	bool                            m_bSubScript;			/* input/output */
	std::string                     m_sTextTransform;       /* input/output */

	PropMap                         m_mapProps; // Holds the current
	XAP_Preview_FontPreview *       m_pFontPreview;

	bool							m_bChangedFontFamily;	/* output */
	bool							m_bChangedFontSize;		/* output */
	bool							m_bChangedFontWeight;	/* output */
	bool							m_bChangedFontStyle;	/* output */
	bool							m_bChangedColor;		/* output */
	bool							m_bChangedBGColor;		/* output */
	bool							m_bChangedUnderline;	/* output */
	bool							m_bChangedOverline;	    /* output */
	bool							m_bChangedStrikeOut;	/* output */
	bool							m_bChangedTopline;	    /* output */
	bool							m_bChangedBottomline;	/* output */
	bool                            m_bChangedHidden;		/* output */
	bool                            m_bChangedSuperScript;	/* output */
	bool                            m_bChangedSubScript;	/* output */
	bool                            m_bChangedTextTransform;	/* output */

	UT_UCSChar *                    m_drawString;
};

class ABI_EXPORT XAP_Preview_FontPreview  : public XAP_Preview
{
public:
	explicit XAP_Preview_FontPreview(GR_Graphics * gc, const gchar * pszClrBackgound);
	virtual ~XAP_Preview_FontPreview(void);
	void setVecProperties( const XAP_Dialog_FontChooser::PropMap * vFontProps);
	const std::string getVal(const std::string &);
	void draw(const UT_Rect *clip=NULL);
	void setDrawString( const UT_UCSChar * pszChars) {m_pszChars = pszChars;}
	void clearScreen(void);


protected:
	const XAP_Dialog_FontChooser::PropMap * m_mapProps;
	UT_RGBColor m_clrBackground;
	const UT_UCSChar * m_pszChars;

private:

	XAP_Preview_FontPreview();
	XAP_Preview_FontPreview(const XAP_Preview_FontPreview &other);
	XAP_Preview_FontPreview& operator=(const XAP_Preview_FontPreview & other);

	GR_Font * m_pFont;

	UT_sint32 m_iAscent;
	UT_sint32 m_iDescent;
	UT_sint32 m_iHeight;
};

#endif /* XAP_DIALOG_FONTCHOOSER_H */
