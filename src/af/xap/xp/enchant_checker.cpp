/* AbiSuite
 * Copyright (C) 2003 Dom Lachowicz
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Strings.h"
#include "enchant_checker.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

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

static size_t s_enchant_broker_count = 0;
static EnchantBroker * s_enchant_broker = 0;

EnchantChecker::EnchantChecker()
	: m_dict(0)
{
	if (s_enchant_broker_count == 0)
	{
		s_enchant_broker = enchant_broker_init ();
#ifdef _MSC_VER
		// hack: the old dictionary installers download to the "dictionary" path...
		gchar* ispell_path1 = g_build_filename (XAP_App::getApp()->getAbiSuiteLibDir(), "dictionary", NULL);
		// ... while in the new situation we support multiple types of dictionaries
		gchar* ispell_path2 = g_build_filename (XAP_App::getApp()->getAbiSuiteLibDir(), "dictionary", "ispell", NULL);
		std::string ispell_path = std::string(ispell_path1) + ";" + std::string(ispell_path2);
		enchant_broker_set_param(s_enchant_broker,  "enchant.ispell.dictionary.path", ispell_path.c_str());
		g_free(ispell_path1);
		g_free(ispell_path2);

		gchar* myspell_path = g_build_filename (XAP_App::getApp()->getAbiSuiteLibDir(), "dictionary", "myspell", NULL);
		enchant_broker_set_param(s_enchant_broker,  "enchant.myspell.dictionary.path", myspell_path);
		g_free(myspell_path);
#endif
	}
	s_enchant_broker_count++;
}

EnchantChecker::~EnchantChecker()
{
	UT_return_if_fail (s_enchant_broker);

	if (m_dict)
		enchant_broker_free_dict (s_enchant_broker, m_dict);

	s_enchant_broker_count--;
	if (s_enchant_broker_count == 0) {
		enchant_broker_free (s_enchant_broker);
		s_enchant_broker = 0;
	}
}

SpellChecker::SpellCheckResult
EnchantChecker::_checkWord (const UT_UCSChar * ucszWord, size_t len)
{
	UT_return_val_if_fail (m_dict, SpellChecker::LOOKUP_ERROR);
	UT_return_val_if_fail (ucszWord, SpellChecker::LOOKUP_ERROR);
	UT_return_val_if_fail (len, SpellChecker::LOOKUP_ERROR);

	UT_UTF8String utf8 (ucszWord, len);

	switch (enchant_dict_check (m_dict, utf8.utf8_str(), utf8.byteLength())) 
	{
	case -1:
		return SpellChecker::LOOKUP_ERROR;
	case 0:
		return SpellChecker::LOOKUP_SUCCEEDED;
	default:
		return SpellChecker::LOOKUP_FAILED;
	}
}

UT_GenericVector<UT_UCSChar*> *
EnchantChecker::_suggestWord (const UT_UCSChar *ucszWord, size_t len)
{
	UT_return_val_if_fail (m_dict, 0);
	UT_return_val_if_fail (ucszWord && len, 0);

	UT_GenericVector<UT_UCSChar*> * pvSugg = new UT_GenericVector<UT_UCSChar*>();

	UT_UTF8String utf8 (ucszWord, len);

	char ** suggestions;
	size_t n_suggestions;

	suggestions = enchant_dict_suggest (m_dict, utf8.utf8_str(), utf8.byteLength(), &n_suggestions);

	if (suggestions && n_suggestions) {
		for (size_t i = 0; i < n_suggestions; i++) {
			UT_UCSChar *ucszSugg = utf8_to_utf32(suggestions[i]);
			if (ucszSugg)
				pvSugg->addItem (ucszSugg);
		}

		enchant_dict_free_suggestions (m_dict, suggestions);
	}

	return pvSugg;
}

bool EnchantChecker::addToCustomDict (const UT_UCSChar *word, size_t len)
{
	UT_return_val_if_fail (m_dict, false);

	if (word && len) {
		UT_UTF8String utf8 (word, len);
		enchant_dict_add_to_personal (m_dict, utf8.utf8_str(), utf8.byteLength());
		return true;
	}
	return false;
}

bool EnchantChecker::isIgnored (const UT_UCSChar *toCorrect, size_t toCorrectLen) const
{
	UT_return_val_if_fail (m_dict, false);

	UT_UTF8String ignore (toCorrect, toCorrectLen);
	return enchant_dict_is_in_session (m_dict, ignore.utf8_str(), ignore.byteLength()) != 0;
}

void EnchantChecker::ignoreWord (const UT_UCSChar *toCorrect, size_t toCorrectLen)
{
	UT_return_if_fail (m_dict);
	UT_return_if_fail (toCorrect && toCorrectLen);

	UT_UTF8String ignore (toCorrect, toCorrectLen);

	enchant_dict_add_to_session (m_dict, ignore.utf8_str(), ignore.byteLength());
}
   
void EnchantChecker::correctWord (const UT_UCSChar *toCorrect, size_t toCorrectLen,
								  const UT_UCSChar *correct, size_t correctLen)
{
	UT_return_if_fail (m_dict);

	UT_return_if_fail (toCorrect && toCorrectLen);
	UT_return_if_fail (correct && correctLen);

	UT_UTF8String bad (toCorrect, toCorrectLen);
	UT_UTF8String good (correct, correctLen);

	enchant_dict_store_replacement (m_dict,
									bad.utf8_str(), bad.byteLength(),
									good.utf8_str(), good.byteLength());
}

bool
EnchantChecker::_requestDictionary (const char * szLang)
{
	UT_return_val_if_fail (szLang, false);
	UT_return_val_if_fail (s_enchant_broker, false);

	// Convert the language tag from en-US to en_US form
	char * lang = g_strdup (szLang);
	char * hyphen = strchr (lang, '-');
	if (hyphen)
		*hyphen = '_';

	m_dict = enchant_broker_request_dict(s_enchant_broker, lang);
	FREEP(lang);

	return (m_dict != NULL);
}
