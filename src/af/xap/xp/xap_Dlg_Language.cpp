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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Dlg_Language.h"

/*****************************************************************/

static int s_compareQ(const void * a, const void * b)
{
	const XML_Char ** A = (const XML_Char ** ) a;
	const XML_Char ** B = (const XML_Char ** ) b;

	return UT_strcoll(*A,*B);
}

XAP_Dialog_Language::XAP_Dialog_Language(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialoglanguage.html")
{
	m_answer		= a_CANCEL;
	m_pLanguage		= NULL;
	m_pLangProperty		= NULL;
	m_bChangedLanguage	= false;
	m_pLangTable = new UT_Language;
	
	UT_ASSERT(m_pLangTable);
	m_iLangCount = m_pLangTable->getCount();
	m_ppLanguages = new const XML_Char * [m_iLangCount];

	for(UT_uint32 i = 0; i < m_iLangCount; i++)
		m_ppLanguages[i] = m_pLangTable->getNthLanguage(i);
	qsort(m_ppLanguages, m_iLangCount, sizeof(XML_Char *), s_compareQ);

	// TODO: move spell-checking into XAP land and make this dialog
	// TODO: more like the MSWord one (i.e. add):
	// [] do not check spelling or grammar
	m_bSpellCheck = true;
}

XAP_Dialog_Language::~XAP_Dialog_Language(void)
{
	if(m_pLangTable)
		delete m_pLangTable;
	if(m_ppLanguages)
		delete [] m_ppLanguages;
}

// we will not use the value passed to us, but rather will reference
// ourselves into m_pLangTable; that way we do not have to worry about
// the string disappearing on us, nor do we need to clone it
void XAP_Dialog_Language::setLanguageProperty(const XML_Char * pLangProp)
{
	UT_ASSERT(m_pLangTable);
	UT_uint32 indx	= m_pLangTable->getIndxFromProperty(
		pLangProp ? pLangProp :"-none-");
	m_pLanguage		= m_pLangTable->getNthLanguage(indx);
	m_pLangProperty	= m_pLangTable->getNthProperty(indx);
}

// in this case we do not need to worry about the lifespan of pLang
// since we call it only internally, always referring back to m_pLangTable
void XAP_Dialog_Language::_setLanguage(const XML_Char * pLang)
{
	UT_ASSERT(m_pLangTable);
	m_pLanguage		= pLang;
	m_pLangProperty	= m_pLangTable->getPropertyFromLanguage(pLang);
}


XAP_Dialog_Language::tAnswer XAP_Dialog_Language::getAnswer(void) const
{
	return m_answer;
}

bool XAP_Dialog_Language::getChangedLangProperty(const XML_Char ** pszLangProp) const
{
	UT_ASSERT(pszLangProp);
	*pszLangProp = m_pLangProperty;
	return m_bChangedLanguage;
}

