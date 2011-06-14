/* AbiSuite
 * Copyright (C) 2012 chenxiajian <chenxiajian1985@gmail.com> 
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Strings.h"
#include "enchant_Hyphenation.h"
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

EnchantHyphenation::EnchantHyphenation()
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

EnchantHyphenation::~EnchantHyphenation()
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

Hyphenation::HyphenationResult
EnchantHyphenation::_hyphenation (const UT_UCSChar * ucszWord, size_t len)
{
	UT_return_val_if_fail (m_dict, Hyphenation::Hyphenation_ERROR);
	UT_return_val_if_fail (ucszWord, Hyphenation::Hyphenation_ERROR);
	UT_return_val_if_fail (len, Hyphenation::Hyphenation_ERROR);

	UT_UTF8String utf8 (ucszWord, len);

	switch (enchant_dict_hyphenation (m_dict, utf8.utf8_str(), utf8.byteLength())) 
	{
	case -1:
		return Hyphenation::Hyphenation_ERROR;
	case 0:
		return Hyphenation::Hyphenation_ERROR;
	default:
		return Hyphenation::Hyphenation_ERROR;
	}
}

UT_GenericVector<UT_UCSChar*> *
EnchantHyphenation::__hyphenateWord (const UT_UCSChar *ucszWord, size_t len)
{
	UT_return_val_if_fail (m_dict, 0);
	UT_return_val_if_fail (ucszWord && len, 0);

	UT_GenericVector<UT_UCSChar*> * pvSugg = new UT_GenericVector<UT_UCSChar*>();

	UT_UTF8String utf8 (ucszWord, len);

	char ** suggestions;
	size_t n_suggestions;

	suggestions = enchant_dict_hyphenationSuggest (m_dict, utf8.utf8_str(), utf8.byteLength(), &n_suggestions);

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

bool
EnchantHyphenation::_requestDictionary (const char * szLang)
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