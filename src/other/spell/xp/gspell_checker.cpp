/* AbiSuite
 * Copyright (C) 2003 Dom Lachowicz
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gspell_checker.h"
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

GSpellChecker::GSpellChecker ()
	: m_dict (CORBA_NIL)
{
	m_dict = bonobo_get_object ("OAFIID:GNOME_Spell_Dictionary:0.2", "GNOME/Spell/Dictionary", 0);
	UT_ASSERT (m_dict != CORBA_NIL)
}

GSpellChecker::~GSpellChecker()
{
	if(m_dict != CORBA_NIL)
		bonobo_object_release_unref (m_dict, NULL);
}

/*!
 * Load the dictionary represented by szLang
 * szLang takes the form of {"en-US", "en_US", "en"}
 * 
 * \param szLang The dictionary to load
 * \return true if we loaded the dictionary, false if not
 */
bool 
GSpellChecker::requestDictionary (const char * szLang)
{
	UT_return_val_if_fail ( szLang, false ) ;

	CORBA_Environment ev;
	CORBA_exception_init (&ev);

	// Convert the language tag from en-US to en_US form
	char * lang = UT_strdup (szLang);
	char * hyphen = strchr (lang, '-');
	if (hyphen)
		*hyphen = '_';

	GNOME_Spell_Dictionary_setLanguage (m_dict, lang, &ev);
	FREEP(lang);

	if(ev._major != CORBA_NO_EXCEPTION)
	{
		s_couldNotLoadDictionary ( szLang );
		CORBA_exception_free (&ev);
		return false;
	}

	CORBA_exception_free (&ev);
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
GSpellChecker::checkWord (const UT_UCSChar * szWord, size_t len)
{
	SpellChecker::SpellCheckResult ret = SpellChecker::LOOKUP_FAILED;
	
	UT_return_val_if_fail ( m_dict != CORBA_NIL, SpellChecker::LOOKUP_ERROR );
	UT_return_val_if_fail ( szWord, SpellChecker::LOOKUP_ERROR ) ;
	UT_return_val_if_fail ( len, SpellChecker::LOOKUP_ERROR ) ;

	CORBA_Environment ev;
	CORBA_exception_init (&ev);

	CORBA_boolean result = GNOME_Spell_Dictionary_checkWord (m_dict, UT_UTF8String (szWord, len).utf8_str(),
															 &ev);

	if (ev._major != CORBA_NO_EXCEPTION)
		ret = SpellChecker::LOOKUP_ERROR;
	else if (result == CORBA_FALSE)
		ret = SpellChecker::LOOKUP_FAILED;
	else
		ret = SpellChecker::LOOKUP_SUCCEEDED;

	CORBA_exception_free (&ev);
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
GSpellChecker::suggestWord (const UT_UCSChar * szWord, 
							size_t len)
{
	GNOME_Spell_StringSeq *seq = 0;
	CORBA_Environment   ev;
	const char *new_word = NULL;
	int count = 0, i = 0;
	
	UT_return_val_if_fail ( m_dict != CORBA_NIL, 0 ) ;
	UT_return_val_if_fail ( szWord && len, 0 ) ;

	CORBA_exception_init (&ev);

	seq = GNOME_Spell_Dictionary_getSuggestions (m_dict, UT_UTF8String (szWord, len).utf8_str(), &ev);
	if (seq == CORBA_NIL) {
		CORBA_exception_free (&ev);
		return 0;
	} else if (seq->_length == 0) {
		CORBA_free (seq);
		CORBA_exception_free (&ev);
		return 0;
	}
	
	UT_Vector * sg = new UT_Vector ();
	
	for (i = 0; i < seq->_length; i++)
	{		
		UT_UCSChar *word = utf8_to_utf32(seq->_buffer [i]);
		if (word)
			sg->addItem (static_cast<void *>(word));
	}
	
	CORBA_free (seq);
	CORBA_exception_free (&ev);

	return sg;
}

bool 
GSpellChecker::addToCustomDict (const UT_UCSChar *word, size_t len)
{
	bool result = false;
	
	if (m_dict != CORBA_NIL && word && len) {
		CORBA_Environment   ev;
		CORBA_exception_init (&ev);
		GNOME_Spell_Dictionary_addWordToPersonal (m_dict, UT_UTF8String (word, len).utf8_str(), &ev);
		if (ev._major == CORBA_NO_EXCEPTION) 
			result = true;
		CORBA_exception_free (&ev);
	}
	return result;
}

void 
GSpellChecker::correctWord (const UT_UCSChar *toCorrect, size_t toCorrectLen,
							const UT_UCSChar *correct, size_t correctLen)
{
	UT_return_if_fail (m_dict != CORBA_NIL);
	UT_return_if_fail (toCorrect || toCorrectLen);
	UT_return_if_fail (correct || correctLen);

	UT_UTF8String bad (toCorrect, toCorrectLen);
	UT_UTF8String good (correct, correctLen);

	CORBA_Environment   ev;
	CORBA_exception_init (&ev);

	GNOME_Spell_Dictionary_setCorrection (m_dict, bad.utf8_str(), good.utf8_str());

	CORBA_exception_free (&ev);
}
