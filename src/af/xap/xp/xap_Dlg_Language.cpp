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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Language.h"
#include "xap_Dlg_Language.h"
#include "xap_Strings.h"   
#include "xap_App.h"

static bool is_utf8_encoding;

static int s_compareQ(const void * a, const void * b)                           
{                                                                               
	const gchar ** A = (const gchar **)(a);                                              
	const gchar ** B = (const gchar **)(b);
	
	if (is_utf8_encoding)
		return g_utf8_collate(*A,*B);
	else
		return g_ascii_strcasecmp(*A,*B);
}       

/*****************************************************************/

XAP_Dialog_Language::XAP_Dialog_Language(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialoglanguage"),
	  m_bDocDefault(false)
{
	UT_uint32 nDontSort = 0, nSort = 0;
	UT_uint32 i;	

	m_answer		   = a_CANCEL;
	m_pLanguage		   = NULL;
	m_pLangProperty	   = NULL;
	m_bChangedLanguage = false;
	m_pLangTable = new UT_Language;
	const gchar ** ppLanguagesTemp = new const gchar * [m_pLangTable->getCount()];	
	
	UT_ASSERT(m_pLangTable);
	m_iLangCount = m_pLangTable->getCount();
	m_ppLanguages = new const gchar * [m_iLangCount];	
	m_ppLanguagesCode = new const gchar * [m_iLangCount];	
	
	is_utf8_encoding = g_ascii_strcasecmp (XAP_App::getApp()->getDefaultEncoding(), "UTF-8") == 0;
	
	for(i=0; i<m_iLangCount; i++)                                           
	{                                                                       
		if (m_pLangTable->getNthId(i)==XAP_STRING_ID_LANG_0) // Unsorted languages
		{
			m_ppLanguages[nDontSort]=m_pLangTable->getNthLangName(i);                                                    
			nDontSort++;                                             
		}
		else
		{
			ppLanguagesTemp[nSort] = m_pLangTable->getNthLangName(i);                                                                     
			nSort++;
		}
	}                                                                       

	// sort the temporary array                                                                                                                      
	qsort(ppLanguagesTemp, m_iLangCount-nDontSort, sizeof(gchar *), s_compareQ);

	  
	// Copy the sorted codes and a ssign each language its code
	for(UT_uint32 nLang = 0; nLang < m_iLangCount; nLang++)
	{
		if (nLang>=nDontSort)
			m_ppLanguages[nLang]=ppLanguagesTemp[nLang-nDontSort];

		for(i = 0; i < m_iLangCount; i++)
		{
			if (strcmp (m_ppLanguages[nLang], m_pLangTable->getNthLangName(i))==0)
			{
				m_ppLanguagesCode[nLang] = m_pLangTable->getNthLangCode(i);
				break;
			}
		}
	}
	delete [] ppLanguagesTemp;

	// TODO: move spell-checking into XAP land and make this dialog
	// TODO: more like the MSWord one (i.e. add):
	// [] do not check spelling or grammar
	m_bSpellCheck = true;
}

XAP_Dialog_Language::~XAP_Dialog_Language(void)
{
	DELETEP(m_pLangTable);
	DELETEPV(m_ppLanguages);
	DELETEPV(m_ppLanguagesCode);
}

// we will not use the value passed to us, but rather will reference
// ourselves into m_pLangTable; that way we do not have to worry about
// the string disappearing on us, nor do we need to clone it
void XAP_Dialog_Language::setLanguageProperty(const gchar * pLangProp)
{
	UT_ASSERT(m_pLangTable);
	UT_uint32 indx = m_pLangTable->getIndxFromCode(
		pLangProp ? pLangProp :"-none-");
	m_pLanguage	    = m_pLangTable->getNthLangName(indx);
	m_pLangProperty = m_pLangTable->getNthLangCode(indx);
}

// in this case we do not need to worry about the lifespan of pLang
// since we call it only internally, always referring back to m_pLangTable
void XAP_Dialog_Language::_setLanguage(const gchar * pLang)
{
	UT_ASSERT(m_pLangTable);
	m_pLanguage	    = pLang;
	m_pLangProperty = m_pLangTable->getCodeFromName(pLang);
}


XAP_Dialog_Language::tAnswer XAP_Dialog_Language::getAnswer(void) const
{
	return m_answer;
}

bool XAP_Dialog_Language::getChangedLangProperty(const gchar ** pszLangProp) const
{
	UT_return_val_if_fail(pszLangProp,false);
	*pszLangProp = m_pLangProperty;
	return m_bChangedLanguage;
}

/*
	Creates a vector with a list of support languages for spell checking

	You must to g_free the allocated memory
*/
UT_Vector* XAP_Dialog_Language::getAvailableDictionaries()
{
#ifdef ENABLE_SPELL
	SpellChecker * checker = SpellManager::instance().getInstance();
	UT_Vector& vec= checker->getMapping();
	DictionaryMapping * mapping;
	UT_Vector* vecRslt = new UT_Vector();

	const UT_uint32 nItems = vec.getItemCount();

	for (UT_uint32 iItem = nItems; iItem; --iItem)
	{
		mapping = static_cast<DictionaryMapping*>(const_cast<void*>(vec.getNthItem(iItem - 1)));

		if (checker->doesDictionaryExist(mapping->lang.c_str()))
			vecRslt->addItem( g_strdup(mapping->lang.c_str()));
	}

	return vecRslt;
#else
	return NULL;
#endif
}

/*!
    Fills s with the string to be used as a label for the the default language checkbox
*/
void XAP_Dialog_Language::getDocDefaultLangCheckboxLabel(UT_UTF8String &s)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	UT_return_if_fail(pSS);

	std::string str;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_ULANG_DefaultLangChkbox, str);
	s = str;
}

void XAP_Dialog_Language::getDocDefaultLangCheckboxLabel(std::string &s)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	UT_return_if_fail(pSS);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_ULANG_DefaultLangChkbox, s);
}

/*!
    Fills s with the string to be displayed in the default language static control
*/
void XAP_Dialog_Language::getDocDefaultLangDescription(UT_UTF8String & s)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	UT_return_if_fail(pSS);

	std::string str;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_ULANG_DefaultLangLabel, str);
	s = str;

	s += m_docLang;
}

void XAP_Dialog_Language::getDocDefaultLangDescription(std::string & s)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	UT_return_if_fail(pSS);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_ULANG_DefaultLangLabel, s);

	s += m_docLang;
}


/*!
    Initialises the dialogue to the current default lanaguage
    pLang is a standard lang property (e.g., en-GB)
 */
void XAP_Dialog_Language::setDocumentLanguage(const gchar * pLang)
{
	UT_return_if_fail( pLang );
	UT_return_if_fail(m_pLangTable);
	
	UT_uint32 indx = m_pLangTable->getIdFromCode(pLang);

	// NB: m_docLang holds the translated language name, not the tag
	XAP_App::getApp()->getStringSet()->getValueUTF8(indx, m_docLang);
}


