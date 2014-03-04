/* AbiWord
 * Copyright (C) 2005 Martin Sevior.
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

#ifndef AP_DIALOG_LATEX_H
#define AP_DIALOG_LATEX_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_string_class.h"
#include "fv_View.h"


class XAP_Frame;

class ABI_EXPORT AP_Dialog_Latex : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_Latex(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Latex(void);

	typedef enum { a_OK, a_CANCEL } tAnswer;

	AP_Dialog_Latex::tAnswer		getAnswer(void) const;
	bool            convertLatexToMathML(void);
	virtual bool    getLatexFromGUI(void) = 0;
	void            setLatex(UT_UTF8String & sLatex)
	  { m_sLatex = sLatex;}
	void            fillLatex(UT_UTF8String & sLatex);
	virtual void    setLatexInGUI(void) = 0;
	void            getLatex(UT_UTF8String & sLatex)
	{ sLatex = m_sLatex;}
	void            setMathML(UT_UTF8String & sMathML)
	  { m_sMathML = sMathML;}
	void            insertIntoDoc(void);
        void            ConstructWindowName(void);
        void            setActiveFrame(XAP_Frame *pFrame);
protected:

	AP_Dialog_Latex::tAnswer  m_answer;
	UT_UTF8String             m_sWindowName;
	bool                      m_compact;
private:
	UT_UTF8String m_sLatex;
	UT_UTF8String m_sMathML;
};

#endif /* AP_DIALOG_LATEX_H */


