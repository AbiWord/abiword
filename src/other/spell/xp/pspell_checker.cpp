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

#include "ut_debugmsg.h"
#include "pspell_checker.h"

#include "xap_EncodingManager.h"
#include "ut_iconv.h"
#include "ut_string.h"

#include "xap_Frame.h"
#include "xap_Strings.h"

static void couldNotLoadDictionary ( const char * szLang )
{
  XAP_App             * pApp   = XAP_App::getApp ();
  XAP_Frame           * pFrame = pApp->getLastFocussedFrame ();
  
  UT_return_if_fail ( pFrame ) ;
  
  const XAP_StringSet * pSS    = pApp->getStringSet ();
  
  UT_String buf; // Boundless buffer
  const char * text = pSS->getValue (XAP_STRING_ID_DICTIONARY_CANTLOAD);
  UT_String_sprintf(buf, text, szLang);
  char *cbuf = buf.c_str();
  pFrame->showMessageBox (cbuf,
			  XAP_Dialog_MessageBox::b_O,
			  XAP_Dialog_MessageBox::a_OK);
}

/*!
 * Convert an UTF32 string to an UTF8 string
 *
 * \param word32 The zero-terminated input string in UTF32 format
 * \param length The lengh of the input string
 * \return A zero-terminated UTF8 string
 */
static unsigned char*
utf32_to_utf8(const UT_UCS4Char * word32, int length)
{
  UT_uint32 len_out;
  unsigned char *result;
  
  /* Note that length is in shorts, so we have to double it here */
  result = (unsigned char*)
    UT_convert ((const char *)word32, length*4, UCS_INTERNAL,
		"utf-8", NULL, &len_out);
  
  /* We assume that UT_convert creates a buffer big enough for this: */
  result[len_out] = 0;
  
  return result;
}

/*!
 * Convert an UTF8 string to an UTF32 string
 *
 * \param word8 The zero-terminated input string in UTF8 format
 * \param length The lengh of the input string
 * \return A zero-terminated UTF16 string
 */
static UT_UCS4Char *
utf8_to_utf32(const char *word8, int length)
{
  UT_uint32 len_out;
  unsigned char *result;
  UT_UCS4Char * word32;
  
  result = (unsigned char *)
    UT_convert (word8, length, "utf-8", UCS_INTERNAL, NULL, &len_out);
  
  UT_return_val_if_fail ( result, NULL ) ;
  
  word32 = (UT_UCS4Char *)result;
  
  /* Hack: len_out is in bytes */
  len_out /= 4;
  
  /* We assume that UT_convert creates result big enough for this: */
  word32[len_out] = 0;
  
  return word32;
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
  pspell_config_replace(spell_config, "language-tag",
			lang);
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
PSpellChecker::checkWord (const UT_UCSChar * szWord, 
			  size_t len)
{
  SpellChecker::SpellCheckResult ret = SpellChecker::LOOKUP_FAILED;
  
  /* pspell segfaults if we don't pass it a valid spell_manager */
  UT_return_val_if_fail ( spell_manager, SpellChecker::LOOKUP_ERROR );
  UT_return_val_if_fail ( szWord, SpellChecker::LOOKUP_ERROR ) ;
  UT_return_val_if_fail ( len, SpellChecker::LOOKUP_ERROR ) ;

  unsigned char *word8 = utf32_to_utf8(szWord, len);
  UT_return_val_if_fail ( word8, SpellChecker::LOOKUP_ERROR ) ;
  
  switch (pspell_manager_check(spell_manager, (char*)word8))
    {
    case 0:
      ret = SpellChecker::LOOKUP_FAILED; break;
    case 1:
      ret = SpellChecker::LOOKUP_SUCCEEDED; break;
    default:
      ret = SpellChecker::LOOKUP_ERROR; break;
    }
  
  FREEP(word8);
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
  UT_return_val_if_fail ( spell_manager, 0 ) ;

  /* trying to spell-check a 0 length word will (rightly) cause pspell 
     to segfault */
  UT_return_val_if_fail ( szWord && len, 0 ) ;

  unsigned char *word8 = utf32_to_utf8(szWord, len);
  UT_return_val_if_fail ( word8, 0 ) ;
  
  word_list   = pspell_manager_suggest(spell_manager, (char*)word8);
  suggestions = pspell_word_list_elements(word_list);
  count       = pspell_word_list_size(word_list);
  FREEP(word8);

  // no suggestions, not an error
  if(count == 0)
    {
      return 0;
    }
  
  UT_Vector * sg = new UT_Vector ();
  
  while ((new_word = pspell_string_emulation_next(suggestions)) != NULL) 
    {
      int len = strlen(new_word);
      
      UT_UCSChar *word = utf8_to_utf32(new_word, len);
      if (word) {
	sg->addItem ((void *)word);
	i++;
      }
    }
  
  return sg;
}
