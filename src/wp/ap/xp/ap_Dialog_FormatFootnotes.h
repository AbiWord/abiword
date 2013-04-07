/* AbiWord
 * Copyright (c) 2003 Martin Sevior
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

#ifndef AP_DIALOG_FORMATFOOTNOTES_H
#define AP_DIALOG_FORMATFOOTNOTES_H

#include "xap_Dialog.h"
#include "ut_xml.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_FormatFootnotes : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_FormatFootnotes(XAP_DialogFactory * pDlgFactory,
				 XAP_Dialog_Id id);
	virtual ~AP_Dialog_FormatFootnotes(void);

	virtual void		 runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK=0, a_CANCEL=1, a_DELETE=2 } tAnswer;

	tAnswer 			getAnswer(void) const;
	void				setAnswer(tAnswer a);
	void                setFrame(XAP_Frame * pFrame);
	void                setFootnoteVal(UT_sint32 iVal);
	UT_sint32           getFootnoteVal(void);
	void                getFootnoteValString(UT_String & sVal);
	void                setFootnoteType(FootnoteType iFootType);
	FootnoteType        getFootnoteType(void);
	void                setRestartFootnoteOnSection(bool bVal);
	bool                getRestartFootnoteOnSection(void);
	void                setRestartFootnoteOnPage(bool bVal);
	bool                getRestartFootnoteOnPage(void);

	void                setEndnoteVal(UT_sint32 iVal);
	UT_sint32           getEndnoteVal(void);
	void                getEndnoteValString(UT_String & sVal);
	void                setEndnoteType(FootnoteType iFootType);
	FootnoteType        getEndnoteType(void);
	void                setRestartEndnoteOnSection(bool bVal);
	bool                getRestartEndnoteOnSection(void);
	void                setRestartEndnoteOnPage(bool bVal);
	bool                getRestartEndnoteOnPage(void);
	void                setPlaceAtDocEnd(bool bVal);
	bool                getPlaceAtDocEnd(void);
	void                setPlaceAtSecEnd(bool bVal);
	bool                getPlaceAtSecEnd(void);

	void                recalcTextValues(void);
	void                updateDocWithValues(void);
	void                setInitialValues(void);

	static const FootnoteTypeDesc * getFootnoteTypeLabelList(void);

private:
	tAnswer         m_answer;
	FL_DocLayout *  m_pDocLayout;
	XAP_Frame *     m_pFrame;
	FV_View *       m_pView;
	PD_Document *   m_pDoc;
	UT_sint32       m_iFootnoteVal;
	UT_String       m_sFootnoteVal;
	FootnoteType    m_iFootnoteType;
	bool            m_bRestartFootSection;
	bool            m_bRestartFootPage;
	UT_sint32       m_iEndnoteVal;
	UT_String       m_sEndnoteVal;
	FootnoteType    m_iEndnoteType;
	bool            m_bRestartEndSection;
	bool            m_bPlaceAtDocEnd;
	bool            m_bPlaceAtSecEnd;
};

#endif /* AP_DIALOG_FORMATFOOTNOTES_H */
