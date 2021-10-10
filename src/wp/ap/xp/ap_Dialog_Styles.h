/* -*- mode: C++; tab-width: 4; c-basic-offset: 4;  indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009-2021 Hubert Figuière
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

#pragma once

#include <string>
#include <map>

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"
#include "xap_Dlg_FontChooser.h"
#include "ap_Preview_Abi.h"

class XAP_Frame;

#include "xap_Preview.h"
#include "ap_Preview_Paragraph.h"

class ABI_EXPORT AP_Dialog_Styles : public XAP_Dialog_NonPersistent
{
 public:
	typedef std::map<std::string,std::string> PropMap;

	AP_Dialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Styles(void);

	virtual void runModal(XAP_Frame * pFrame) override = 0;

	typedef enum { a_OK, a_CANCEL }   tAnswer;
	AP_Dialog_Styles::tAnswer	  getAnswer(void) const;

	// wish that this could be protected
	void _tabCallback(const char *, const char *);

 protected:
	void event_paraPreviewUpdated (const gchar * pageLeftMargin,
				       const gchar * pageRightMargin,
				       const gchar * align,
				       const gchar * firstLineIndent,
				       const gchar * leftIndent,
				       const gchar * rightIndent,
				       const gchar * beforeSpacing,
				       const gchar * afterSpacing,
				       const gchar * lineSpacing) const;
	virtual void event_charPreviewUpdated (void) const;

	virtual const char * getCurrentStyle (void) const = 0;
	virtual void setDescription (const char * desc) const = 0;
	virtual void setModifyDescription (const char * desc) = 0;
	virtual void _populatePreviews(bool isModify);
	FV_View * getLView(void) const;
	PD_Document * getLDoc(void) const;
	void invalidatePreview(void);
	void destroyAbiPreview(void);
	void fillVecWithProps(const gchar * szStyle, bool bReplaceAttributes);
	void fillVecFromCurrentPoint(void);
	const gchar * getVecVal(const UT_Vector * v, const gchar * szProp) const;
	void ModifyLists(void);
	void ModifyFont(void);
	void ModifyParagraph(void);
	void ModifyTabs(void);
	void ModifyLang(void);

	void updateCurrentStyle(void);
    bool createNewStyle(const gchar * szName);
	bool applyModifiedStyleToDoc(void);
	void setDoc(PD_Document * pDoc);
	void setFrame(XAP_Frame * pFrame);
	void setView(FV_View * pView);
	FV_View * getView(void) const;
	PD_Document * getDoc(void) const;
	XAP_Frame * getFrame(void) const;

protected:

	void				  _createParaPreviewFromGC(GR_Graphics * gc,  UT_uint32 width,  UT_uint32 height);

	void				  _createCharPreviewFromGC(GR_Graphics * gc,  UT_uint32 width, UT_uint32 height);
	void				  _createAbiPreviewFromGC(GR_Graphics * gc,  UT_uint32 width, UT_uint32 height);
	void                  _populateAbiPreview(bool isNew);
	AP_Dialog_Styles::tAnswer	  m_answer;
	PD_Style *                    m_pCurStyle;
	char *                        m_pszCurStyleName;
	std::string                       m_curStyleDesc;
	AP_Preview_Paragraph  *		  m_pParaPreview;
	XAP_Preview_FontPreview *	  m_pCharPreview;
	AP_Preview_Abi *	          m_pAbiPreview;
	PP_PropertyVector             m_vecAllProps;
	PP_PropertyVector             m_vecAllAttribs;

private:
	XAP_Frame *                   m_pFrame;
	FV_View *                     m_pView;
	PD_Document *                 m_pDoc;
	PT_DocPosition                m_posBefore;
	PT_DocPosition                m_posFocus;
	PT_DocPosition                m_posAfter;
	PropMap                       m_mapCharProps;
	std::string                   m_ListProps[8];
};
