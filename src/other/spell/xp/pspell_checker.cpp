/* AbiSuite
 * Copyright (C) 2001, 2002 Dom Lachowicz
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

/* Pspell 0.12 added a size param to a lot of its functions. If this is
 * defined before <pspell.h>, we retain source compatibility with
 * Pspell 0.11. Theoretically with 0.12, we could use UCS-4, but in practice
 * this doesn't work at all. We'll stick to converting between  UCS-4 and UTF-8
 * instead.
 */
#define USE_ORIGINAL_MANAGER_FUNCS 1

#if defined(WIN32)
#include <windows.h>
// TODO Is this always going to be 15?
#define ASPELL_DLL_NAME "aspell-15"
#define ASPELL_PATH_BUFSIZE 128
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Strings.h"
#include "pspell_checker.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#if defined(WIN32)
HINSTANCE PSpellChecker::sm_hinstLib(0);
int PSpellChecker::sm_nDllUseCount(0);
#endif

/*!
 * Convert a UTF-8 string to a UTF-32 string
 *
 * \param word8 The zero-terminated input string in UTF-8 format
 * \return A zero-terminated UTF-32 string
 */
static UT_UCS4Char *
utf8_to_utf32(const char *word8)
{
	UT_UCS4Char * ucs4 = 0;
	UT_UCS4_cloneString (&ucs4, UT_UCS4String (word8).ucs4_str());
	return ucs4;
}

/*!
 * Constructor
 */
