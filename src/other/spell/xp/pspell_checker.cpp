/* AbiSuite
 * UT_DEBUGMESG(
 * Copyright (C) 2001 AbiSource, Inc.
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
 * Pspell 0.11. If we upgrade to Pspell 0.12, we can ditch the utfXXX_to_utfYYY
 * functions if we pass the size in bytes of the passed string and specify
 * that we're using "machine unsigned 16" instead of utf8 encoding
 */
#define USE_ORIGINAL_MANAGER_FUNCS 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_debugmsg.h"
#include "pspell_checker.h"

#include "xap_EncodingManager.h"
#include "ut_iconv.h"
#include "ut_string.h"

#define UCS_2_INTERNAL "UCS-2"

/*!
 * Convert a word16 to an UCS2 value by byteswapping if needed
 *
 * \param word16 The input string
 * \param len The lengh of the input string
 */
static void toucs2(unsigned short *word16, int length)
{
	int i = 0;
	if (XAP_EncodingManager__swap_utos) {
		for(;i<length;++i)
		{
			word16[i] = ((word16[i]>>8) & 0xff) | ((word16[i]&0xff)<<8);
		}
	}
}

/*!
 * Convert an UCS2 value to word16 by byteswapping if needed
 *
 * \param word16 The input string
 * \param len The lengh of the input string
 */
static void fromucs2(unsigned short *word16, int length)
{
	int i = 0;
	if (XAP_EncodingManager__swap_stou) {
		for(;i<length;++i)
		{
			word16[i] = ((word16[i]>>8) & 0xff) | ((word16[i]&0xff)<<8);
		}
	}
}

/*!
 * Convert an UTF16 string to an UTF8 string
 *
 * \param word16 The zero-terminated input string in UTF16 format
 * \param length The lengh of the input string
 * \return A zero-terminated UTF8 string
 */
static unsigned char*
utf16_to_utf8(const unsigned short *word16, int length)
{
  UT_uint32 len_out;

  UT_UCSChar * ucs2;
  unsigned char *result;

  UT_UCS_cloneString (&ucs2, word16);

  toucs2 (ucs2, length);
  /* Note that length is in shorts, so we have to double it here */
  (char *) result = UT_convert ((const char *)ucs2, length*2, UCS_2_INTERNAL,
                                "utf-8", NULL, &len_out);

  /* We assume that UT_convert creates a buffer big enough for this: */
  result[len_out] = 0;

  FREEP (ucs2);
  
  return result;
}

/*!
 * Convert an UTF8 string to an UTF16 string
 *
 * \param word16 The zero-terminated input string in UTF8 format
 * \param length The lengh of the input string
 * \return A zero-terminated UTF16 string
 */
static unsigned short *
utf8_to_utf16(const char *word8, int length)
{
  UT_uint32 len_out;
  unsigned char *result;
  UT_UCSChar * word16;

  (char *) result = UT_convert (word8, length, "utf-8",UCS_2_INTERNAL,
		              NULL, &len_out);
  if (! result)
    return NULL;

  word16 = (UT_UCSChar *)result;

  /* Hack: len_out is in bytes */
  len_out /= 2;

  /* We assume that UT_convert creates result big enough for this: */
  word16[len_out] = 0;

  fromucs2 (word16, len_out);

  return word16;
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
	UT_ASSERT(szLang);
	// Done: convert the language tag from en-US to en_US form
	char * lang = UT_strdup (szLang);
	char * hyphen = strchr (lang, '-');
	if (hyphen)
		*hyphen = '_';

	spell_config = new_pspell_config();
	pspell_config_replace(spell_config, "language-tag",
						  lang);
	pspell_config_replace(spell_config, "encoding", "utf-8");

	spell_error = new_pspell_manager(spell_config);
	delete_pspell_config(spell_config);
	UT_DEBUGMSG(("Attempting to load %s \n",lang));
		
	FREEP(lang);

	if(pspell_error_number(spell_error) != 0)
    {
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
PSpellChecker::checkWord (const UT_UCSChar * szWord, 
			  size_t len)
{
	SpellChecker::SpellCheckResult ret = SpellChecker::LOOKUP_FAILED;

	/* pspell segfaults if we don't pass it a valid spell_manager */
	if (spell_manager == NULL)
		return SpellChecker::LOOKUP_ERROR;

	/* trying to spell-check a 0 length word will (rightly) cause pspell 
	   to segfault */
	if(szWord == NULL || len == 0)
		return SpellChecker::LOOKUP_FAILED;

	unsigned char *word8 = utf16_to_utf8(szWord, len);
	if (! word8) 
		return SpellChecker::LOOKUP_ERROR;

	switch (pspell_manager_check(spell_manager, (char*)word8))
	{
	case 0:
		ret = SpellChecker::LOOKUP_FAILED; break;
	case 1:
		ret = SpellChecker::LOOKUP_SUCCEEDED; break;
	default:
		ret = SpellChecker::LOOKUP_ERROR; break;
	}

	free(word8);

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

	/* pspell segfaults if we don't pass it a valid spell_manager */
	if (spell_manager == NULL)
		return 0;

	/* trying to spell-check a 0 length word will (rightly) cause pspell 
	   to segfault */
	if(szWord == NULL || len == 0)
	{
		return 0;
	}

	unsigned char *word8 = utf16_to_utf8(szWord, len);
	if (! word8) {
		return 0;
	}
	word_list   = pspell_manager_suggest(spell_manager, (char*)word8);
	suggestions = pspell_word_list_elements(word_list);
	count       = pspell_word_list_size(word_list);
	free(word8);

	if(count == 0)
	{
		return 0;
	}

	UT_Vector * sg = new UT_Vector ();

	while ((new_word = pspell_string_emulation_next(suggestions)) != NULL) 
    {
		int len = strlen(new_word);

		UT_UCSChar *word = utf8_to_utf16(new_word, len);
		if (word) {
			sg->addItem ((void *)word);
			i++;
		}
    }

	return sg;
}
