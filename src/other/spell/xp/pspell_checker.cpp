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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pspell_checker.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

/*!
 * Convert a UTF-8 string to a UTF-32 string
 *
 * \param word8 The zero-terminated input string in UTF-8 format
 * \return A zero-terminated UTF32 string
 */
static UT_UCS4Char *
utf8_to_utf32(const char *word8)
{
	UT_UCS4Char * ucs4 = 0;
	UT_UCS4_cloneString (&ucs4, UT_UCS4String (word8).ucs4_str());
	return ucs4;
}

PSpellChecker::PSpellChecker ()
  : spell_manager(0)
{
}

PSpellChecker::~PSpellChecker()
{
	// some versions of pspell segfault here for some reason
	if(spell_manager)
		delete_pspell_manager(spell_manager);
}

/*!
 * Load the dictionary represented by szLang
 * szLang takes the form of {"en-US", "en_US", "en"}
 * 
 * \param szLang The dictionary to load
 * \return true if we loaded the dictionary, false if not
 */
bool 
PSpellChecker::requestDictionary (const char * szLang)
{
	PspellConfig *spell_config;
	PspellCanHaveError *spell_error;

	UT_return_val_if_fail ( szLang, false ) ;

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
	UT_DEBUGMSG(("Attempting to load %s \n",lang));
	
	FREEP(lang);
	
	if(pspell_error_number(spell_error) != 0)
	{
		couldNotLoadDictionary ( szLang );
		UT_DEBUGMSG(("SpellCheckInit: Pspell error: %s\n",
					 pspell_error_message(spell_error)));
		return false;
	}
	
	spell_manager = to_pspell_manager(spell_error);
	return true;
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
PSpellChecker::checkWord (const UT_UCSChar * szWord, size_t len)
{
	SpellChecker::SpellCheckResult ret = SpellChecker::LOOKUP_FAILED;
	
	UT_return_val_if_fail ( spell_manager, SpellChecker::LOOKUP_ERROR );
	UT_return_val_if_fail ( szWord, SpellChecker::LOOKUP_ERROR ) ;
	UT_return_val_if_fail ( len, SpellChecker::LOOKUP_ERROR ) ;

	switch (pspell_manager_check(spell_manager, const_cast<char*>(UT_UTF8String (szWord, len).utf8_str())))
	{
	case 0:
		ret = SpellChecker::LOOKUP_FAILED; break;
	case 1:
		ret = SpellChecker::LOOKUP_SUCCEEDED; break;
	default:
		ret = SpellChecker::LOOKUP_ERROR; break;
	}
	
	return ret;
}

/*!
 * Suggest replacement words for szWord
 * \param szWord Non-null word to find suggestions for
 * \param len Length of szWord
 *
 * \return A vector of UT_UCSChar * suggestions. The vector must be
 *         'delete'd and its UT_UCSChar * suggests must be 'free()'d
 */
UT_Vector * 
PSpellChecker::suggestWord (const UT_UCSChar * szWord, 
			    size_t len)
{
	PspellStringEmulation *suggestions = NULL;
	const PspellWordList *word_list = NULL;
	const char *new_word = NULL;
	int count = 0, i = 0;
	
	UT_return_val_if_fail ( spell_manager, 0 ) ;
	UT_return_val_if_fail ( szWord && len, 0 ) ;

	word_list   = pspell_manager_suggest(spell_manager, const_cast<char*>(UT_UTF8String (szWord, len).utf8_str()));
	suggestions = pspell_word_list_elements(word_list);
	count       = pspell_word_list_size(word_list);

	// no suggestions, not an error
	if(count == 0) {
		return 0;
	}
	
	UT_Vector * sg = new UT_Vector ();
	
	while ((new_word = pspell_string_emulation_next(suggestions)) != NULL) 
	{
		UT_UCSChar *word = utf8_to_utf32(new_word);
		if (word)
		{
			sg->addItem (static_cast<void *>(word));
			i++;
		}
	}

	delete_pspell_string_emulation (suggestions);
	return sg;
}

bool 
PSpellChecker::addToCustomDict (const UT_UCSChar *word, size_t len)
{
  if (spell_manager && word && len) {
    pspell_manager_add_to_personal(spell_manager, const_cast<char *>(UT_UTF8String (word, len).utf8_str()));
    return true;
  }
  return false;
}

void 
PSpellChecker::correctWord (const UT_UCSChar *toCorrect, size_t toCorrectLen,
							const UT_UCSChar *correct, size_t correctLen)
{
	UT_return_if_fail (spell_manager);
	UT_return_if_fail (toCorrect || toCorrectLen);
	UT_return_if_fail (correct || correctLen);

	UT_UTF8String bad (toCorrect, toCorrectLen);
	UT_UTF8String good (correct, correctLen);

	pspell_manager_store_replacement (spell_manager, bad.utf8_str(), good.utf8_str());
}
