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

#define WORD_SIZE 256 /* or whatever */

/*
 * We should really do away with these functions if at all possible
 */
static void 
utf16_to_utf8(const unsigned short *word16, unsigned char * word8,
			  int length)
{
  unsigned char *pC = word8;
  unsigned short *pS = (unsigned short*)word16;
  int i;

  for (i = 0; i < length; i++, pS++)
    pC += unichar_to_utf8(*pS, pC);
  *pC++ = 0;
}

static void 
utf8_to_utf16(const char *word8, unsigned short *word16, 
			  int length)
{
  unsigned short *p;
  int i;

  /* this should work since UTF8 is a subset of UTF16 */
  for(i = 0, p = word16; i < length; i++)
    *p++ = (unsigned short)*word8++;
  *p = 0;
}

PSpellChecker::PSpellChecker ()
{
}

PSpellChecker::~PSpellChecker()
{
	/* pspell segfaults for some reason. get this fixed */
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
SpellChecker::SpellCheckResult PSpellChecker::checkWord (const UT_UCSChar * szWord, 
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
