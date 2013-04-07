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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_DIALOG_LANGUAGE_H
#define XAP_DIALOG_LANGUAGE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include <string>

#include "xap_Dialog.h"

#ifdef ENABLE_SPELL
#include "spell_manager.h"
#endif

#include "ut_vector.h"

class UT_Language;
class UT_UTF8String;

/********************************************************************
INSTRUCTIONS FOR DESIGN OF THE PLATFORM VERSIONS OF THIS DIALOGUE

(1)	implement runModal(); at the moment we display a single listbox

(2)	m_iLangCount will tell you how many list entries there will be;
	the language strings are then in m_ppLanguages (already sorted)

(3)	use _setLanguage() to set the member variables in response
	to the user selection when the dialog is closing.
*********************************************************************/



class ABI_EXPORT XAP_Dialog_Language : public XAP_Dialog_NonPersistent
{
public:
	typedef enum { a_OK, a_CANCEL, a_YES, a_NO }	tAnswer;

	XAP_Dialog_Language(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Language(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;
	void							setLanguageProperty(const gchar * pLangProp);
	bool							getChangedLangProperty(const gchar ** pszLangProp) const;

	bool                            isMakeDocumentDefault() const {return m_bDocDefault;}
	void                            setMakeDocumentDefault(bool b) {m_bDocDefault = b;}

	void                            getDocDefaultLangDescription(UT_UTF8String &s);
	void                            getDocDefaultLangDescription(std::string &s);
	void                            getDocDefaultLangCheckboxLabel(UT_UTF8String &s);
	void                            getDocDefaultLangCheckboxLabel(std::string &s);
	void                            setDocumentLanguage(const gchar * pLang);

	XAP_Dialog_Language::tAnswer	getAnswer(void) const;

	inline bool getSpellCheck(void) const {return m_bSpellCheck;}
	UT_Vector* 						getAvailableDictionaries();

protected:
	void							_setLanguage(const gchar * pLang);

	XAP_Dialog_Language::tAnswer	m_answer;

	// the following keeps the string that the user sees in the dialogue; this is locale-dependent
	const gchar *				m_pLanguage;
	// this keeps the actual property string corresponding to m_pLanguage
	const gchar *				m_pLangProperty;
	bool							m_bChangedLanguage;
	UT_Language *					m_pLangTable;
	const gchar **				m_ppLanguages;
	const gchar **				m_ppLanguagesCode;
	UT_uint32					    m_iLangCount;
	bool                            m_bSpellCheck;
	bool                            m_bDocDefault;
	std::string                   m_docLang;
};
#endif /* XAP_DIALOG_LANGUAGE_H */

