/* AbiSuite
 * Copyright (C) 2001 AbiSource, Inc.
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

#define WORD_SIZE 256 /* or whatever */
#define UCS_2_INTERNAL "UCS-2"

/* this one fills ucs2 with values that iconv will treat as UCS-2. */
static void toucs2(const unsigned short *word16, int length, unsigned short *out)
{
	int i = 0;
	const unsigned short* in = word16;
/*	unsigned short* out = ucs2; */
	for(;i<length;++i)
	{
		if (XAP_EncodingManager__swap_utos)
		    out[i] = ((in[i]>>8) & 0xff) | ((in[i]&0xff)<<8);
		else
		    out[i] = in[i];
	}
	out[i]= 0;
}

/* this one copies from 'ucs2' to word16 swapping bytes if necessary */
static void fromucs2(unsigned short *word16, int length, unsigned short *ucs2)
{
	int i = 0;
	unsigned short *in = ucs2;
	unsigned short *out = word16; 
	for(;i<length;++i)
	{
		if (XAP_EncodingManager__swap_stou)
			out[i] = ((in[i]>>8) & 0xff) | ((in[i]&0xff)<<8);
		else
			out[i] = in[i];
	}
	out[i]= 0;
}

static void 
utf16_to_utf8(const unsigned short *word16, unsigned char * word8,
	      int length)
{
  UT_uint32 len_out;

  UT_UCSChar * ucs2;

  UT_UCS_cloneString (&ucs2, word16);

  toucs2 (ucs2, length, ucs2);
  UT_convert ((const char *)ucs2, length, "utf-8", UCS_2_INTERNAL,
	      (UT_uint32 *)word8, &len_out);

  FREEP (ucs2);

  word8[len_out] = '\0';
}

static void 
utf8_to_utf16(const char *word8, unsigned short *word16, 
	      int length)
{
  UT_uint32 len_out;

  UT_convert ((const char *) word8, length, UCS_2_INTERNAL, "utf-8",
	      (UT_uint32 *)word16, &len_out);
  word16[len_out] = '\0';
  fromucs2 (word16, len_out, word16);
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
	unsigned char  word8[WORD_SIZE];
	SpellChecker::SpellCheckResult ret = SpellChecker::LOOKUP_FAILED;

	/* pspell segfaults if we don't pass it a valid spell_manager */
	if (spell_manager == NULL)
		return SpellChecker::LOOKUP_ERROR;

	/* trying to spell-check a 0 length word will (rightly) cause pspell 
	   to segfault */
	if(szWord == NULL || len == 0)
		return SpellChecker::LOOKUP_FAILED;

	utf16_to_utf8(szWord, word8, len);

	switch (pspell_manager_check(spell_manager, (char*)word8))
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
	unsigned char word8[WORD_SIZE];
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

	utf16_to_utf8(szWord, word8, len);
	word_list   = pspell_manager_suggest(spell_manager, (char*)word8);
	suggestions = pspell_word_list_elements(word_list);
	count       = pspell_word_list_size(word_list);

	if(count == 0)
	{
		return 0;
	}

	UT_Vector * sg = new UT_Vector ();

	while ((new_word = pspell_string_emulation_next(suggestions)) != NULL) 
    {
		int len = strlen(new_word);

		UT_UCSChar * word = (UT_UCSChar *)malloc(sizeof(UT_UCSChar) * len + 2);
		if (word == NULL) 
		{
			// out of memory, but return what was copied so far
			return sg;
		}
      
		utf8_to_utf16(new_word, word, len);
		sg->addItem ((void *)word);
		i++;
    }

	return sg;
}