PSpellChecker::PSpellChecker ()
  :	m_pPSpellManager(0)
{
#if defined(WIN32)

	// only open the DLL once
	UT_DEBUGMSG(("SPELL: pspell++ == %d\n", sm_nDllUseCount));
	if (sm_nDllUseCount++ == 0)
	{
		HKEY hKey;
		TCHAR szPath[ASPELL_PATH_BUFSIZE];
		DWORD dwBufLen = ASPELL_PATH_BUFSIZE;
		LONG lRet;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						 TEXT("SOFTWARE\\Aspell"),
						 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
		{
			lRet = RegQueryValueEx(hKey,
								   TEXT("Path"),
								   NULL, NULL, reinterpret_cast<LPBYTE>(szPath),
								   &dwBufLen);

			RegCloseKey(hKey);

			if (lRet == ERROR_SUCCESS)
			{
				UT_String sPath(szPath);

				sPath += "\\"ASPELL_DLL_NAME;

				sm_hinstLib = LoadLibrary(sPath.c_str());

				if (sm_hinstLib)
					UT_DEBUGMSG(("SPELL: pspell %lx \?\?-\?\? dll loaded "ASPELL_DLL_NAME".dll %lx\n", this, sm_hinstLib));
				else
					UT_DEBUGMSG(("SPELL: pspell %lx \?\?-\?\? dll load failed "ASPELL_DLL_NAME".dll\n", this));
			}
			else
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		else
			UT_DEBUGMSG(("HIPI: Aspell registry key not found\n"));
	}

	if (!sm_hinstLib)
	{
		XAP_App		*pApp   = XAP_App::getApp ();
		XAP_Frame	*pFrame = pApp->getLastFocussedFrame ();

		UT_String sErr(UT_String_sprintf(pApp->getStringSet()->getValue(XAP_STRING_ID_SPELL_CANTLOAD_DLL),
										 ASPELL_DLL_NAME));

		if (pFrame)
			pFrame->showMessageBox(sErr.c_str(),
								   XAP_Dialog_MessageBox::b_O,
								   XAP_Dialog_MessageBox::a_OK);
	}
#endif
}

/*!
 * Destructor
 */
PSpellChecker::~PSpellChecker()
{
	// some versions of pspell segfault here for some reason
	if (m_pPSpellManager)
		delete_pspell_manager(m_pPSpellManager);

#if defined(WIN32)
	UT_DEBUGMSG(("SPELL: --pspell == %d\n", sm_nDllUseCount-1));
	if (--sm_nDllUseCount == 0)
	{
		if (sm_hinstLib)
		{
			FreeLibrary(sm_hinstLib);
			sm_hinstLib = 0;
			UT_DEBUGMSG(("SPELL: pspell %lx \?\?-\?\? dll unloaded %lx\n", this, sm_hinstLib));
		}
	}
#endif
}

/*!
 * Load the dictionary represented by szLang
 * szLang takes the form of {"en-US", "en_US", "en"}
 *
 * \param szLang The dictionary to load
 * \return true if we loaded the dictionary, false if not
 */
bool
PSpellChecker::_requestDictionary (const char * szLang)
{
	bool bSuccess = true;
	PspellConfig *spell_config;
	PspellCanHaveError *spell_error;

#if defined(WIN32)
	if (! sm_hinstLib)
		return false;
#endif
	UT_return_val_if_fail ( szLang, false );

	// Convert the language tag from en-US to en_US form
	char * lang = UT_strdup (szLang);
	char * hyphen = strchr (lang, '-');
	if (hyphen)
		*hyphen = '_';

	spell_config = new_pspell_config();
	pspell_config_replace(spell_config, "language-tag", lang);
	pspell_config_replace(spell_config, "encoding", "utf-8");

	spell_error = new_pspell_manager(spell_config);
	delete_pspell_config(spell_config);

	FREEP(lang);

	if (pspell_error_number(spell_error) != 0)
	{
		couldNotLoadDictionary ( szLang );
		UT_DEBUGMSG(("SpellCheckInit: Pspell error: %s\n",
					 pspell_error_message(spell_error)));
		bSuccess = false;
	}

	m_pPSpellManager = to_pspell_manager(spell_error);

	return bSuccess;
}

/*!
 * Is szWord in our dictionary?
 *
 * \param szWord The word you'd like to check
 * \param len The length of szWord
 *
 * \return One of SpellChecker::SpellCheckResult
 */
SpellChecker::SpellCheckResult
PSpellChecker::_checkWord (const UT_UCSChar * ucszWord, size_t len)
{
	SpellChecker::SpellCheckResult ret = SpellChecker::LOOKUP_FAILED;

	UT_return_val_if_fail ( m_pPSpellManager, SpellChecker::LOOKUP_ERROR );
	UT_return_val_if_fail ( ucszWord, SpellChecker::LOOKUP_ERROR );
	UT_return_val_if_fail ( len, SpellChecker::LOOKUP_ERROR );

	switch (pspell_manager_check(m_pPSpellManager, const_cast<char*>(UT_UTF8String (ucszWord, len).utf8_str())))
	{
	case 0:
		ret = SpellChecker::LOOKUP_FAILED;
		break;
	case 1:
		m_bIsDictionaryWord = true;
		ret = SpellChecker::LOOKUP_SUCCEEDED;
		break;
	default:
		ret = SpellChecker::LOOKUP_ERROR;
		break;
	}

	return ret;
}

/*!
 * Suggest replacement words for szWord
 * \param szWord Non-null word to find suggestions for
 * \param len Length of szWord
 *
 * \return A vector of UT_UCSChar * suggestions. The vector must be
 *         'delete'd and its UT_UCSChar * suggestions must be 'free()'d
 */
UT_Vector *
PSpellChecker::_suggestWord (const UT_UCSChar *ucszWord, size_t len)
{
	// Check validity
	UT_return_val_if_fail ( ucszWord && len, 0 );

	UT_Vector * pvSugg = new UT_Vector ();

	if (m_pPSpellManager)
	{
		const PspellWordList	*word_list;
		PspellStringEmulation	*suggestions;

		word_list = pspell_manager_suggest(m_pPSpellManager,
										   const_cast<char*>(UT_UTF8String(ucszWord, len).utf8_str()));

		// Normal spell checker suggests words second
		if (!m_bIsDictionaryWord && ((suggestions = pspell_word_list_elements(word_list)) != 0))
		{
			int count = pspell_word_list_size(word_list);
			const char *szSugg;

			int sugn = 0;
			while ((szSugg = pspell_string_emulation_next(suggestions)) != NULL)
			{
				UT_UCSChar *ucszSugg = utf8_to_utf32(szSugg);
				if (ucszSugg)
					pvSugg->addItem (static_cast<void *>(ucszSugg));
			}
			delete_pspell_string_emulation (suggestions);
		}
	}

	return pvSugg;
}

bool
PSpellChecker::addToCustomDict (const UT_UCSChar *ucszWord, size_t len)
{
	if (m_pPSpellManager && ucszWord && len)
	{
		pspell_manager_add_to_personal(m_pPSpellManager, const_cast<char *>(UT_UTF8String (ucszWord, len).utf8_str()));
		return true;
	}
	return false;
}

void
PSpellChecker::correctWord (const UT_UCSChar *toCorrect, size_t toCorrectLen,
							const UT_UCSChar *correct, size_t correctLen)
{
	UT_return_if_fail (m_pPSpellManager);
	UT_return_if_fail (toCorrect || toCorrectLen);
	UT_return_if_fail (correct || correctLen);

	UT_UTF8String bad (toCorrect, toCorrectLen);
	UT_UTF8String good (correct, correctLen);

	pspell_manager_store_replacement (m_pPSpellManager, bad.utf8_str(), good.utf8_str());
}

