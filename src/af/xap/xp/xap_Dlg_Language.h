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

#ifndef XAP_DIALOG_LANGUAGE_H
#define XAP_DIALOG_LANGUAGE_H

#include "ut_types.h"
#include "ut_xml.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Dialog.h"
#include "ut_Language.h"

/********************************************************************
INSTRUCTIONS FOR DESIGN OF THE PLATFORM VERSIONS OF THIS DIALOGUE

(1)	implement runModal(); at the moment we display a single listbox

(2)	use m_pLangTable->getCount() and m_pLangTable->getNthLanguage()
	to fill the listbox with strings; the languages are already
	sorted, so you can just use a for loop to do that.

(3)	use _setLanguage() to set the member variables in response
	to the user selection when the dialog is closing.
*********************************************************************/



class XAP_Dialog_Language : public XAP_Dialog_NonPersistent
{
public:
	typedef enum { a_OK, a_CANCEL, a_YES, a_NO }	tAnswer;
	
	XAP_Dialog_Language(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Language(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;
	void							setLanguageProperty(const XML_Char * pLangProp);
	bool							getChangedLangProperty(const XML_Char ** pszLangProp) const;
	XAP_Dialog_Language::tAnswer	getAnswer(void) const;
	

protected:
	void							_setLanguage(const XML_Char * pLang);
	
	XAP_Dialog_Language::tAnswer	m_answer;

	// the following keeps the string that the user sees in the dialogue; this is locale-dependent
	const XML_Char *				m_pLanguage;
	// this keeps the actual property string corresponding to m_pLanguage
	const XML_Char *				m_pLangProperty;
	bool							m_bChangedLanguage;
	UT_Language *					m_pLangTable;
};
#endif /* XAP_DIALOG_LANGUAGE_H */

